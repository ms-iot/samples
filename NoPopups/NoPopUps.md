---
layout: docs
title: NoPopUps
description: A pattern for implementing PopUp message boxes as a grid element in IoT-Core UWP apps where PopUps aren't supported
keyword: PopUp, UWP, MessageBox, Accept, Cancel, IoT-Core
permalink: /en-US/Docs/NoPopUps.htm
samplelink: https://github.com/ms-iot/samples/NoPopUps/
lang: en-US
---

# NoPopUps

With a UWP UI app you might what a user response in an app to a prompt to delete something, exit the app, shutdown the OS etc. The problem is, that IoT-Core does not support the Popup class. In that context, with app prompts, using a PopUp message box, the app just skips through and does nothing, or may generate an unhandled error. 
* [Windows 10 PopUp Class](https://msdn.microsoft.com/en-us/library/windows/apps/windows.ui.xaml.controls.primitives.popup.aspx)
* [IoT-Core Unsupported Classes](https://developer.microsoft.com/en-us/windows/iot/docs/unavailableapis)

The above link says that   **Windows.UI.Popups.MessageDialog** class is not supported but teh whole Windows.UI.PopUps class is problematic with IoT-Core. This UI feature is probably excluded with IOT-Core apps because they may typically run with out much user interaction, or none with the a headless device.

This project demonstrates how to have a [Yes] [Cancel] Popup panel with a message with the app awaiting the user response and responding accordingly. The project code implements the confirmation message box both using the PopUp class and using the PopUp panel. A project compilation symbol (IOTCORE) determines which mechanism is used. That way the UWP app can run both on the desktop (with expected user interaction - **IOTCORE** _not defined_) and on an IoT-Core device with the PopUp panel (**IOTCORE** _defined_).

## Load the project in Visual Studio

You can find the source code for this sample by downloading a zip of all of our samples [here](https://github.com/ms-iot/samples/archive/develop.zip) , or by cloning te repository, and navigating to the `samples\NoPopUps`.  The sample code is available in C# as a Universal Windows Platform App. Make a copy of the folder on your disk and open the project from Visual Studio, 2015 Update 3. 

## Configure, Build and Deploy the App.
* Choose your CPU (eg ARM for Raspberry Pi2 or 3, x86 for the desktop).
* Set the build type to Debug or Release 
* For the desktop _clear_ the IOTCORE definition as follows:
* For IoT-Core (eg RPI2/3) _set_ the IOTCORE definition as follows:
* Go to the project property pages _(Menu: Project-->NoPopUps Properties)_ and select the **Build** page. In the _Conditional Compilation Symbols_ box add or remove the IOTCORE definition. Definitions must be separated by a semicolon. Note that when you set these definitions, they only apply for the current CPU and build type.
You can find the source code for this sample by downloading a zip of all of our samples [here](https://github.com/ms-iot/samples/archive/develop.zip) , or by cloning te repository, and navigating to the `samples\NoPopUps`.  The sample code is available in C# as a Universal Windows Platform App. Make a copy of the folder on your disk and open the project from Visual Studio.
* Whilst in the property pages, on the Application page,  examine the target build numbers, set to 10586 (first update) to 14393 (the Anniversary Edition).
* Build and deploythe app

## Testing
Run each of the 3 dummy actions and respond by accepting [Yes] or rejecting [Cancel] the confirmations. Note that on the desktop PopUp Message Boxes are used, whereas with IoT-Core, the PopUp Panel is used. 

# Further
Examine the code to view the asynchronous code used including a Semaphore with the PopUp Panel. If hardware buttons were to be used instead for confirmation, GPIO events could be used in place of the UI Button Click event, using this code pattern.



## Additional resources
* [Windows 10 IoT Core home page](https://developer.microsoft.com/en-us/windows/iot/)
* [Documentation for all samples](https://developer.microsoft.com/en-us/windows/iot/samples)

