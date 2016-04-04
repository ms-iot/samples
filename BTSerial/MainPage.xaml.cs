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
using Windows.Devices.Enumeration;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace BTSerial
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {

        private DeviceInformationCollection deviceCollection;
        private DeviceInformation selectedDevice;
        private RfcommDeviceService deviceService;

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

            if (selectedDevice == null)
            {
                errorStatus.Visibility = Visibility.Visible;
                errorStatus.Text = "Cannot find the device specified; Please check the device name";
                return;
            }
            else
            {
                deviceService = await RfcommDeviceService.FromIdAsync(selectedDevice.Id);

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
                        errorStatus.Text = "Cannot connect bluetooth device:" + ex.Message;
                    }

                }
                else
                {
                    errorStatus.Visibility = Visibility.Visible;
                    errorStatus.Text = "Didn't find the specified bluetooth device";
                }
            }
           
        }

        private async void SendData_Click(object sender, RoutedEventArgs e)
        {
            if (deviceService != null)
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
            else
            {
                errorStatus.Visibility = Visibility.Visible;
                errorStatus.Text = "Bluetooth is not connected correctly!";
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

            uint stringLength;
            uint actualStringLength;

            try
            {
                stringLength = dreader.ReadUInt32();
                actualStringLength = await dreader.LoadAsync(stringLength);

                if (stringLength != actualStringLength)
                {
                    return;
                }
                string text = dreader.ReadString(actualStringLength);

                message.Text = text;

            }
            catch (Exception ex)
            {
                errorStatus.Visibility = Visibility.Visible;
                errorStatus.Text = "Reading data from Bluetooth encountered error!"+ex.Message;
            }


        }
    }
}
