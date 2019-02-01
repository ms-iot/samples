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
            //Humidity Bytes are bits 1 & 2 out of 5
            //Byte 1 is integer Humidity - bit 2 is decimal humidity  
            //Byte 2 is always 00000000 for a DHT 11 - the DHT22 does decimal points
            ulong value = GetBitsValue();
            double humidity = ((value >> 24) & 0xFF) * 0.1; //Decode the Decimal into Humidity
            humidity = humidity + ((value >> 32) & 0xFF);// And add on the integer part of humidity
            return humidity;
        }

        public double Temperature()
        {
            //Temp Bytes are bits 3 & 4 out of 5
            //Byte 3 is integer temp - bit 4 is decimal temp  
            //Byte 4 is always 00000000 for a DHT 11 - the DHT22 does decimal points
            ulong value = GetBitsValue();
            double temp = ((value >> 8)  & 0xFF) * 0.1; //Decode the Decimal into Temp
            temp = temp + ((value >> 16) & 0x7F);// And add on the integer part of temp
            if ((value >> 8) & 0x8000)  // if the MSB of temp is 1 then its negative
                temp = -temp;
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
