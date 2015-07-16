/**
   Copyright(c) Microsoft Open Technologies, Inc.All rights reserved.
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
**/

using Windows.Foundation;

namespace AirHockeyApp
{
    public class MotorHelper
    {
        // Coordinates when X and Y are at offset 0
        private static Point zeroPoint = new Point(1430, 78);

        // Coordinates when X and Y are at their max offsets
        private static Point maxPoint = new Point(1000, 689);

        // Used to get current mallet and target positions
        public static Point GetCoordinatesFromOffset(Point offset)
        {
            double x = GetCoordinateXFromOffsetY((long)offset.Y);
            double y = GetCoordinateYFromOffsetX((long)offset.X);

            return new Point(x, y);
        }

        public static double GetCoordinateYFromOffsetX(long offset)
        {
            double pointYLength = maxPoint.Y - zeroPoint.Y;
            double ratioY = (double)offset / Config.MAX_MALLET_OFFSET_X;
            double y = ratioY * pointYLength + zeroPoint.Y;

            return y;
        }

        public static double GetCoordinateXFromOffsetY(long offset)
        {
            double pointXLength = zeroPoint.X - maxPoint.X;
            double ratioX = (double)(Config.MAX_MALLET_OFFSET_Y - offset) / Config.MAX_MALLET_OFFSET_Y;
            double x = ratioX * pointXLength + maxPoint.X;

            return x;
        }

        public static long GetOffsetXFromCoordinateY(double y)
        {
            double pointYLength = maxPoint.Y - zeroPoint.Y;
            double malletRatioX = (y - zeroPoint.Y) / pointYLength;
            double x = malletRatioX * Config.MAX_MALLET_OFFSET_X;
            x = Helper.Constrain(x, 0, Config.MAX_MALLET_OFFSET_X);

            return (long)x;
        }

        public static long GetOffsetYFromCoordinateX(double x)
        {
            double pointXLength = zeroPoint.X - maxPoint.X;
            double malletRatioY = (zeroPoint.X - x) / pointXLength;
            double y = malletRatioY * Config.MAX_MALLET_OFFSET_Y;
            y = Helper.Constrain(y, 0, Config.MAX_MALLET_OFFSET_Y);

            return (long)y;
        }

        public static Point GetOffsetFromCoordinates(Point point)
        {
            return new Point(GetOffsetXFromCoordinateY(point.Y), GetOffsetYFromCoordinateX(point.X));
        }
    }
}
