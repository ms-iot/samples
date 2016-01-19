#pragma once
#include "Adapter.h"

namespace AdapterLib
{
    namespace bhht = ::BackgroundHost::Headed::Tasks;
    namespace dsb = ::BridgeRT;
    namespace wamb = ::Windows::ApplicationModel::Background;
    namespace wfm = ::Windows::Foundation::Metadata;

    [wfm::WebHostHidden]
    public ref class HeadedBackgroundTask sealed : wamb::IBackgroundTask, dsb::IAdapterFactory
    {
    public:
        virtual void Run(wamb::IBackgroundTaskInstance^ taskInstance)
        {
            auto serviceImplementation = ref new dsb::AdapterBridgeServiceImplementation(this);

            auto task = ref new bhht::HeadedBackgroundTaskImplementation();
            task->Run(taskInstance, serviceImplementation);
        }

    public:
        virtual dsb::IAdapter^ CreateAdapter()
        {
            return ref new Adapter();
        }
    };
}
