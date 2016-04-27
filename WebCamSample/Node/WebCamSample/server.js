var http = require('http'),
    uwp = require('uwp'),
    fs = require('fs'),
    url = require('url');
uwp.projectNamespace('Windows');

var PHOTO_FILENAME = 'photo.jpg';
var VIDEO_FILENAME = 'video.mp4';
var AUDIO_FILENAME = 'audio.mp3';

var mediaCapture;
var photoFile;
var recordStorageFile;
var audioFile;

var isRecording;
var mode;

var _status;

var urlMap = {
    '/' : function (req, res) {
        fs.readFile('./default.html', function (err, html) {
            if (err)
                throw err;
            res.simpleHTML(200, html);
        });
    },
    '/Stylesheets/fabric.min.css': function (req, res) {
        fs.readFile('.' + req.url, function (err, content) {
            if (err)
                throw err;
            res.writeHead(200, { 'Content-Type': 'text/css', 'Content-Length': content.length });
            res.end(content);
        });
    },
    '/Stylesheets/fabric.components.min.css': function (req, res) {
        fs.readFile('.' + req.url, function (err, content) {
            if (err)
                throw err;
            res.writeHead(200, { 'Content-Type': 'text/css', 'Content-Length': content.length });
            res.end(content);
        });
    },
    '/listener/takephoto': function (req, res){
        takePhoto();
        res.simpleJSON(200, _status);
    }
    ,
    '/listener/initvideo': function(req, res){
        initVideo();
        res.simpleJSON(200, _status);
    },
    '/listener/startvideo': function (req, res) {
        startRecordVideo();
        res.simpleJSON(200, _status);
    },
    '/listener/stopvideo': function (req, res) {
        stopRecordVideo();
        res.simpleJSON(200, _status);
    },
    '/listener/initaudio': function (req, res) {
        initAudioOnly();
        res.simpleJSON(200, _status);
    },
    '/listener/startaudio': function (req, res) {
        startAudioRecord();
        res.simpleJSON(200, _status);
    },
    '/listener/stopaudio': function (req, res){
        stopAudioRecord();
        res.simpleJSON(200, _status);
    },
    '/listener/status': function (req, res){
        res.simpleJSON(200, { status: _status, isRecording: isRecording, mode:mode });
    }
};

function status(text, append){
    if (append) {
        _status += text;
    }
    else {
        _status = text;
    }
    console.log(text)
}

function notFound(req, res) {
    var NOT_FOUND = "Not Found\n";
    res.writeHead(404, { "Content-Type": "text/plain", "Content-Length": NOT_FOUND.length });
    res.write(NOT_FOUND);
    res.end();
}

(function () {
    http.createServer(function (req, res) {
        handler = urlMap[url.parse(req.url).pathname] || notFound;
        
        res.simpleHTML = function (code, body) {
            res.writeHead(code, {
                'Content-Type': 'text/html',
                'Content-Length': body.length
            });
            res.end(body);
        };
        
        res.simpleJSON = function (code, obj) {
            var body = JSON.stringify(obj);
            res.writeHead(code, {
                'Content-Type': 'application/json',
                'Content-Length': body.length
            });
            res.end(body);
        };

        handler(req, res);
    }).listen(80);
    
    isRecording = false;
})();

function cleanup(){
    if (mediaCapture != null) {

        if (isRecording) {
            mediaCapture.stopRecordAsync();
            isRecording = false;
        }

        mediaCapture.close();
        mediaCapture = null;
    }
}

function initVideo(){
    try {
        if (mediaCapture != null) {
            // Cleanup MediaCaputre object
            if (isRecording) {
                mediaCapture.stopRecordAsync();
                isRecording = false;
            }

            mediaCapture.close();
            mediaCapture = null;
        }

        status('Initializing camera to capture audio and video...');
        // Use default initialization
        mediaCapture = new Windows.Media.Capture.MediaCapture();
        mediaCapture.initializeAsync();

        // Set callbacks for failure and recording limit exceeded
        status('Device successfully initialized for video recording and photos!');        
        mediaCapture.onfailed = mediaCapture_Failed; 
        mediaCapture.onrecordlimitationexceeded = mediaCapture_RecordLimitExceeded;
    }
    catch (ex) {
        status('Unable to initialize camera for audio/video mode: ' + ex.message);
    }
}

function initAudioOnly(){
    try {
        if (mediaCapture != null) {
            if (isRecording) {
                mediaCapture.stopRecordAsync();
                isRecording = false;
            }

            mediaCapture.close();
            mediaCapture = null;
        }

        status('Initializing camera to capture audio only...');
        mediaCapture = new Windows.Media.Capture.MediaCapture();
        var settings = new Windows.Media.Capture.MediaCaptureInitializationSettings();
        settings.streamingCaptureMode = Windows.Media.Capture.StreamingCaptureMode.audio;
        settings.mediaCategory = Windows.Media.Capture.MediaCategory.other;
        settings.audioProcessing = Windows.Media.AudioProcessing.default;

        mediaCapture.initializeAsync(settings);

        // Set callbacks for failure and recording limit exceeded
        status('Device successfully initialized for audio recording!\nPress "Start Audio Record" to record');
        mediaCapture.onfailed = mediaCapture_Failed;
        mediaCapture.onrecordlimitationexceeded = mediaCapture_RecordLimitExceeded;
    }
    catch (ex) {
        status('Unable to initialize camera for audio mode: ' + ex.Message); 
    }
}

function takePhoto(){
    try {
        photoFile = Windows.Storage.KnownFolders.picturesLibrary.createFileAsync(PHOTO_FILENAME, Windows.Storage.CreationCollisionOption.generateUniqueName)
        .then(function (storageFile) {
            photoFile = storageFile;

            var imageProperties = Windows.Media.MediaProperties.ImageEncodingProperties.createJpeg();
            
            mediaCapture.capturePhotoToStorageFileAsync(imageProperties, photoFile).done(function () {
                status('Photo successfully taken and is available at ' + photoFile.path);
            });
        });
    }
    catch (ex){
        status(ex.message);
        cleanup();
    }
}

function startRecordVideo(){
    try {
        status('Starting video recording...');
        mode = 'video';

        Windows.Storage.KnownFolders.videosLibrary.createFileAsync(VIDEO_FILENAME, Windows.Storage.CreationCollisionOption.generateUniqueName)
        .then(function (storageFile) {
            recordStorageFile = storageFile;
            status('Video storage file preparation successful'); 
            var recordProfile = Windows.Media.MediaProperties.MediaEncodingProfile.createMp4(Windows.Media.MediaProperties.VideoEncodingQuality.auto);

            mediaCapture.startRecordToStorageFileAsync(recordProfile, recordStorageFile).then(function (result) {
                isRecording = true;
                status('Video recording in progress...press "Stop Video Record" to stop');
            });
        });
    }
    catch (ex) {
        if (ex === Windows.System.UnauthorizedAccessException) {
            status('Unable to play recorded video; video recorded successfully to: ' + recordStorageFile.path);
        }
        else {
            status(ex.message);
            cleanup();
        }
    }
}

function stopRecordVideo(){
    try {
        status('Stopping video recording...');
        
        mediaCapture.stopRecordAsync();
        isRecording = false;

        status('Recorded video is ready for viewing and is available here ' + recordStorageFile.path);
    }
    catch (ex) {
        if (ex === Windows.System.UnauthorizedAccessException) {
            status('Unable to play recorded video; video recorded successfully to: ' + recordStorageFile.path);
        }
        else {
            status(ex.message);
            cleanup();
        }
    }
}

function startAudioRecord(){
    try {
        status('Starting audio recording...');
        mode = 'audio';

        Windows.Storage.KnownFolders.videosLibrary.createFileAsync(AUDIO_FILENAME, Windows.Storage.CreationCollisionOption.generateUniqueName)
        .then(function (storageFile) {
            audioFile = storageFile;
            
            status('Audio storage file preparation successful');
            var recordingProfile = Windows.Media.MediaProperties.MediaEncodingProfile.createM4a(Windows.Media.MediaProperties.AudioEncodingQuality.auto);
            mediaCapture.startRecordToStorageFileAsync(recordingProfile, audioFile).then(function (result) {
                isRecording = true;
                status('Audio recording in progress...pres "Stop Audio Record" to stop');
            });
        });
    }
    catch (ex) {
        status(ex.message);
        cleanup();
    }
}

function stopAudioRecord(){
    try {
        status('Stopping audio recording...');

        mediaCapture.stopRecordAsync();
        isRecording = false;
        
        status('Playback recorded audio: ' + audioFile.path);
    }
    catch (ex) {
        status(ex.message);
        cleanup();
    }
}

function mediaCapture_Failed(sender, args) {
    try {
        status('Media capture failed: ' + args.message);

        if (isRecording) {
            mediaCapture.stopRecordAsync();
            status('\n Recording stopped', true);
        }
    }
    catch (ex) {

    }
    finally {
        status('\nCheck if camera is disconnected. Try re-launching the app', true);
    }
}

function mediaCapture_RecordLimitExceeded(sender){
    try {
        if (isRecording) {
            status('Stopping recording on exceeding max record duration');
            mediaCapture.stopRecordAsync();
            isRecording = false;

            if (mediaCapture.MediaCaptureSettings.StreamingCaptureMode == Windows.Media.Capture.StreamingCaptureMode.Audio) {
                status('Stopped recording on exceeding max record duration: ' + audioFile.path);
            }
            else {
                status('Stopped recording on exceeding max record duration: ' + recordStorageFile.path);
            }
        }
    }
    catch (ex) {
        status(ex.message);
    }
}