// Copyright (c) Microsoft. All rights reserved.

const express = require("express");
const app = express();
const uwp = require("uwp")
uwp.projectNamespace("Windows");

var blinkyService;
var requestedPinValue;
const interval = 1000;

function connectAppService() {
	if (blinkyService === undefined) {
		blinkyService = new Windows.ApplicationModel.AppService.AppServiceConnection();

		blinkyService.appServiceName = "BlinkyService";
		blinkyService.packageFamilyName = "BlinkyService-uwp_gpek5j0d8wyr0";

		blinkyService.openAsync().done((status) => {
			if (status !== Windows.ApplicationModel.AppService.AppServiceConnectionStatus.success) {
				console.log("connection unsuccessful. status: " + status);
			} else {
				blinkyService.onserviceclosed = onAppServiceClosed;
			}
		}, (err) => {
		    if (err) {
		        console.log("openAsync error: " + err);
		    }
		});
	}
}

function onAppServiceClosed() {
	blinkyService = undefined;
}

function callAppService(requestedPinValue, res) {
	var result;
	var message = new Windows.Foundation.Collections.ValueSet();
	message.insert("requestedPinValue", requestedPinValue);

	blinkyService.sendMessageAsync(message).done((response) => {
		if (response.status === Windows.ApplicationModel.AppService.AppServiceResponseStatus.success) {
			// Get the data  that the service sent  to us.
			if (response.message["Status"] === "OK") {
				result = response.message["Result"];
			} else {
				console.log("sendMessageAsync error: " + response.message["Status"]);
			}
		}
	}, (err) => {
		if (err) {
			console.log("sendMessageAsync error: " + err);
		}
	});
}

// Try to connet to the app service when we start this app
connectAppService();

// Send message to BlinkyService every second to toggle the LED
setInterval(function () {
	if ("High" === requestedPinValue) {
		requestedPinValue = "Low";
	} else {
		requestedPinValue = "High";
	}
	callAppService(requestedPinValue);
}, interval);

