using Windows.ApplicationModel.Background;

// The Background Application template is documented at http://go.microsoft.com/fwlink/?LinkID=533884&clcid=0x409

namespace CompanionAppServer
{
    public sealed class Foo
    {
        public string Bar { get; set; }
    }
    public sealed class StartupTask : IBackgroundTask
    {
        BackgroundTaskDeferral deferral;
        public async void Run(IBackgroundTaskInstance taskInstance)
        {
            deferral = taskInstance.GetDeferral();

            var server = new Server();
            await server.Start();
        }
    }
}
