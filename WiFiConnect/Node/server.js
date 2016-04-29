var http = require('http'),
    fs = require('fs'),
    url = require('url'),
    uwp = require('uwp');
uwp.projectNamespace('Windows');

var firstAdapter = null;
var resultCollection = [];
var selectedNetwork = null;

var _status = null;
var _errors = [];

var urlMap = {
    '/': function (req, res){
        fs.readFile('./default.html', function (err, html) {
            if (err)
                throw err;
            res.simpleHTML(200, html);
        });
    },
    '/listener/state': function (req, res){
        res.simpleJSON(200, { adapter: firstAdapter, results: resultCollection, status: _status, errors: _errors });
        _errors = [];
    },
    '/listener/scan': function (req, res){
        scanForNetworks();
        res.simpleJSON(202, null);
    },
    '/listener/connect': function (req, res){
        var url_parts = url.parse(req.url, true);
        var query = url_parts.query;
        var key = (url_parts.key != undefined) ? url_parts.key : null;
        connectToNetwork(query.id, key);
        res.simpleJSON(202, null);
    },
    '/listener/disconnect': function (req, res) {
        var url_parts = url.parse(req.url, true);
        var query = url_parts.query;
        connectToNetwork(query.id);
        res.simpleJSON(202, null);
    }
};

function status(message, isError){
    _status = message;
    if (isError) {
        console.error(message);
        _errors.push(message);
    }
    else {
        console.log(message);
    }
}

function notFound(req, res) {
    var NOT_FOUND = "Not Found\n";
    res.writeHead(404, { "Content-Type": "text/plain", "Content-Length": NOT_FOUND.length });
    res.write(NOT_FOUND);
    res.end();
}

(function () {
    var access = Windows.Devices.WiFi.WiFiAdapter.requestAccessAsync()
    .then(function (accessResult) {
        if (accessResult != Windows.Devices.WiFi.WiFiAccessStatus.allowed) {
            status('Access to WiFi control has been denied!', true);
        }
        else {
            Windows.Devices.WiFi.WiFiAdapter.findAllAdaptersAsync()
            .done(function (deviceResults) {
                if (deviceResults.length > 0) {
                    firstAdapter = deviceResults[0];
                }
                else {
                    status('No WiFi Adapters detected on this device.', true);
                }
            });
        }
    });
})();

function scanForNetworks(){
    firstAdapter.scanAsync().done(function () {       
        var report = firstAdapter.networkReport;
        resultCollection = [];
        
        if (_status == null)
            status('Scan completed.', false);

        for (i = 0; i < report.availableNetworks.length; i++) {
            var network = report.availableNetworks[i];
            resultCollection.push(new WiFiNetworkDisplay(network, firstAdapter));
        }
    });
}

function connectToNetwork(index, key){
    if (resultCollection.length <= index) {
        status('Invalid network selected', true);
        return;
    }

    selectedNetwork = resultCollection[index];
    var reconnectionKind = Windows.Devices.WiFi.WiFiReconnectionKind.manual;

    if (selectedNetwork.availableNetwork.securitySettings.networkAuthenticationType == Windows.Networking.Connectivity.NetworkAuthenticationType.open80211) {
        firstAdapter.connectAsync(selectedNetwork.availableNetwork, reconnectionKind).done(onAdapterConnectionComplete);
    }
    else {
        var credential = new Windows.Security.Credentials.PasswordCredential();
        credential.password = key;

        firstAdapter.connectAsync(selectedNetwork.availableNetwork, reconnectionKind, credential).done(onAdapterConnectionComplete);
    }    
}

function onAdapterConnectionComplete(result){
    scanForNetworks();

    var connectionStatus;
    switch (result.connectionStatus) {
        case 1:
            connectionStatus = 'Success';
            break;
        case 2:
            connectionStatus = 'Access Revoked';
            break;
        case 3:
            connectionStatus = 'Invalid Credential';
            break;
        case 4:
            connectionStatus = 'Network Not Available';
            break;
        case 5:
            connectionStatus = 'Timeout';
            break;
        case 6:
            connectionStatus = 'Unsupported Authentication Protocol';
            break;
        default:
            connectionStatus = 'Unspecified Failure';
    }

    if (result.connectionStatus == Windows.Devices.WiFi.WiFiConnectionStatus.success) {
        status('Successfully connected to ' + selectedNetwork.ssid, false);
    }
    else {
        status('Could not connect to ' + selectedNetwork.ssid + '. Error: ' + connectionStatus, true);
    }
}

function WiFiNetworkDisplay(availableNetwork, adapter){
    this.availableNetwork = availableNetwork;
    this.adapter = adapter;
    this.connectivityLevel = 'Not Connected';
    this.ssid = this.availableNetwork.ssid;
    this.bssid = this.availableNetwork.bssid;
    this.channelCenterFrequency = this.availableNetwork.channelCenterFrequencyInKilohertz + 'kHz';
    this.rssi = this.availableNetwork.networkRssiInDecibelMilliwatts + 'dBm';
    this.authenticationType = this.availableNetwork.securitySettings.networkAuthenticationType;
    this.encryptionType = this.availableNetwork.securitySettings.networkEncryptionType;
    this.securitySettings = 'Authentication: ' + this.availableNetwork.securitySettings.networkAuthenticationType + '; Encryption: ' + this.availableNetwork.securitySettings.networkEncryptionType;
    
    var self = this;
    this.updateConnectivityLevel = function() {
        var connectedSsid = null;

        this.adapter.networkAdapter.getConnectedProfileAsync().done(function (profile) {
            if (profile != null && profile.isWlanConnectionProfile && profile.wlanConnectionProfileDetails != null) {
                connectedSsid = profile.wlanConnectionProfileDetails.getConnectedSsid();
            }

            if (connectedSsid != null && connectedSsid.length > 0) {
                if (connectedSsid == self.availableNetwork.ssid) {
                    if (profile.getNetworkConnectivityLevel() == 0) {
                        self.connectivityLevel = 'Not Connected';
                    }
                    else {
                        self.connectivityLevel = 'Connected';
                        status('Connected to ' + self.ssid, false);
                    }
                }
            }
        });
    };

    this.updateConnectivityLevel();
}

http.createServer(function (req, res) {
    handler = urlMap[url.parse(req.url).pathname] || notFound;
    
    res.simpleHTML = function (code, body) {
        res.writeHead(code, {
            "Content-Type": "text/html",
            "Content-Length": body.length
        });
        res.end(body);
    };
    
    res.simpleJSON = function (code, obj) {
        var body = JSON.stringify(obj);
        res.writeHead(code, {
            "Content-Type": "application/json",
            "Content-Length": body.length
        });
        res.end(body);
    };
    
    handler(req, res);
}).listen(80);
