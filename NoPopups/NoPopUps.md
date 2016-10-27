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
* [IoT-Core Unsupported Classes]()

This project demonstrates how to have a [Yes] [Cancel] Popup panel with a message with the app awaiting the user response and responding accordingly. The project code implements the confirmation message box both using the PopUp class and using the PopUp panel. A project compilation symbol (IOTCORE) determines which mechanism is used. That way the UWP app can run both on the desktop (with expected user interaction - **IOTCORE** _not defined_) and on an IoT-Core device with the PopUp panel (**IOTCORE** _defined_).

## How to download:

Unfortunately, GitHub does not support downloading individual code. 

Navigate to [ms-iot/samples](https://github.com/ms-iot/samples) and select **Clone or download** to download the whole repository.


## Additional resources
* [Windows 10 IoT Core home page](https://developer.microsoft.com/en-us/windows/iot/)
* [Documentation for all samples](https://developer.microsoft.com/en-us/windows/iot/samples)

This project has adopted the Microsoft Open Source Code of Conduct. For more information see the Code of Conduct FAQ or contact opencode@microsoft.com with any additional questions or comments.
