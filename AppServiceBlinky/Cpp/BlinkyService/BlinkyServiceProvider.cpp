#include "pch.h"
#include "BlinkyServiceProvider.h"

using namespace BlinkyService;

using namespace Platform;
using namespace Windows::ApplicationModel::AppService;
using namespace Windows::ApplicationModel::Background;
using namespace Windows::Devices::Gpio;
using namespace Windows::Foundation::Collections;

// The Background Application template is documented at http://go.microsoft.com/fwlink/?LinkID=533884&clcid=0x409

void BlinkyServiceProvider::Run(IBackgroundTaskInstance^ taskInstance)
{
    serviceDeferral = taskInstance->GetDeferral();

    taskInstance->Canceled += ref new Windows::ApplicationModel::Background::BackgroundTaskCanceledEventHandler(this, &BlinkyService::BlinkyServiceProvider::OnCanceled);

    auto appServiceTrigger = dynamic_cast<AppServiceTriggerDetails^>(taskInstance->TriggerDetails);

    if (appServiceTrigger != nullptr)
    {
        if (String::operator==(appServiceTrigger->Name, L"BlinkyService"))
        {
            controller = GpioController::GetDefault();
            serviceConnection = appServiceTrigger->AppServiceConnection;
            serviceConnection->RequestReceived += ref new Windows::Foundation::TypedEventHandler<Windows::ApplicationModel::AppService::AppServiceConnection ^, Windows::ApplicationModel::AppService::AppServiceRequestReceivedEventArgs ^>(this, &BlinkyService::BlinkyServiceProvider::OnRequestReceived);
        }
        else
        {
            serviceDeferral->Complete();
        }
    }
}

void BlinkyService::BlinkyServiceProvider::OnCanceled(Windows::ApplicationModel::Background::IBackgroundTaskInstance ^sender, Windows::ApplicationModel::Background::BackgroundTaskCancellationReason reason)
{
    if (serviceDeferral != nullptr)
    {
        serviceDeferral->Complete();
    }
}

void BlinkyService::BlinkyServiceProvider::OnRequestReceived(Windows::ApplicationModel::AppService::AppServiceConnection ^sender, Windows::ApplicationModel::AppService::AppServiceRequestReceivedEventArgs ^args)
{
    auto messageDeferral = args->GetDeferral();

    auto responseMessage = ref new ValueSet();

    if (args->Request->Message != nullptr)
    {
        if ((args->Request->Message->HasKey(L"SetLedState")) && (args->Request->Message->HasKey(L"PinNumber")))
        {
            try
            {
                auto commandObject = args->Request->Message->Lookup(L"SetLedState");
                String^ command = safe_cast<String^>(commandObject);
                auto pinNumberObject = args->Request->Message->Lookup(L"PinNumber");
                int pinNumber = safe_cast<int>(pinNumberObject);
                auto pin = controller->OpenPin(pinNumber);
                pin->SetDriveMode(GpioPinDriveMode::Output);

                if (String::operator==(command, L"SetLedStateOn"))
                {
                    pin->Write(GpioPinValue::High);
                    responseMessage->Insert(L"Response", L"Success");
                }
                else if (String::operator==(command, L"SetLedStateOff"))
                {
                    pin->Write(GpioPinValue::Low);
                    responseMessage->Insert(L"Response", L"Succes");
                }
                else
                {
                    responseMessage->Insert(L"Response", L"Request Failed:Invalid SetLedState parameter value.");
                }
            }
            catch (InvalidCastException^ ic)
            {
                responseMessage->Insert(L"Response", L"Request Failed:Invalid cast exception occurred.");
            }
            catch (Exception^ e)
            {
                responseMessage->Insert(L"Response", L"Request Failed:Unknown exception occurred.");
            }
        }
        else
        {
            responseMessage->Insert(L"Response", L"Request Failed:Invalid request.");
        }
    }
    else
    {
        responseMessage->Insert(L"Response", L"Failed: Request message is empty.");
    }

    args->Request->SendResponseAsync(responseMessage);

    messageDeferral->Complete();
}
