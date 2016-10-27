---
layout: docs
title: NoPopUps
description: A pattern for implementing PopUp message boxes as a grid element in IoT-Core UWP apps where PopUps aren't supported
keyword: PopUp, UWP, MessageBox, Accept, Cancel, IoT-Core
permalink: /en-US/Docs/NoPopUps.htm
samplelink: https://github.com/ms-iot/samples/NoPopUps/
lang: en-US
---

PopUps

With a UWP UI app you might what user response in an app such as to delete something, exit teh app, shutdown the OS etc. The problem is, that IoT-Core does not support Popups. In that context, such an app prompts to shutdown or restart the OS or stop an app etc, using a PopUp message box, the app just skips through and does nothing, or may generate an unhandled error. 

This project demonstrates how to have a [Yes] [Cancel] Popup panel with a message withe the app awaiting teh user response and responding accordingly. The project code implements the confirmation message box both using the PopUp class and using the PopUp panel. A project compilation symbol (IOTCORE) determines which mechaism is used. That way the UWP app can run both on teh desktop (with expected user interaction) and on an IoT-Core device with eth PopUp panel.

## How to download:

Unfortunately, GitHub does not support downloading individual code. 

Navigate to [ms-iot/samples](https://github.com/ms-iot/samples) and select **Clone or download** to download the whole repository.


## Additional resources
* [Windows 10 IoT Core home page](https://developer.microsoft.com/en-us/windows/iot/)
* [Documentation for all samples](https://developer.microsoft.com/en-us/windows/iot/samples)

This project has adopted the Microsoft Open Source Code of Conduct. For more information see the Code of Conduct FAQ or contact opencode@microsoft.com with any additional questions or comments.
