// Copyright (c) Microsoft. All rights reserved.

using System;
using Windows.Foundation;

namespace AirHockeyApp
{
    public struct Vector
    {
        public double X, Y, H;
    }

    public class Helper
    {
        public static Vector GetDistance(Point p1, Point p2)
        {
            Vector v = new Vector();
            v.X = p2.X - p1.X;
            v.Y = p2.Y - p1.Y;
            v.H = Math.Sqrt(v.X * v.X + v.Y * v.Y);

            return v;
        }

        public static double[] ComputeCoefficents(double[,] X, double[] Y)
        {
            // Used for calibration
            int I, J, K, K1, N;
            N = Y.Length;
            for (K = 0; K < N; K++)
            {
                K1 = K + 1;
                for (I = K; I < N; I++)
                {
                    if (X[I, K] != 0)
                    {
                        for (J = K1; J < N; J++)
                        {
                            X[I, J] /= X[I, K];
                        }
                        Y[I] /= X[I, K];
                    }
                }
                for (I = K1; I < N; I++)
                {
                    if (X[I, K] != 0)
                    {
                        for (J = K1; J < N; J++)
                        {
                            X[I, J] -= X[K, J];
                        }
                        Y[I] -= Y[K];
                    }
                }
            }
            for (I = N - 2; I >= 0; I--)
            {
                for (J = N - 1; J >= I + 1; J--)
                {
                    Y[I] -= X[I, J] * Y[J];
                }
            }

            return Y;
        }

        public static double Constrain(double x, double a, double b)
        {
            if (x <= a)
            {
                return a;
            }
            else if (x >= b)
            {
                return b;
            }

            return x;
        }
    }
}

