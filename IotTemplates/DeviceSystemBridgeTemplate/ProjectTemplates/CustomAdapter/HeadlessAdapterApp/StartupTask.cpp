#include "pch.h"
#include "StartupTask.h"

using namespace Platform;
using namespace Windows::Foundation;
using namespace HeadlessAdapterApp;

void StartupTask::Run(IBackgroundTaskInstance^ taskInstance)
{
    AdapterLib::Adapter^ adapter = nullptr;

    deferral = taskInstance->GetDeferral();

    try
    {
        adapter = ref new AdapterLib::Adapter();

        dsbBridge = ref new BridgeRT::DsbBridge(adapter);

        HRESULT hr = this->dsbBridge->Initialize();
        if (FAILED(hr))
        {
            throw ref new Exception(hr, "DSB Bridge initialization failed!");
        }
    }
    catch (Exception^ ex)
    {

        if (dsbBridge != nullptr)
        {
            dsbBridge->Shutdown();
            dsbBridge = nullptr;
        }

        if (adapter != nullptr)
        {
            adapter->Shutdown();
            adapter = nullptr;
        }
    }
}