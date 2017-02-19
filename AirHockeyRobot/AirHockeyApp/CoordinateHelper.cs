// Copyright (c) Microsoft. All rights reserved.

using System;
using Windows.Foundation;

namespace AirHockeyApp
{
    public class CoordinateHelper
    {
        public static Point INVALID_POINT = new Point(-1, -1);

        // Points used for calibrating
        private static Point bottomLeft, bottomRight, bottomCenter, topLeft, topRight, topCenter, center, centerLeft, centerRight;

        // Coefficients for reducing lens distortion
        private static double[] topQuadraticCoeff, bottomQuadraticCoeff, centerHorizontalQuadraticCoeff,
            leftQuadraticCoeff, rightQuadraticCoeff, centerVerticalQuadraticCoeff;

        public static double VirtualHeight, VirtualWidth;

        public static void Initialize(double virtualWidth, double virtualHeight)
        {
            VirtualWidth = virtualWidth;
            VirtualHeight = virtualHeight;

            setDefaultCalibration();
        }

        /// <summary>
        /// Translate raw camera coordinates into points in the 1366 X 766 canvas
        /// </summary>
        /// <param name="x">Raw X-coordinate returned by Pixy camera</param>
        /// <param name="y">Raw Y-coordinate returned by Pixy camera</param>
        /// <returns>Point on the screen</returns>
        public static Point TranslatePoint(double x, double y)
        {
            if (topQuadraticCoeff.Length < 1 || bottomQuadraticCoeff.Length < 1)
            {
                return new Point(-1, -1);
            }

            Point translatedPoint = new Point();

            // Calculate the horizontal and vertical lines of the table
            double centerY = centerHorizontalQuadraticCoeff[0] * Math.Pow(x, 2) + centerHorizontalQuadraticCoeff[1] * x + centerHorizontalQuadraticCoeff[2];
            double centerX = centerVerticalQuadraticCoeff[0] * Math.Pow(y, 2) + centerVerticalQuadraticCoeff[1] * y + centerVerticalQuadraticCoeff[2];

            if (y < centerY)
            {
                double topY = topQuadraticCoeff[0] * Math.Pow(x, 2) + topQuadraticCoeff[1] * x + topQuadraticCoeff[2];
                double ratioY = (y - topY) / (centerY - topY);
                translatedPoint.Y = VirtualHeight / 2 * ratioY;
            }
            else
            {
                double bottomY = bottomQuadraticCoeff[0] * Math.Pow(x, 2) + bottomQuadraticCoeff[1] * x + bottomQuadraticCoeff[2];
                double ratioY = (bottomY - y) / (bottomY - centerY);
                translatedPoint.Y = VirtualHeight - (VirtualHeight / 2 * ratioY);
            }

            if (x < centerX)
            {
                double leftX = leftQuadraticCoeff[0] * Math.Pow(y, 2) + leftQuadraticCoeff[1] * y + leftQuadraticCoeff[2];
                double ratioX = (x - leftX) / (centerX - leftX);
                translatedPoint.X = VirtualWidth / 2 * ratioX;
            }
            else
            {
                double rightX = rightQuadraticCoeff[0] * Math.Pow(y, 2) + rightQuadraticCoeff[1] * y + rightQuadraticCoeff[2];
                double ratioX = (rightX - x) / (rightX - centerX);
                translatedPoint.X = VirtualWidth - (VirtualWidth / 2 * ratioX);
            }

            return translatedPoint;
        }

        /// <summary>
        /// Calculate the trajectory given 2 points
        /// </summary>
        /// <param name="p1">Starting point of trajectory</param>
        /// <param name="p2">Point between start and final point</param>
        /// <param name="x">Final X coordinate</param>
        /// <returns>Trajectory point bound by the x parameter and the bounds of the air hockey table</returns>
        public static Point CalculateTrajectoryPoint(Point p1, Point p2, double x)
        {
            var coeff = Helper.ComputeCoefficents(
                new double[,] {
                    { p1.X, 1 },
                    { p2.X, 1 }
                },
                new double[] { p1.Y, p2.Y }
            );

            double y = coeff[0] * x + coeff[1];

            if (y >= 0 && y <= VirtualHeight)
            {
                return new Point(x, y);
            }
            else
            {
                y = Helper.Constrain(y, 0, VirtualHeight);
                x = (y - coeff[1]) / coeff[0];
                return new Point(x, y);
            }
        }

        /// <summary>
        /// Calculates the point where the puck will bounce off of the table
        /// </summary>
        /// <param name="p1">Starting point of trajectory</param>
        /// <param name="p2">Point where the puck will bounce</param>
        /// <param name="x">Final X coordinate</param>
        /// <param name="angleAdjust">Value used to adjust the angle of the bounce to account for lost energy</param>
        /// <returns>Final point after the bounce</returns>
        public static Point CalculateBouncePoint(Point p1, Point p2, double x, double angleAdjust)
        {
            var coeff = Helper.ComputeCoefficents(
               new double[,] {
                    { p1.X, 1 },
                    { p2.X, 1 }
               },
               new double[] { p1.Y, p2.Y }
            );

            // Find reflected b
            double m = -coeff[0] * angleAdjust;
            double b = p2.Y - (-coeff[0] * p2.X);

            double y = m * x + b;

            if (y >= 0 && y <= VirtualHeight)
            {
                return new Point(x, y);
            }
            else
            {
                y = Helper.Constrain(y, 0, VirtualHeight);
                x = (y - b) / m;

                return new Point(x, y);
            }
        }

        /// <summary>
        /// Sets pre-calibrated points for our specific setup
        /// </summary>
        private static void setDefaultCalibration()
        {
            // Set pre-calibrated points
            topLeft = new Point(17, 20);
            topRight = new Point(299, 30);
            topCenter = new Point(160, 12);
            bottomLeft = new Point(12, 170);
            bottomRight = new Point(293, 182);
            bottomCenter = new Point(155, 190);
            center = new Point(156, 100);
            centerLeft = new Point(9, 96);
            centerRight = new Point(303, 107);

            // Calculate coefficients for the quadratic equations to help account for curvature distortion in lens
            topQuadraticCoeff = Helper.ComputeCoefficents(
                new double[,] {
                    { Math.Pow(topLeft.X, 2), topLeft.X, 1 },
                    { Math.Pow(topCenter.X, 2), topCenter.X, 1 },
                    { Math.Pow(topRight.X, 2), topRight.X, 1 }
                },
                new double[] { topLeft.Y, topCenter.Y, topRight.Y }
            );

            bottomQuadraticCoeff = Helper.ComputeCoefficents(
                new double[,] {
                    { Math.Pow(bottomLeft.X, 2), bottomLeft.X, 1 },
                    { Math.Pow(bottomCenter.X, 2), bottomCenter.X, 1 },
                    { Math.Pow(bottomRight.X, 2), bottomRight.X, 1 }
                },
                new double[] { bottomLeft.Y, bottomCenter.Y, bottomRight.Y }
            );

            centerHorizontalQuadraticCoeff = Helper.ComputeCoefficents(
                new double[,] {
                    { Math.Pow(centerLeft.X, 2), centerLeft.X, 1 },
                    { Math.Pow(center.X, 2), center.X, 1 },
                    { Math.Pow(centerRight.X, 2), centerRight.X, 1 }
                },
                new double[] { centerLeft.Y, center.Y, centerRight.Y }
            );

            leftQuadraticCoeff = Helper.ComputeCoefficents(
                new double[,] {
                    { Math.Pow(topLeft.Y, 2), topLeft.Y, 1 },
                    { Math.Pow(centerLeft.Y, 2), centerLeft.Y, 1 },
                    { Math.Pow(bottomLeft.Y, 2), bottomLeft.Y, 1 }
                },
                new double[] { topLeft.X, centerLeft.X, bottomLeft.X }
            );

            rightQuadraticCoeff = Helper.ComputeCoefficents(
                new double[,] {
                    { Math.Pow(topRight.Y, 2), topRight.Y, 1 },
                    { Math.Pow(centerRight.Y, 2), centerRight.Y, 1 },
                    { Math.Pow(bottomRight.Y, 2), bottomRight.Y, 1 }
                },
                new double[] { topRight.X, centerRight.X, bottomRight.X }
            );

            centerVerticalQuadraticCoeff = Helper.ComputeCoefficents(
                new double[,] {
                    { Math.Pow(topCenter.Y, 2), topCenter.Y, 1 },
                    { Math.Pow(center.Y, 2), center.Y, 1 },
                    { Math.Pow(bottomCenter.Y, 2), bottomCenter.Y, 1 }
                },
                new double[] { topCenter.X, center.X, bottomCenter.X }
            );
        }

    }
}
