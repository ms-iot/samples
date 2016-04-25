using BackgroundHost.Headless;
using BridgeRT;
using Windows.ApplicationModel.Background;

namespace AdapterLib
{
    [Windows.Foundation.Metadata.WebHostHidden]
    public sealed class HeadlessStartupTask : IBackgroundTask, IAdapterFactory
    {
        public void Run(IBackgroundTaskInstance taskInstance)
        {
            var serviceImplementation = new AdapterBridgeServiceImplementation(this);

            taskImplementation = new HeadlessStartupTaskImplementation();
            taskImplementation.Run(taskInstance, serviceImplementation);
        }

        public IAdapter CreateAdapter()
        {
            return new Adapter();
        }

        private HeadlessStartupTaskImplementation taskImplementation;
    }
         
}
