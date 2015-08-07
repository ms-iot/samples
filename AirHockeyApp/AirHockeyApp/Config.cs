// Copyright (c) Microsoft. All rights reserved.

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
