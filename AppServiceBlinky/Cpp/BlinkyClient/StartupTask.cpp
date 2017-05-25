#include "pch.h"
#include "StartupTask.h"

using namespace BlinkyClient;

using namespace Platform;
using namespace Windows::ApplicationModel::Background;
using namespace Windows::ApplicationModel::AppService;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::System::Threading;
using namespace Windows::Devices::Gpio;

// The Background Application template is documented at http://go.microsoft.com/fwlink/?LinkID=533884&clcid=0x409

void StartupTask::Run(IBackgroundTaskInstance^ taskInstance)
{
    serviceDeferral = taskInstance->GetDeferral();

    serviceConnection = ref new AppServiceConnection();
    serviceConnection->PackageFamilyName = L"BlinkyService-uwp_2yx4q2bk84nj4";
    serviceConnection->AppServiceName = L"BlinkyService";

    auto connectTask = Concurrency::create_task(serviceConnection->OpenAsync());
    connectTask.then([this](AppServiceConnectionStatus connectStatus)
    {
        if (AppServiceConnectionStatus::Success == connectStatus)
        {
            auto message = ref new ValueSet();
            command = L"SetLedStateOn";
            message->Insert(L"SetLedState", command);
            message->Insert(L"PinNumber", 5);
            auto messageTask = Concurrency::create_task(serviceConnection->SendMessageAsync(message));

            messageTask.then([this](AppServiceResponse ^appServiceResponse)
            {
                if ((appServiceResponse->Status == AppServiceResponseStatus::Success) &&
                    (appServiceResponse->Message != nullptr) &&
                    (appServiceResponse->Message->HasKey(L"Response")))
                {
                    auto response = appServiceResponse->Message->Lookup(L"Response");
                    String^ responseMessage = safe_cast<String^>(response);
                    if (String::operator==(responseMessage, L"Success"))
                    {
                        TimerElapsedHandler ^handler = ref new TimerElapsedHandler(
                            [this](ThreadPoolTimer ^timer)
                        {
                            RequestBlinkService();
                        });

                        TimeSpan interval;
                        interval.Duration = 500 * 1000 * 10;
                        timer = ThreadPoolTimer::CreatePeriodicTimer(handler, interval);
                    }
                }
            });
        }
        else
        {
            serviceDeferral->Complete();
        }
    });
}

void StartupTask::RequestBlinkService()
{
    command = String::operator==(command, L"SetLedStateOn") ? L"SetLedStateOff" : L"SetLedStateOn";
    auto message = ref new ValueSet();
    message->Insert(L"SetLedState", command);
    message->Insert(L"PinNumber", 5);
    auto messageTask = Concurrency::create_task(serviceConnection->SendMessageAsync(message));
}

void BlinkyClient::StartupTask::OnCanceled(Windows::ApplicationModel::Background::IBackgroundTaskInstance ^sender, Windows::ApplicationModel::Background::BackgroundTaskCancellationReason reason)
{
    if (serviceDeferral != nullptr)
    {
        serviceDeferral->Complete();
    }

    if (serviceConnection != nullptr)
    {
        serviceConnection = nullptr;
    }
}