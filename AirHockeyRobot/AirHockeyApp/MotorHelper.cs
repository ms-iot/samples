// Copyright (c) Microsoft. All rights reserved.

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
