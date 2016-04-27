var http = require('http'),
    uwp = require('uwp');
uwp.projectNamespace('Windows');

var firstAdapter = null;
var resultCollection = [];

function status(message){
    console.log(message);
}

(function () {
    var access = Windows.Devices.WiFi.WiFiAdapter.requestAccessAsync()
    .then(function (accessResult){
        if (accessResult != Windows.Devices.WiFi.WiFiAccessStatus.allowed) {
            status('Access to WiFi control has been denied!');
        }
        else {
            Windows.Devices.WiFi.WiFiAdapter.findAllAdaptersAsync()
            .done(function (deviceResults) {
                if (deviceResults.length > 1) {
                    Windows.Devices.WiFi.WiFiAdapter.fromIdAsync(deviceResults[0].id).done(function (adapter) {
                        firstAdapter = adapter;
                    });
                }
                else {
                    status('No WiFi Adapters detected on this device.');
                }
            });
        }
    })

})();

function scanForNetworks(){
    firstAdapter.scanAsync().done(function () {
        console.log('Scan complete');
        
        var report = firstAdapter.networkReport;
        resultCollection = [];

        for (i = 0; i < report.availableNetworks.length; i++) {
            var network = report.availableNetworks[i];
            resultCollection.push(new WiFiNetworkDisplay(network, firstAdapter));
        }
    });
}

function connectToNetwork(){
    var selectedNetwork;
    var reconnectionKind = Windows.Devices.WiFi.WiFiReconnectionKind.manual;

    if (selectedNetwork.availableNetwork.SecuritySettings.networkAuthenticationType == Windows.Networking.Connectivity.NetworkAuthenticationType.open80211) {
        firstAdapter.connectAsync(selectedNetwork.availableNetwork, reconnectionKind);
    }
    else {
        var credential = new PasswordCredential();
        credential.password = NetworkKey.password;

        firstAdapter.connectAsync(selectedNetwork.availableNetwork, reconnectionKind, credential);
    }

    if (result.connectionStatus = Windows.Devices.WiFi.WiFiConnectionStatus.success) {
        status('Successfully connected to ' + selectedNetwork.ssid);
    }
    else {
        status('Could not connect to ' + selectedNetwork.ssid + '. Error: ' + result.connectionStatus);
    }
}

function WiFiNetworkDisplay(availableNetwork, adapter){
    this.availableNetwork = availableNetwork;
    this.adapter = adapter;
    this.connectivityLevel = 'Not Connected';
    
    function updateConnectivityLevel() {
        var connectedSsid = null;

        this.adapter.NetworkAdapter.getConnectedProfileAsync().done(function (profile) {
            if (profile != null && profile.isWlanConnectionProfile && profile.wlanConnectionProfileDetails != null) {
                connectedSsid = profile.WlanConnectionProfileDetails.getConnectedSsid();
            }

            if (connectedSsid != null && _connectedSsid.length > 0) {
                if (connectedSsid == this.availableNetwork.ssid) {
                    this.connectivityLevel = profile.getNetworkConnectivityLevel();
                }
            }
        });
    };

    this.ssid = function () {
        return this.availableNetwork.ssid;
    };

    this.bssid = function () {
        return this.availableNetwork.bssid;
    };

    this.channelCenterFrequency = function () {
        return this.availableNetwork.channelCenterFrequencyInKilohertz + 'kHz';
    };
    
    this.rssi = function () {
        return this.availableNetwork.networkRssiInDecibelMilliwatts + 'dBm';
    };
    
    this.securitySettings = function () {
        return 'Authentication: ' + this.availableNetwork.SecuritySettings.networkAuthenticationType + '; Encryption: ' + this.availableNetwork.SecuritySettings.networkEncryptionType;
    };
}

/*
http.createServer(function (req, res) {
    res.writeHead(200, { 'Content-Type': 'text/plain' });
    res.end('Hello World\n');
}).listen(1337);
 */