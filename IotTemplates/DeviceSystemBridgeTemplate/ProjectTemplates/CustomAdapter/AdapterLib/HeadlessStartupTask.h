#pragma once
#include "Adapter.h"

namespace AdapterLib
{
    namespace bhh = ::BackgroundHost::Headless;
    namespace dsb = ::BridgeRT;
    namespace wamb = ::Windows::ApplicationModel::Background;
    namespace wfm = ::Windows::Foundation::Metadata;

    [wfm::WebHostHidden]
    public ref class HeadlessStartupTask sealed : wamb::IBackgroundTask, dsb::IAdapterFactory
    {
    public:
        virtual void Run(wamb::IBackgroundTaskInstance^ taskInstance)
        {
            auto serviceImplementation = ref new dsb::AdapterBridgeServiceImplementation(this);

            taskImplementation = ref new bhh::HeadlessStartupTaskImplementation();
            taskImplementation->Run(taskInstance, serviceImplementation);
        }

    public:
        virtual dsb::IAdapter^ CreateAdapter()
        {
            return ref new Adapter();
        }

    private:
        bhh::HeadlessStartupTaskImplementation^ taskImplementation;
    };
}
