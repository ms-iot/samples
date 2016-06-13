using Microsoft.IoT.AdcMcp3008;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Devices.Adc;

namespace IoTBlocklyHelper
{
    public sealed class Adc
    {
        // notice how we keep the "state" static so that it can be reused when we stop/restart/modify the js script
        static private AdcController controller;
        static private Dictionary<int, AdcChannel> channels = new Dictionary<int, AdcChannel>();
        private const AdcChannelMode mode = AdcChannelMode.SingleEnded;
        private static Task _initializingTask;

        public Adc()
        {
            if (_initializingTask == null)
            {
                _initializingTask = Init();
            }
        }

        private async Task Init()
        {
            controller = (await AdcController.GetControllersAsync(AdcMcp3008Provider.GetAdcProvider()))[0];
            controller.ChannelMode = mode;
        }

        public static bool Valid()
        {
            if (controller == null)
            {
                _initializingTask.Wait();
            }
            return (controller != null);
        }

        private static AdcChannel GetChannel(int channelNumber)
        {
            if (!Valid()) { return null; }
            if (!channels.ContainsKey(channelNumber))
            {
                try
                {
                    var channel = controller.OpenChannel(channelNumber);
                    channels[channelNumber] = channel;
                }
                catch
                {
                    channels[channelNumber] = null;
                }
            }
            return channels[channelNumber];
        }


        public int GetValueFromChannel(int channelNumber)
        {
            var channel = GetChannel(channelNumber);
            return channel.ReadValue();
        }

    }
}
