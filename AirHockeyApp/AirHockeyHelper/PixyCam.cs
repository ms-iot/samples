// Copyright (c) Microsoft. All rights reserved.
// Adapted from C++ sample code on cmucam.org

using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Windows.Devices.Enumeration;
using Windows.Devices.Spi;

namespace AirHockeyHelper
{
    public class PixyCamEventArgs : EventArgs
    {
        public ObjectBlock Block;
    }
    public class PixyCam
    {
        public SpiDevice Device;

        private bool skipStart;
        private Queue<byte> outBytes;

        uint PIXY_START_WORD = 0xaa55;
        uint PIXY_START_WORD_CC = 0xaa56;
        uint PIXY_START_WORDX = 0x55aa;

        byte PIXY_SERVO_SYNC = 0xff;
        byte PIXY_CAM_BRIGHTNESS_SYNC = 0xfe;
        byte PIXY_LED_SYNC = 0xfd;

        byte PIXY_SYNC_BYTE = 0x5a;
        byte PIXY_SYNC_BYTE_DATA = 0x5b;

        int PIXY_OUTBUF_SIZE = 1024;

        long lastBlockTime = 0;

        public PixyCam()
        {
            outBytes = new Queue<byte>();
        }

        public async Task Initialize()
        {
            var spiAqs = SpiDevice.GetDeviceSelector();
            var devicesInfo = await DeviceInformation.FindAllAsync(spiAqs);
            var settings = new SpiConnectionSettings(0);
            settings.ClockFrequency = 1000000;
            settings.DataBitLength = 8;
            settings.SharingMode = SpiSharingMode.Shared;

            Device = await SpiDevice.FromIdAsync(devicesInfo[0].Id, settings);
        }

        public void Close()
        {
            Device.Dispose();
        }

        public int GetStart()
        {
            uint curr, prev = 0xffff;

            while (true)
            {
                curr = getWord();

                if (curr == 0 && prev == 0)
                {
                    return 0;
                }
                else if (curr == PIXY_START_WORD && prev == PIXY_START_WORD)
                {
                    return 1;
                }
                else if (curr == PIXY_START_WORD_CC && prev == PIXY_START_WORD)
                {
                    return 1;
                }
                else if (curr == PIXY_START_WORDX)
                {
                    getByte();
                }

                prev = curr;
            }
        }

        private void getByte()
        {
            byte[] writeData = new byte[1] { 0x00 };
            byte[] readData = new byte[1];

            Device.TransferFullDuplex(writeData, readData);
        }

        private uint getWord()
        {
            byte[] writeData = new byte[2];
            byte[] readData = new byte[2];

            if (outBytes.Count > 0)
            {
                writeData[1] = PIXY_SYNC_BYTE_DATA;
                writeData[0] = outBytes.Dequeue();
            }
            else
            {
                writeData[1] = PIXY_SYNC_BYTE;
                writeData[0] = 0x00;
            }

            Device.TransferFullDuplex(writeData, readData);

            byte temp = readData[0];
            readData[0] = readData[1];
            readData[1] = temp;

            return BitConverter.ToUInt16(readData, 0);
        }

        public List<ObjectBlock> GetBlocks(uint maxBlocks)
        {
            uint checksum, w;
            List<ObjectBlock> objectBlocks = new List<ObjectBlock>();

            if (!skipStart)
            {
                if (GetStart() == 0)
                {
                    return null;
                }
            }
            else
            {
                skipStart = false;
            }

            for (int i = 0; i < maxBlocks;)
            {
                // After the sync sequence, the rest of the block is read in this order
                checksum = getWord();

                if (checksum == PIXY_START_WORD || checksum == PIXY_START_WORD_CC)
                {
                    skipStart = true;
                    return objectBlocks;
                }
                else if (checksum == 0)
                {
                    return objectBlocks;
                }

                ObjectBlock block = getBlock();

                // The checksum should be the sum of all the other values
                uint calculatedSum = block.Signature + block.X + block.Y + block.Width + block.Height;

                // Make sure that the checksum that came with the block and our sum match
                if (calculatedSum == checksum)
                {
                    var args = new PixyCamEventArgs();
                    args.Block = block;
                    objectBlocks.Add(block);
                    lastBlockTime = Global.Stopwatch.ElapsedMilliseconds;
                    i++;
                }

                w = getWord();
                if (w == PIXY_START_WORD || w == PIXY_START_WORD_CC)
                {

                }
                else
                {
                    return objectBlocks;
                }
            }

            return objectBlocks;
        }

        private ObjectBlock getBlock()
        {
            byte[] readData = getBytes(10);
            if (readData != null)
            {
                ObjectBlock block = new ObjectBlock();
                block.Signature = BitConverter.ToUInt16(readData, 0);
                block.X = BitConverter.ToUInt16(readData, 2);
                block.Y = BitConverter.ToUInt16(readData, 4);
                block.Width = BitConverter.ToUInt16(readData, 6);
                block.Height = BitConverter.ToUInt16(readData, 8);

                return block;
            }

            return null;
        }

        private byte[] getBytes(int num)
        {
            if (num % 2 != 0)
            {
                return null;
            }

            byte[] writeData = new byte[num];
            byte[] readData = new byte[num];

            for (int i = 0; i < num; i += 2)
            {
                if (outBytes.Count > 0)
                {
                    writeData[i] = outBytes.Dequeue();
                    writeData[i + 1] = PIXY_SYNC_BYTE_DATA;
                }
                else
                {
                    writeData[i] = 0x00;
                    writeData[i + 1] = PIXY_SYNC_BYTE;
                }
            }

            Device.TransferFullDuplex(writeData, readData);

            for (int i = 0; i < num; i += 2)
            {
                var temp = readData[i + 1];
                readData[i + 1] = readData[i];
                readData[i] = temp;
            }

            return readData;
        }

        private bool send(byte[] data)
        {
            if (data.Length > (PIXY_OUTBUF_SIZE - outBytes.Count))
            {
                return false;
            }

            foreach (byte d in data)
            {
                outBytes.Enqueue(d);
            }

            return true;
        }

        public bool SetBrightness(byte value)
        {
            return send(new byte[] { 0x00, PIXY_CAM_BRIGHTNESS_SYNC, value });
        }

        public bool SetLED(byte r, byte g, byte b)
        {
            return send(new byte[] { 0x00, PIXY_LED_SYNC, r, g, b });
        }

        public bool SetServos(UInt16 s0, UInt16 s1)
        {
            byte[] s0Array = BitConverter.GetBytes(s0);
            byte[] s1Array = BitConverter.GetBytes(s1);

            return send(new byte[]
                {
                    0x00,
                    PIXY_SERVO_SYNC,
                    s0Array[0],
                    s0Array[1],
                    s1Array[0],
                    s1Array[1]
                });
        }
    }
    public class ObjectBlock
    {
        public uint Signature, X, Y, Width, Height;
    }
}
