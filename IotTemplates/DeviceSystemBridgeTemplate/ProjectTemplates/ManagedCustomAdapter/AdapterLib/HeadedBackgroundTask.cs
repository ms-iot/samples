using BackgroundHost.Headed.Tasks;
using BridgeRT;
using Windows.ApplicationModel.Background;

namespace AdapterLib
{
    public sealed class HeadedBackgroundTask : IBackgroundTask, IAdapterFactory
    {      
        public void Run(IBackgroundTaskInstance taskInstance)
        {
            var serviceImplementation = new AdapterBridgeServiceImplementation(this);

            var task = new HeadedBackgroundTaskImplementation();
            task.Run(taskInstance, serviceImplementation);
        }

        public IAdapter CreateAdapter()
        {
            return new Adapter();
        }
    }
}