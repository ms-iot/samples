//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

using System.Threading.Tasks;
using Windows.Devices.SmartCards;
using System.Collections.Generic;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Storage.Streams;
using System.Diagnostics;
using System.Linq;

using Pcsc;
using System;

namespace MifareUltralight
{
    /// <summary>
    /// Access handler class for MifareUL based ICC. It provides wrappers for different MifareUL 
    /// commands
    /// </summary>
    public class AccessHandler
    {



        public const int SerialNumberTopPage = 0;
        public const int SerialNumberBottomPage = 1;
        public const int StaticLockPage = 2;
        public const int CapabilityPage = 3;
        public const int PasswordPage = 229;

        public const int MemorySizeByte = 2;

        public enum NTagType
        {
            Not,
            NTAG213,
            NTAG215,
            NTAG216
        };

        public struct NTagData
        {
            public byte ConfigurationPage;
            public byte AccessPage;
            public byte PasswordPage;
            public byte PasswordAckPage;
        }

        public readonly Dictionary<NTagType, NTagData> NTagInfo = new Dictionary<NTagType, NTagData>()
        {
            {  NTagType.NTAG213, new NTagData() { ConfigurationPage = 0x29, AccessPage = 0x2A, PasswordPage = 0x2b, PasswordAckPage = 0x2C } },
            {  NTagType.NTAG215, new NTagData() { ConfigurationPage = 0x83, AccessPage = 0x84, PasswordPage = 0x85, PasswordAckPage = 0x86 } },
            {  NTagType.NTAG216, new NTagData() { ConfigurationPage = 0xE3, AccessPage = 0xE4, PasswordPage = 0xE5, PasswordAckPage = 0xE6 } },
        };

        public uint MemorySize { get; internal set; } = 0;
        public uint Blocks { get; internal set; } = 0;
        public uint Pages { get; internal set; } = 0;

        public NTagType NTag { get; internal set; } = NTagType.Not;

        public bool isNTag21x { get; internal set; } = false;

        /// <summary>
        /// connection object to smart card
        /// </summary>
        private SmartCardConnection connectionObject { set; get; }
        /// <summary>
        /// Class constructor
        /// </summary>
        /// <param name="ScConnection">
        /// connection object to a MifareUL ICC
        /// </param>
        public AccessHandler(SmartCardConnection ScConnection)
        {
            connectionObject = ScConnection;
        }

        public async Task ReadCapsAsync()
        {
            byte[] command = { 0x60 };
            var response = await connectionObject.TransparentExchangeAsync(command);
            if (response.Length > 7)
            {
                if (response[1] == 0x4 &&   // NXP
                    response[2] == 0x4 &&   // NTAG
                    response[3] == 0x2)  // 50pf 
                {
                    isNTag21x = true;

                    MemorySize = response[6] * 8u;

                    switch (response[6])
                    {
                        case 0x0F:
                            NTag = NTagType.NTAG213;
                            MemorySize = 180;
                            break;
                        case 0x11:
                            NTag = NTagType.NTAG215;
                            MemorySize = 540;
                            break;
                        case 0x13:
                            NTag = NTagType.NTAG216;
                            MemorySize = 924;
                            break;
                        default:
                            isNTag21x = false;
                            NTag = NTagType.Not;
                            break;
                    }
                    Blocks = MemorySize / 16 + 1;
                    Pages = MemorySize / 4;
                }
            }

        }

        /// <summary>
        /// Wrapper method to read 16 bytes (4 pages) starting at pageAddress
        /// </summary>
        /// <param name="pageAddress">
        /// start page to read
        /// </param>
        /// <returns>
        /// byte array of 16 bytes
        /// </returns>
        public async Task<byte[]> ReadAsync(byte pageAddress)
        {
            var apduRes = await connectionObject.TransceiveAsync(new MifareUltralight.Read(pageAddress));

            if (!apduRes.Succeeded)
            {
                throw new Exception("Failure reading MIFARE Ultralight card, " + apduRes.ToString());
            }

            return apduRes.ResponseData;
        }
        /// <summary>
        /// Wrapper method write 4 bytes at the pageAddress
        /// </param name="pageAddress">
        /// page address to write
        /// </param>
        /// byte array of the data to write
        /// </returns>
        public async void WriteAsync(byte pageAddress, byte[] data)
        {
            if (data.Length != 4)
            {
                throw new NotSupportedException();
            }

            var apduRes = await connectionObject.TransceiveAsync(new MifareUltralight.Write(pageAddress, ref data));

            if (!apduRes.Succeeded)
            {
                throw new Exception("Failure writing MIFARE Ultralight card, " + apduRes.ToString());
            }
        }
        /// <summary>
        /// Wrapper method to perform transparent transceive data to the MifareUL card
        /// </summary>
        /// <param name="commandData">
        /// The command to send to the MifareUL card
        /// </param>
        /// <returns>
        /// byte array of the read data
        /// </returns>
        public async Task<byte[]> TransparentExchangeAsync(byte[] commandData)
        {
            byte[] responseData = await connectionObject.TransparentExchangeAsync(commandData);

            return responseData;
        }
        /// <summary>
        /// Wrapper method get the MifareUL ICC UID
        /// </summary>
        /// <returns>
        /// byte array UID
        /// </returns>
        public async Task<byte[]> GetUidAsync()
        {
            var apduRes = await connectionObject.TransceiveAsync(new MifareUltralight.GetUid());

            if (!apduRes.Succeeded)
            {
                throw new Exception("Failure getting UID of MIFARE Ultralight card, " + apduRes.ToString());
            }

            return apduRes.ResponseData;
        }

        public async Task<uint> GetAccessCountAsync()
        {
            var transRet = await connectionObject.TransparentExchangeAsync(new byte[] { 0x39, 0x2 });
            if (transRet.Length >= 3)
            {
                byte[] convert = new byte[4];
                convert[0] = transRet[0];
                convert[1] = transRet[1];
                convert[2] = transRet[2];

                return BitConverter.ToUInt32(convert, 0);
            }
            return uint.MaxValue;
        }

        public async Task EnableAccessCountAsync()
        {
            byte[] accessPage = await ReadAsync(NTagInfo[NTag].AccessPage);
            byte[] writePage = new byte[4];

            // 4rd bit in 4th byte
            writePage[0] = (byte)(accessPage[0] & ~0x8 | 0x10);
            writePage[1] = accessPage[1];
            writePage[2] = accessPage[2];
            writePage[3] = accessPage[3];


            WriteAsync(NTagInfo[NTag].AccessPage, writePage);
        }

        public async Task ProvisionPassword(bool authFoWriteOnly, int authLimit, byte[] Password, byte[] PasswordAcknowledge)
        {
            try
            {
                bool prot = authFoWriteOnly; // false = PWD_AUTH needed for write only, true = PWD_AUTH needed for read and write
                int authlim = authLimit; // Value between 0 and 7, 1-7 = number of PWD_AUTH attempts allowed, 0 = unlimited number of attempts
                byte auth0 = 0x4; 

                byte[] command = null;
                byte[] response = null;

                command = new byte[] {
                    0xA2, // WRITE
                    NTagInfo[NTag].PasswordPage,
                    Password[0], Password[1], Password[2], Password[3] // Password
                };
                Debug.WriteLine("Writing to Password Page: " + BitConverter.ToString(command));
                response = await connectionObject.TransparentExchangeAsync(command);
                Debug.WriteLine("Password WRITE response: " + BitConverter.ToString(response));

                command = new byte[] {
                    0xA2, // WRITE
                    NTagInfo[NTag].PasswordAckPage,
                    PasswordAcknowledge[0], PasswordAcknowledge[1], // Password acknowledge
                    0x00, 0x00 // RFU
                };
                Debug.WriteLine("Writing to Password Ack Page: " + BitConverter.ToString(command));
                response = await connectionObject.TransparentExchangeAsync(command);
                Debug.WriteLine("Password acknowledge WRITE response: " + BitConverter.ToString(response));

                command = new byte[] {
                    0x30, // READ
                    NTagInfo[NTag].AccessPage
                };
                Debug.WriteLine("Reading from Access Page: " + BitConverter.ToString(command));
                response = await connectionObject.TransparentExchangeAsync(command);
                Debug.WriteLine("READ Access Page response: " + BitConverter.ToString(response));

                if (response == null || response.Length < 16)
                {
                    throw new Exception("Invalid READ AccessPage response");
                }

                byte accessByte = (byte)(response[0] & 0x87);
                accessByte |= (byte)(prot ? 0x80 : 0x00);
                accessByte |= (byte)(authlim & 0x07);

                command = new byte[] {
                    0xA2, // WRITE
                    NTagInfo[NTag].AccessPage,
                    accessByte, // Set PROT and AUTHLIM in ACCESS byte
                    response[1], response[2], response[3] // Keep old values the same
                };
                Debug.WriteLine("Writing to Access Page: " + BitConverter.ToString(command));
                response = await connectionObject.TransparentExchangeAsync(command);
                Debug.WriteLine("WRITE ACCESS byte response: " + BitConverter.ToString(response));

                command = new byte[] {
                    0x30, // READ
                    NTagInfo[NTag].ConfigurationPage // Config
                };
                Debug.WriteLine("Reading from Config page: " + BitConverter.ToString(command));
                response = await connectionObject.TransparentExchangeAsync(command);
                Debug.WriteLine("READ Config pageresponse: " + BitConverter.ToString(response));

                if (response == null || response.Length < 16)
                {
                    throw new Exception("Invalid READ Access Page response");
                }

                command = new byte[] {
                    0xA2, // WRITE
                    NTagInfo[NTag].ConfigurationPage,
                    response[0], response[1], response[2], // Keep old values the same
                    auth0 // Set AUTH0
                };
                Debug.WriteLine("Writing to Config Page: " + BitConverter.ToString(command));
                response = await connectionObject.TransparentExchangeAsync(command);
                Debug.WriteLine("WRITE AUTH0 byte response: " + BitConverter.ToString(response));
            }
            catch (Exception ex)
            {
                Debug.WriteLine("Exception sending transparent commands: " + ex);
            }
        }

        public async Task<bool> AuthenticateWithPassword(byte[] Password, byte[] PasswordAcknowledge)
        {
            bool authenticated = false;

            try
            {
                byte[] command = null;
                byte[] response = null;

                command = new byte[] {
                    0x1B, // PWD_AUTH
                    Password[0], Password[1], Password[2], Password[3] // Password
                };
                Debug.WriteLine("PWD_AUTH command: " + BitConverter.ToString(command));
                response = await connectionObject.TransparentExchangeAsync(command);
                Debug.WriteLine("PWD_AUTH response: " + BitConverter.ToString(response));

                if (!PasswordAcknowledge.SequenceEqual(response))
                {
                    Debug.WriteLine("Password acknowledge incorrect");
                }
                else
                {
                    Debug.WriteLine("Password acknowledge correct");
                    authenticated = true;
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine("Exception sending transparent commands: " + ex);
            }

            return authenticated;
        }
    }
}
