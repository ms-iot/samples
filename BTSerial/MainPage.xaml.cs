using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

using Windows.Devices.Bluetooth;
using Windows.Devices.Bluetooth.Rfcomm;
using Windows.Networking.Sockets;
using Windows.Storage.Streams;
using System.Threading.Tasks;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace BTSerial
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {

        // The Chat Server's custom service Uuid: 34B1CF4D-1069-4AD6-89B6-E161D79BE4D8

        private Windows.Devices.Enumeration.DeviceInformationCollection deviceCollection;
        private Windows.Devices.Enumeration.DeviceInformation selectedDevice;
        Windows.Devices.Bluetooth.Rfcomm.RfcommDeviceService deviceService;

        public string deviceName = "RNBT-76B7"; // Specify the device name to be selected; You can find the device name from the webb under bluetooth 

        StreamSocket streamSocket = new StreamSocket();

        public MainPage()
        {
            this.InitializeComponent();
            InitializeRfcommServer();

        }
       
        private void ConnectDevice_Click(object sender, RoutedEventArgs e)
        {
            ConnectToDevice();
        }

        private async void InitializeRfcommServer()
        {
            try
            {
                string device1=RfcommDeviceService.GetDeviceSelector(RfcommServiceId.SerialPort);
                deviceCollection = await Windows.Devices.Enumeration.DeviceInformation.FindAllAsync(device1);
            }
            catch (Exception exception)
            {
                errorStatus.Visibility = Visibility.Visible;
                errorStatus.Text = exception.Message;
            }
        }

        private async void ConnectToDevice()
        {
            foreach(var item in deviceCollection)
            {
                if (item.Name == deviceName)
                {
                    selectedDevice = item;
                    break;
                }
            }

            var deviceService= await Windows.Devices.Bluetooth.Rfcomm.RfcommDeviceService.FromIdAsync(selectedDevice.Id);
            
            if (deviceService != null)
            {
                //connect the socket   
                try
                {
                    await streamSocket.ConnectAsync(deviceService.ConnectionHostName, deviceService.ConnectionServiceName);
                }
                catch (Exception ex)
                {
                    errorStatus.Visibility = Visibility.Visible;
                    errorStatus.Text = "Cannot connect bluetooth device:"+ex.Message;
                }
                          
            }
            else
            {
                errorStatus.Visibility = Visibility.Visible;
                errorStatus.Text = "Didn't find the specified bluetooth device";
            }
           
        }

        private async void SendData_Click(object sender, RoutedEventArgs e)
        {

            //send data
            string sendData = messagesent.Text;
            if (string.IsNullOrEmpty(sendData))
            {
                errorStatus.Visibility = Visibility.Visible;
                errorStatus.Text = "Please specify the string you are going to send";
            }
            else
            {
                DataWriter dwriter = new DataWriter(streamSocket.OutputStream);
                UInt32 len = dwriter.MeasureString(sendData);
                dwriter.WriteUInt32(len);
                dwriter.WriteString(sendData);
                await dwriter.StoreAsync();
                await dwriter.FlushAsync();
            }
        
        }

        private async void ReceiveData_Click(object sender, RoutedEventArgs e)
        {
            // read the data

            DataReader dreader = new DataReader(streamSocket.InputStream);
            uint sizeFieldCount = await dreader.LoadAsync(sizeof(uint));
            if (sizeFieldCount != sizeof(uint))
            {
                return;
            }

            uint stringLength = dreader.ReadUInt32();
            uint actualStringLength = await dreader.LoadAsync(stringLength);
            if(stringLength != actualStringLength)
            {
                return;
            }
            string text = dreader.ReadString(actualStringLength);

            message.Text = text;

        }
    }
}
