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
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Threading;
using Windows.System.Threading;
using System.Diagnostics;

using AdapterLib;

namespace AdapterLibTest
{
    class TestMain
    {
        private static UInt64 DRESDEN_ELEKTRONIK_BALLAST_MAC_ADDRESS = 0x00212EFFFF006C95;
        private static byte DRESDEN_ELEKTRONIK_BALLAST_END_POINT = 0x0A;

        private static UInt64 BITRON_HOME_DOOR_SENSOR_MAC_ADDRESS = 0x124B000416FEAB;
        private static byte BITRON_HOME_DOOR_SENSOR_END_POINT = 0x01;


        private Adapter m_zigBeeAdapter = null;
        private ZigBeeDevice m_testedDevice = null;
        private ZigBeeEndPoint m_testedEndPoint = null;

        public void Test()
        {
            m_zigBeeAdapter = new Adapter();
            m_zigBeeAdapter.Initialize();

            if(SetBitronHomeDevice())
            {
                TestBitronHome();
            }
            if(SetDresdenElektronikDevice())
            {
                TestDresdenElektronik();
            }
        }

        private bool SetDresdenElektronikDevice()
        {
            m_testedDevice = null;
            m_testedEndPoint = null;
            if (m_zigBeeAdapter.DeviceList.TryGetValue(DRESDEN_ELEKTRONIK_BALLAST_MAC_ADDRESS, out m_testedDevice) &&
                m_testedDevice.EndPointList.TryGetValue(DRESDEN_ELEKTRONIK_BALLAST_END_POINT, out m_testedEndPoint))
            {
                Debug.WriteLine("End point: {0} - {1} - Mac address: 0x{2:X}, Id: 0x{3:X2}, ZigBee profile 0x{4:X4}",
                    m_testedEndPoint.Vendor, m_testedEndPoint.Model, m_testedDevice.MacAddress, m_testedEndPoint.Id, m_testedEndPoint.CommandProfileId);
                return true;
            }
            else
            {
                return false;
            }
        }

        private bool SetBitronHomeDevice()
        {
            m_testedDevice = null;
            m_testedEndPoint = null;
            int nb_trial = 100;
            while(nb_trial > 0)
            {
                if (m_zigBeeAdapter.DeviceList.TryGetValue(BITRON_HOME_DOOR_SENSOR_MAC_ADDRESS, out m_testedDevice) &&
                    m_testedDevice.EndPointList.TryGetValue(BITRON_HOME_DOOR_SENSOR_END_POINT, out m_testedEndPoint))
                {
                    Debug.WriteLine("End point: {0} - {1} - Mac address: 0x{2:X}, Id: 0x{3:X2}, ZigBee profile 0x{4:X4}",
                        m_testedEndPoint.Vendor, m_testedEndPoint.Model, m_testedDevice.MacAddress, m_testedEndPoint.Id, m_testedEndPoint.CommandProfileId);
                    break;
                }
                else
                {
                    Debug.WriteLine("Wait for BitronDevice - {0}", nb_trial);
                    System.Threading.Tasks.Task.Delay(1000).Wait();
                    nb_trial--;
                }
            }

            if (m_testedDevice != null &&
                m_testedEndPoint != null)
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        private void TestBitronHome()
        {
            ZclAttribute attribute = null;
            System.Object value;

            var IasZoneCluster = m_testedEndPoint.GetCluster(IASZoneCluster.CLUSTER_ID);
            if(IasZoneCluster == null)
            {
                Debug.WriteLine("ERROR - CAN'T GET BitronHome IAS Zone cluster !!!!");
                return;
            }

            IasZoneCluster.InternalAttributeList.TryGetValue(IASZoneCluster.ATTRIBUTE_ZONESTATUS, out attribute);
            if (attribute != null &&
                attribute.Read(out value) &&
                value is UInt16)
            {
                Debug.WriteLine("IAS Zone status = {0}", value);
            }
            else
            {
                Debug.WriteLine("ERROR - IAS Zone status !!!!");
            }

            IasZoneCluster.InternalAttributeList.TryGetValue(IASZoneCluster.ATTRIBUTE_ZONETYPE, out attribute);
            if (attribute != null &&
                attribute.Read(out value) &&
                value is UInt16)
            {
                Debug.WriteLine("IAS Zone type = {0}", value);
            }
            else
            {
                Debug.WriteLine("ERROR - IAS Zone type !!!!");
            }

            IasZoneCluster.InternalAttributeList.TryGetValue(IASZoneCluster.ATTRIBUTE_ZONESTATE, out attribute);
            if (attribute != null &&
                attribute.Read(out value) &&
                value is byte)
            {
                Debug.WriteLine("IAS Zone state = {0}", value);
            }
            else
            {
                Debug.WriteLine("ERROR - IAS Zone state !!!!");
            }

            IasZoneCluster.InternalAttributeList.TryGetValue(IASZoneCluster.ATTRIBUTE_IASCIEADDRESS, out attribute);
            if (attribute != null &&
                attribute.Read(out value) &&
                value is UInt64)
            {
                UInt64 macAddr = (UInt64)value;
                Debug.WriteLine("IAS Zone Cie address = 0x{0:X}", macAddr);
                if(macAddr == 0)
                {
                    attribute.Write(m_zigBeeAdapter.MacAddress);
                }
            }
            else
            {
                Debug.WriteLine("ERROR - IAS Zone Cie Address !!!!");
            }
        }
        private void DisplayOnOffStatus()
        {
            ZclAttribute attributeOnOff = null;
            System.Object value;

            var onOffcluster = m_testedEndPoint.GetCluster(OnOffCluster.CLUSTER_ID);
            if (onOffcluster != null)
            {
                onOffcluster.InternalAttributeList.TryGetValue(OnOffCluster.ATTRIBUTE_ONOFF, out attributeOnOff);
            }

            if (attributeOnOff.Read(out value))
            {
                if (value is bool)
                {
                    var isOn = (bool)value;
                    if (isOn)
                    {
                        Debug.WriteLine("   -> Light is on");
                    }
                    else
                    {
                        Debug.WriteLine("   -> Light is off");
                    }
                }
            }
        }

        private void SendOnOffCommand(ZclCommand command)
        {
            Debug.WriteLine("Command {0}", command.Name);
            command.Send();
            DisplayOnOffStatus();

            // wait some time after command has been sent so man can see something happening on LED band
            System.Threading.Tasks.Task.Delay(1000).Wait();
        }
        public void TestDresdenElektronik()
        {
            ZclCommand commandToggle = null;
            ZclCommand commandOn = null;

            // look for OnOff cluster in the In cluster list of this end point
            var onOffcluster = m_testedEndPoint.GetCluster(OnOffCluster.CLUSTER_ID);
            if (onOffcluster != null)
            {
                onOffcluster.CommandList.TryGetValue(OnOffCluster.COMMAND_ON, out commandOn);
                onOffcluster.CommandList.TryGetValue(OnOffCluster.COMMAND_TOGGLE, out commandToggle);
            }

            // set light on
            SendOnOffCommand(commandOn);
            for (int index = 0; index < 9; index++)
            {
                // toggle light
                SendOnOffCommand(commandToggle);
            }
            // set light on
            SendOnOffCommand(commandOn);
        }
    }
}
