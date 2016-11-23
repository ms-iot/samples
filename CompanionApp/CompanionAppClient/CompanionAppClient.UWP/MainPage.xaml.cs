namespace CompanionAppClient.UWP
{
    public sealed partial class MainPage
    {
        public MainPage()
        {
            this.InitializeComponent();

            LoadApplication(new CompanionAppClient.App());
        }
    }
}
