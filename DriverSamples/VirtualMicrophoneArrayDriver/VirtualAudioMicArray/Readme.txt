 Virtual Audio Microphone Array Sample Driver

 Originally derived from the SYSVAD driver sample (https://github.com/Microsoft/Windows-driver-samples/tree/master/audio/sysvad),
 this sample illustrates how to construct a virtual microphone array at runtime using monoaural audio devices present on the system.

 Before running the sample, the INF must be updated to setup registry keys under the 
 drivers 'Parameters' subkey to identify which audio endpoints should be used as inputs and the
 format (NumChannels - for the entire array, SamplesPerSecond, BitsPerSample) of each input stream.

 Limitations in the sample:

 All input streams must use the same format - this sample performs no format conversion.

 There is no mechanism to control the gain of the input streams.

 The metadata for the microphone array geometry is hardcoded and CMicArrayMiniportTopology::PropertyHandlerMicArrayGeometry should be
 updated in micarraytopo.cpp should be updated to reflect the actual physical characteristics of the microphone array.
