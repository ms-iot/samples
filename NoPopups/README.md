Windows 10 IoT Core sample code
===============
With a UWP UI app you might what a user response in an app to a prompt to delete something, exit the app, shutdown the OS etc. The problem is, that IoT-Core does not support the Popup class. In that context, with app prompts, using a PopUp message box, the app just skips through and does nothing, or may generate an unhandled error. 

This project demonstrates how to have a [Yes] [Cancel] Popup panel with a message with the app awaiting the user response and responding accordingly.


[Documentation for this sample](https://github.com/djaus2/samples/blob/develop/NoPopups/NoPopUps.md) 

## How to download:

Unfortunately, GitHub does not support downloading individual code. 

Navigate to [ms-iot/samples](https://github.com/ms-iot/samples) and select **Clone or download** to download the whole repository.


## Additional resources
* [Windows 10 IoT Core home page](https://developer.microsoft.com/en-us/windows/iot/)
* [Documentation for all samples](https://developer.microsoft.com/en-us/windows/iot/samples)

This project has adopted the Microsoft Open Source Code of Conduct. For more information see the Code of Conduct FAQ or contact opencode@microsoft.com with any additional questions or comments.
