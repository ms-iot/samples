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
using Windows.UI;

namespace MyLivingRoom.Extensions
{
    public static class ColorExtensions
    {
        public static void ToHSV(this Color color, out double hue, out double saturation, out double value)
        {
            int max = Math.Max(color.R, Math.Max(color.G, color.B));
            int min = Math.Min(color.R, Math.Min(color.G, color.B));
            int chroma = max - min;

            if (chroma != 0)
            {
                double r = color.R;
                double g = color.G;
                double b = color.B;

                double alpha = 0.5 * (2.0 * r - g - b);
                double beta = (Math.Sqrt(3.0) / 2.0) * (g - b);
                double newHue = Math.Atan2(beta, alpha);
                if (newHue < 0)
                {
                    newHue += 2 * Math.PI;
                }
                hue = (newHue * ushort.MaxValue) / (2 * Math.PI);
            }
            else
            {
                hue = double.MaxValue;
            }

            saturation = (max == 0) ? 0 : ((double)chroma / (double)max);
            saturation = saturation * ushort.MaxValue;

            value = (color.R + color.G + color.B) / (3.0 * 255.0);
            value = value * ushort.MaxValue;
        }
    }
}
