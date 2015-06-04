/*
Copyright(c) Microsoft Open Technologies, Inc. All rights reserved.

The MIT License(MIT)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
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

	String^ aqs = I2cDevice::GetDeviceSelector("I2C1");
	auto deviceEnumTask = create_task(DeviceInformation::FindAllAsync(aqs));

	deviceEnumTask.then([this](DeviceInformationCollection^ devices) {
		auto getDeviceTask = create_task(I2cDevice::FromIdAsync(devices->GetAt(0)->Id, ref new I2cConnectionSettings(0x40)));
		getDeviceTask.then([this](I2cDevice^ device)
		{
			Device = device;
		});
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