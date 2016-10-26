// Copyright (c) Microsoft. All rights reserved.

#include "pch.h"
#include "StartupTask.h"

using namespace WeatherStation;

using namespace Platform;
using namespace Windows::ApplicationModel::Background;
using namespace Windows::Foundation;
using namespace Windows::Devices::I2c;
using namespace Windows::Devices::Enumeration;
using namespace Windows::System::Threading;
using namespace concurrency;

StartupTask::StartupTask()
{
}

void StartupTask::Run(IBackgroundTaskInstance^ taskInstance)
{
	Deferral = taskInstance->GetDeferral();

	auto controllerTask = create_task(I2cController::GetDefaultAsync());
	controllerTask.then([this](I2cController^ controller) {
		Device = controller->GetDevice(ref new I2cConnectionSettings(0x40));
	});
	


	TimerElapsedHandler ^handler = ref new TimerElapsedHandler(
		[this](ThreadPoolTimer ^timer)
	{
		wchar_t humidityOutput[100];
		wchar_t temperatureOutput[100];

		double humidity = GetHumidity();
		swprintf_s(humidityOutput, 100, L"Humidity: %f\n", humidity);
		OutputDebugStringW(humidityOutput);

		
		double temperature = GetTemperature();
		swprintf_s(temperatureOutput, 100, L"Temperature: %f\n", temperature);
		OutputDebugStringW(temperatureOutput);



		
	});

	TimeSpan interval;
	interval.Duration = 1000 * 1000 * 10;
	Timer = ThreadPoolTimer::CreatePeriodicTimer(handler, interval);
}

double StartupTask::GetTemperature() 
{
	Platform::Array<byte>^ command = ref new Platform::Array<byte>(1);
	command[0] = 0xE3;
	Array<byte>^ data = ref new Array<byte>(2);
	Device->WriteRead(command, data);
	auto rawReading = data[0] << 8 | data[1];
	auto ratio = rawReading / (float)65536;
	double temperature = (-46.85 + (175.72 * ratio)) * 9 / 5 + 32;
	return temperature;
}

double StartupTask::GetHumidity()
{
	Platform::Array<byte>^ command = ref new Platform::Array<byte>(1);
	command[0] = 0xE5;
	Array<byte>^ data = ref new Array<byte>(2);
	Device->WriteRead(command, data);
	auto rawReading = data[0] << 8 | data[1];
	auto ratio = rawReading / (float)65536;
	double humidity = -6 + (125 * ratio);
	return humidity;
}