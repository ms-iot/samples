using System.Collections.Generic;
using Windows.Devices.Adc.Provider;

namespace Microsoft.IoT.AdcMcp3008
{
    class AdcMcp3008Provider : IAdcProvider
    {
        static IAdcProvider providerSingleton = null;

        static public IAdcProvider GetAdcProvider()
        {
            if (providerSingleton == null)
            {
                providerSingleton = new AdcMcp3008Provider();
            }
            return providerSingleton;

        }

        public IReadOnlyList<IAdcControllerProvider> GetControllers()
        {
            AdcMcp3008ControllerProvider provider = new AdcMcp3008ControllerProvider(AdcMcp3008ControllerProvider.DefaultChipSelectLine);

            List<IAdcControllerProvider> list = new List<IAdcControllerProvider>();
            list.Add(provider);

            return list;
        }
    }
}
