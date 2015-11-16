// Copyright (c) 2015, Microsoft Corporation
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
// SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
// IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//

using System;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;
using Windows.Devices.Enumeration;
using Windows.Devices.SerialCommunication;
using Windows.Storage.Streams;

namespace AdapterLib
{
    class SerialController
    {
        private SerialDevice m_device = null;
        private CancellationTokenSource m_readCancellationTokenSource = null;
        private Task m_readTask = null;
        public Action<byte[], int> OnByteReception = null;

        private struct VidPidPair
        {
            public ushort vid;
            public ushort pid;

            public VidPidPair(ushort vid, ushort pid)
            {
                this.vid = vid;
                this.pid = pid;
            }
        }

        private VidPidPair[] m_validVidPidList =
        {
            new VidPidPair(0x0403, 0x6001),     // XBee module with SparkFun USB dongle ref 1 (large USB connector)
            new VidPidPair(0x0403, 0x6015)      // XBee module with SparkFun USB dongle ref 2 (mini USB connector)
        };
        public async Task InitializeAsync()
        {
            try
            {
                string deviceId = await GetDeviceIdFromUsbVidPid();
                m_device = await SerialDevice.FromIdAsync(deviceId);

                if (null == m_device)
                {
                    throw new InvalidOperationException("No Xbee dongle");
                }

                // initialize serial communication parameters
                m_device.BaudRate = 9600;
                m_device.DataBits = 8;
                m_device.Parity = SerialParity.None;
                m_device.StopBits = SerialStopBitCount.One;
                m_device.Handshake = SerialHandshake.None;
                m_device.WriteTimeout = TimeSpan.FromMilliseconds(500);
                m_device.ReadTimeout = TimeSpan.FromMilliseconds(500);
            }
            catch
            {
                throw;
            }

            m_readCancellationTokenSource = new CancellationTokenSource();
            StartReading();
        }

        private async Task<string> GetDeviceIdFromUsbVidPid()
        {
            DeviceInformationCollection deviceInfos = null;
            string selector;

            // go through all valid Vid/Pid pair and try GetDeviceSelectorFromUsbVidPid()
            foreach (var vidPidPair in m_validVidPidList)
            {
                selector = SerialDevice.GetDeviceSelectorFromUsbVidPid(vidPidPair.vid, vidPidPair.pid);
                deviceInfos = await DeviceInformation.FindAllAsync(selector);
                if (deviceInfos.Count > 0)
                {
                    return deviceInfos[0].Id;
                }
            }

            // Enumerate all devices and find VID and PID in device IDs
            // e.g. "\\\\?\\FTDIBUS#VID_0403+PID_6001+A603GDNAA#0000#{86e0d1e0-8089-11d0-9ce4-08003e301f73}"
            selector = SerialDevice.GetDeviceSelector();
            deviceInfos = await DeviceInformation.FindAllAsync(selector);
            if (deviceInfos.Count > 0)
            {
                // go through all valid Vid/Pid pair
                foreach (var vidPidPair in m_validVidPidList)
                {
                    string vidString = "VID_" + vidPidPair.vid.ToString("X4");
                    string pidString = "PID_" + vidPidPair.pid.ToString("X4");
                    foreach (var deviceInfo in deviceInfos)
                    {
                        if (deviceInfo.Id.Contains(vidString) && deviceInfo.Id.Contains(pidString))
                        {
                            return deviceInfo.Id;
                        }
                    }
                }
            }

            // can't find any matching Vid/Pid 
            return null;
        }


        private const int TIMEOUT_EXITREADTASK = 1000;
        public void Shutdown()
        {
            if (null != m_readCancellationTokenSource)
            {
                if (!m_readCancellationTokenSource.IsCancellationRequested)
                {
                    m_readCancellationTokenSource.Cancel();
                }
            }

            if (null != m_device)
            {
                if (null != m_readTask)
                {
                    // Wait for read task to exit gracefully
                    m_readTask.Wait(TIMEOUT_EXITREADTASK);
                }
                m_device.Dispose();
                m_device = null;
            }
        }

        public async Task WriteAsync(byte[] buffer)
        {
            if (null == m_device)
            {
                return;
            }

            Task<UInt32> storeAsyncTask = null;
            DataWriter dataWriter = new DataWriter(m_device.OutputStream);

            try
            {
                dataWriter.WriteBytes(buffer);
                storeAsyncTask = dataWriter.StoreAsync().AsTask();
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex);
            }

            if (null != storeAsyncTask)
            {
                await storeAsyncTask;
            }

            if (null != dataWriter)
            {
                dataWriter.DetachStream();
                dataWriter = null;
            }
        }

        private const int MAX_BUFFER = 256;

        private async void StartReading()
        {
            m_readTask = Task.Run(async () =>
            {
                Task<UInt32> loadAsyncTask = null;

                DataReader dataReader = new DataReader(m_device.InputStream);
                dataReader.InputStreamOptions = InputStreamOptions.Partial;

                while (null != m_readCancellationTokenSource && !m_readCancellationTokenSource.IsCancellationRequested)
                {
                    loadAsyncTask = dataReader.LoadAsync(MAX_BUFFER).AsTask(m_readCancellationTokenSource.Token);

                    uint nbOfLoadedBytes = 0;
                    try
                    {
                        nbOfLoadedBytes = await loadAsyncTask;
                    }
                    catch (TaskCanceledException)
                    {
                        // Ignore
                    }
                    catch
                    {
                        throw;
                    }

                    if (nbOfLoadedBytes > 0)
                    {
                        byte[] buffer = null;

                        try
                        {
                            buffer = new byte[nbOfLoadedBytes];
                            dataReader.ReadBytes(buffer);
                        }
                        catch (Exception ex)
                        {
                            Debug.WriteLine(ex);
                            throw ex;
                        }

                        if (null != OnByteReception)
                        {
                            OnByteReception(buffer, (int)nbOfLoadedBytes);
                        }
                    }
                }

                if (null != dataReader)
                {
                    dataReader.DetachStream();
                    dataReader = null;
                }
            });

            await m_readTask;
        }
    }
}
