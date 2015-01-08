// © Copyright(C) Microsoft. All rights reserved.

using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Input;


namespace bertha
{
    public sealed partial class ConnectToPC : Page
    {
        public ConnectToPC()
        {
            this.InitializeComponent();
            DeviceInfoPresenter presenter = new DeviceInfoPresenter();
            ConnectText.Text = string.Format(ConnectText.Text, presenter.GetBoardName());
        }

        private void BackButton_Tapped(object sender, TappedRoutedEventArgs e)
        {
            NavigationUtils.GoBack();
        }
    }
}
