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

namespace AirHockeyApp
{
    public struct Config
    {
        public static float
            MOTOR_X_MAX_SPEED = 100000,
            MOTOR_Y_MAX_SPEED = 70000,
            MOTOR_X_ACCELERATION = 1000000,
            MOTOR_Y_ACCELERATION = 600000;

        // Default values, not const because Calibration can adjust values
        public static int MAX_MALLET_OFFSET_X = 2681;
        public static int MAX_MALLET_OFFSET_Y = 2462;

        public const long TABLE_HEIGHT = 766;
        public const long TABLE_MID_X_COORDINATE = 950;
        public const long TABLE_GOAL_Y_TOP = TABLE_HEIGHT / 2 - 150;
        public const long TABLE_GOAL_Y_BOTTOM = TABLE_HEIGHT / 2 + 150;
    }
}
