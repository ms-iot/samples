using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GpioOneWire
{
    public class DhtSensorReading
    {
        public BitArray Bits { get; } = new BitArray(40, false);

        public bool IsValid()
        {
            ulong value = GetBitsValue();

            ulong checksum =
                ((value >> 32) & 0xff) +
                ((value >> 24) & 0xff) +
                ((value >> 16) & 0xff) +
                ((value >> 8) & 0xff);

            return (checksum & 0xff) == (value & 0xff);
        }

        public double Humidity()
        {
            ulong value = GetBitsValue();

            return ((value >> 24) & 0xffff) * 0.1;
        }

        public double Temperature()
        {
            ulong value = GetBitsValue();

            double temp = ((value >> 8) & 0x7FFF) * 0.1;

            if (((value >> 8) & 0x8000) == 0x8000)
            {
                temp = -temp;
            }

            return temp;
        }

        private ulong GetBitsValue()
        {
            ulong value = 0;
            ulong mask = 1;

            for (int i = 0; i < Bits.Length; i++)
            {
                if (Bits[i])
                {
                    value ^= mask;
                }

                mask <<= 1;
            }

            return value;
        }
    };
}
