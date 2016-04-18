Windows 10 IoT Blockly Sample
==============
IoTBlockly leverages various pieces of open source software to create a "block" development experience right on your Raspberry Pi.

<ol>
<li>Google Blockly from https://developers.google.com/blockly for the block editor</li>
<li>Chakra JavaScript engine to execute JavaScript snippets (see sample repo at https://github.com/Microsoft/Chakra-Samples)</li>
<li>Emmellsoft.IoT.Rpi.SenseHat library to control the Raspberry Pi Sense Hat (https://github.com/emmellsoft/RPi.SenseHat)</li>
</ol>

###Requirements:
<ol>
<li>Raspberry Pi 2 or Raspberry Pi 3</li>
<li>Raspberry Pi Sense Hat (https://www.raspberrypi.org/products/sense-hat)</li>
<li>Windows 10 Core installed and running on the Raspberry Pi</li>
</ol>

###Usage
Compile the solution and deploy the IoTBlocklyBackgroundApp to your Raspberry Pi. Once the app is up and running, browse to http://your-rpi-name:8000 and starting coding with Blockly! 
