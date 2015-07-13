/*
    Copyright(c) Microsoft Open Technologies, Inc. All rights reserved.

    The MIT License(MIT)

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files(the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions :

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/

using System;
using System.Collections.ObjectModel;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.Devices.Enumeration;
using Windows.Devices.SerialCommunication;
using Windows.Storage.Streams;
using System.Threading;
using System.Threading.Tasks;

namespace SerialSample
{    
    public sealed partial class MainPage : Page
    {
        private SerialDevice serialPort = null;
        DataWriter DataWriteObject = null;
        DataReader DataReaderObject = null;

        private ObservableCollection<DeviceInformation> listOfDevices;
        private CancellationTokenSource ReadCancellationTokenSource;
       
        public MainPage()
        {
            this.InitializeComponent();            
            comPortInput.IsEnabled = false;
            sendTextButton.IsEnabled = false;
            listOfDevices = new ObservableCollection<DeviceInformation>();
            ListAvailablePorts();
        }

        private async void ListAvailablePorts()
        {
            try
            {
                string aqs = SerialDevice.GetDeviceSelector();
                var dis = await DeviceInformation.FindAllAsync(aqs);

                status.Text = "Select a device and connect";

                for (int i = 0; i < dis.Count; i++)
                {
                    listOfDevices.Add(dis[i]);
                }

                DeviceListSource.Source = listOfDevices;
                comPortInput.IsEnabled = true;
                ConnectDevices.SelectedIndex = -1;
            }
            catch (Exception ex)
            {
                status.Text = ex.Message;
            }
        }
        private async void comPortInput_Click(object sender, RoutedEventArgs e)
        {
            var selection = ConnectDevices.SelectedItems;

            if (selection.Count <= 0)
            {
                return;
            }

            DeviceInformation entry = (DeviceInformation)selection[0];         

            try
            {                
                serialPort = await SerialDevice.FromIdAsync(entry.Id);

                // Disable the 'OK' button 
                comPortInput.IsEnabled = false;

                // Configure serial settings
                serialPort.WriteTimeout = TimeSpan.FromMilliseconds(1000);
                serialPort.ReadTimeout = TimeSpan.FromMilliseconds(1000);                
                serialPort.BaudRate = 9600;
                serialPort.Parity = SerialParity.None;
                serialPort.StopBits = SerialStopBitCount.One;
                serialPort.DataBits = 8;

                // Display configured settings
                status.Text = "Serial port configured successfully!\n ----- Properties ----- \n";
                status.Text += "BaudRate: " + serialPort.BaudRate.ToString() + "\n";
                status.Text += "DataBits: " + serialPort.DataBits.ToString() + "\n";
                status.Text += "Handshake: " + serialPort.Handshake.ToString() + "\n";
                status.Text += "Parity: " + serialPort.Parity.ToString() + "\n";
                status.Text += "StopBits: " + serialPort.StopBits.ToString() + "\n";                
                status.Text += "FlowControl: None\n";                
                    
                // Set the RcvdText field to invoke the TextChanged callback
                // The callback launches an async Read task to wait for data
                rcvdText.Text = "Waiting for data...";

                // Create cancellation token object to close I/O operations when closing the device
                ReadCancellationTokenSource = new CancellationTokenSource();

                // Enable 'WRITE' button to allow sending data
                sendTextButton.IsEnabled = true;
            }
            catch (Exception ex)
            {
                status.Text = ex.Message;
                comPortInput.IsEnabled = true;
                sendTextButton.IsEnabled = false;
            }
        }        
       
        private async void sendTextButton_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                if (serialPort != null)
                {
                    DataWriteObject = new DataWriter(serialPort.OutputStream);
                    await WriteAsync();
                }
                else
                {
                    status.Text = "Enter serial port number before sending text\n";                
                }
            }
            catch (Exception ex)
            {
                status.Text = "sendTextButton_Click: " + ex.Message;
            }
            finally
            {
                if (DataWriteObject != null)
                {
                    DataWriteObject.DetachStream();
                    DataWriteObject = null;
                }
            }
        }

        private async Task WriteAsync()
        {
            Task<UInt32> storeAsyncTask;

            if (WriteBytesInputValue.Text.Length != 0)
            {
                char[] buffer = new char[WriteBytesInputValue.Text.Length];
                WriteBytesInputValue.Text.CopyTo(0, buffer, 0, WriteBytesInputValue.Text.Length);
                String InputString = new string(buffer);
                DataWriteObject.WriteString(InputString);
                WriteBytesInputValue.Text = "";

                storeAsyncTask = DataWriteObject.StoreAsync().AsTask();

                UInt32 bytesWritten = await storeAsyncTask;
                if (bytesWritten > 0)
                {
                    status.Text = InputString.Substring(0, (int)bytesWritten) + '\n';
                }
                status.Text += "Bytes written successfully!";
            }
            else
            {
                status.Text = "No input received to write";
            }
        }

        private async void rcvdText_TextChanged(object sender, TextChangedEventArgs e)
        {
            try
            {
                if (serialPort != null)
                {
                    DataReaderObject = new DataReader(serialPort.InputStream);
                    await ReadAsync(ReadCancellationTokenSource.Token);
                }
            }
            catch (Exception ex)
            {
                if (ex.GetType().Name == "TaskCanceledException")
                {
                    status.Text = "Reading task was cancelled, closing device and cleaning up";
                    CloseDevice();
                }
                else
                {
                    status.Text = ex.Message;
                }
            }
            finally
            {
                if (DataReaderObject != null)
                {
                    DataReaderObject.DetachStream();
                    DataReaderObject = null;
                }
            }
        }
        private async Task ReadAsync(CancellationToken cancellationToken)
        {
            Task<UInt32> loadAsyncTask;

            uint ReadBufferLength = 1024;

            cancellationToken.ThrowIfCancellationRequested();

            DataReaderObject.InputStreamOptions = InputStreamOptions.Partial;
            loadAsyncTask = DataReaderObject.LoadAsync(ReadBufferLength).AsTask(cancellationToken);

            UInt32 bytesRead = await loadAsyncTask;
            if (bytesRead > 0)
            {
                rcvdText.Text = DataReaderObject.ReadString(bytesRead);
            }
            status.Text = "\nBytes read successfully!";
        }

        private void CancelReadTask()
        {         
            if (ReadCancellationTokenSource != null)
            {
                if (!ReadCancellationTokenSource.IsCancellationRequested)
                {
                    ReadCancellationTokenSource.Cancel();
                }
            }         
        }

        private void CloseDevice()
        {            
            if (serialPort != null)
            {
                serialPort.Dispose();
            }
            serialPort = null;

            comPortInput.IsEnabled = true;
            sendTextButton.IsEnabled = false;            
            rcvdText.Text = "";
            listOfDevices.Clear();               
        }
        private void closeDevice_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                status.Text = "";
                CancelReadTask();
                CloseDevice();
                ListAvailablePorts();
            }
            catch (Exception ex)
            {
                status.Text = ex.Message;
            }          
        }        
    }
}
