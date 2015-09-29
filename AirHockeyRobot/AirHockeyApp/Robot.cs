// Copyright (c) Microsoft. All rights reserved.

using System;
using Windows.Devices.Gpio;
using Windows.Foundation;
using AirHockeyHelper;

namespace AirHockeyApp
{
    public class Robot
    {
        GpioController controller;

        /* Each stepper motor is controller with 2 pins. 1 to control direction, and 1 to step the motor */
        // MBM pins
        private const int MOTOR_DIR_PIN_X = 6;
        private const int MOTOR_STEP_PIN_X = 7;
        private const int MOTOR_STEP_PIN_Y = 8;
        private const int MOTOR_DIR_PIN_Y = 9;

        // When MotorDirPin_Y is low, mallet moves towards you
        // When MotorDirPin_X is low, mallet moves left
        private GpioPin MotorStepPin_X;
        private GpioPin MotorDirPin_X;
        private GpioPin MotorStepPin_Y;
        private GpioPin MotorDirPin_Y;

        /* Limit switches are placed at the ends of the guide rails 
         * That way we know when the carraige is near the ends
         */
        // MBM pins
        private const int LIMIT_SWITCH_PIN_X1 = 0;
        private const int LIMIT_SWITCH_PIN_X2 = 1;
        private const int LIMIT_SWITCH_PIN_Y1 = 2;
        private const int LIMIT_SWITCH_PIN_Y2 = 3;

        // Y sensors go low when triggered, X sensors go high when triggered
        private GpioPin LimitSwitchPin_X1;  // Right 
        private GpioPin LimitSwitchPin_X2;  // Left
        private GpioPin LimitSwitchPin_Y1;  // Bottom
        private GpioPin LimitSwitchPin_Y2;  // Top

        /* Goal sensors go high when the puck blocks the IR beam */
        // MBM pins
        private const int GOAL_SENSOR_PIN_ROBOT = 4;
        private const int GOAL_SENSOR_PIN_HUMAN = 5;
        
        private GpioPin GoalSensorPin_Robot;
        private GpioPin GoalSensorPin_Human;

        private bool stop = false;
        private object stepperLock;

        public event EventHandler<EventArgs> RobotGoalSensorTriggered;
        public event EventHandler<EventArgs> HumanGoalSensorTriggered;

        public StepperLib StepperX, StepperY;

        public AIHelper AI;

        public Robot()
        {
            initialize();
            AI = new AIHelper(this);
            stepperLock = new object();
        }

        private void initialize()
        {
            controller = GpioController.GetDefault();

            initializeLimitSwitches();
            initializeMotors();
            initializeGoalSensors();
        }

        public void MoveMotorsToZero()
        {
            stop = false;

            long moveSpeed = (long)Config.DEFAULT_MOVE_SPEED;
            StepperX.Speed = moveSpeed;
            StepperY.Speed = moveSpeed;

            while (LimitSwitchPin_X1.Read() == GpioPinValue.High || LimitSwitchPin_Y1.Read() == GpioPinValue.High)
            {
                if (LimitSwitchPin_X1.Read() == GpioPinValue.High)
                {
                    StepperX.Move(-10);
                    StepperX.RunSpeed();
                }

                if (LimitSwitchPin_Y1.Read() == GpioPinValue.High)
                {
                    StepperY.Move(-10);
                    StepperY.RunSpeed();
                }

                if (stop) return;
            }

            StepperX.SetCurrentPosition(0);
            StepperY.SetCurrentPosition(0);

            LimitSwitchPin_X1.ValueChanged += LimitSwitchPin_X1_ValueChanged;
            LimitSwitchPin_X2.ValueChanged += LimitSwitchPin_X2_ValueChanged;
            LimitSwitchPin_Y1.ValueChanged += LimitSwitchPin_Y1_ValueChanged;
            LimitSwitchPin_Y2.ValueChanged += LimitSwitchPin_Y2_ValueChanged;
        }

        public void Stop()
        {
            stop = true;
        }

        public void Close()
        {
            stop = true;

            // Dispose limit switches
            LimitSwitchPin_X1.Dispose();
            LimitSwitchPin_X2.Dispose();
            LimitSwitchPin_Y1.Dispose();
            LimitSwitchPin_Y2.Dispose();

            LimitSwitchPin_X1 = null;
            LimitSwitchPin_X2 = null;
            LimitSwitchPin_Y1 = null;
            LimitSwitchPin_Y2 = null;

            // Dispose motors
            MotorDirPin_X.Dispose();
            MotorDirPin_Y.Dispose();
            MotorStepPin_X.Dispose();
            MotorStepPin_Y.Dispose();

            // Dispose goal sensors
            GoalSensorPin_Human.Dispose();
            GoalSensorPin_Robot.Dispose();

            controller = null;
        }

        public void RunToPosition()
        {
            while (StepperX.Run() || StepperY.Run()) ;
        }

        public Point GetOffsets()
        {
            return new Point(StepperX.CurrentPosition, StepperY.CurrentPosition);
        }

        public void ResetMotorParameters()
        {
            StepperX.MaxSpeed = Config.MOTOR_X_MAX_SPEED;
            StepperY.MaxSpeed = Config.MOTOR_Y_MAX_SPEED;
            StepperX.Acceleration = Config.MOTOR_X_ACCELERATION;
            StepperY.Acceleration = Config.MOTOR_Y_ACCELERATION;
        }

        public void MoveToOffset(Point offset)
        {
            StepperX.MoveTo((long)offset.X);
            StepperY.MoveTo((long)offset.Y);
        }

        public void MoveFastToOffset(Point offset)
        {
            ResetMotorParameters();
            MoveToOffset(offset);
        }

        public void MoveStraightToOffset(Point offset)
        {
            ResetMotorParameters();

            float diffX = (float)Math.Abs(StepperX.CurrentPosition - offset.X);
            float diffY = (float)Math.Abs(StepperY.CurrentPosition - offset.Y);

            if (diffX > 0 && diffY > 0)
            {
                float newAccelY = Config.MOTOR_Y_ACCELERATION;
                float newAccelX = Config.MOTOR_X_ACCELERATION;

                long minMaxSpeed = (long)Math.Min(Config.MOTOR_X_MAX_SPEED, Config.MOTOR_Y_MAX_SPEED);
                StepperX.MaxSpeed = minMaxSpeed;
                StepperY.MaxSpeed = minMaxSpeed;

                // We need to move more in the X direction than Y, so Y accel will be slower than X accel
                if (diffX > diffY)
                {
                    if (Config.MOTOR_Y_ACCELERATION < Config.MOTOR_X_ACCELERATION)
                    {
                        newAccelY = diffY / diffX * Config.MOTOR_X_ACCELERATION;
                        if (newAccelY > Config.MOTOR_Y_ACCELERATION)
                        {
                            newAccelX = Config.MOTOR_X_ACCELERATION * (Config.MOTOR_Y_ACCELERATION / newAccelY);
                            newAccelY = Config.MOTOR_Y_ACCELERATION;
                        }
                    }
                    else
                    {
                        newAccelY = diffY / diffX * Config.MOTOR_X_ACCELERATION;
                        newAccelX = Config.MOTOR_X_ACCELERATION;
                    }
                }
                // We need to move more in the Y direction, so X accel will be slower than Y accel
                else
                {
                    if (Config.MOTOR_Y_ACCELERATION < Config.MOTOR_X_ACCELERATION)
                    {
                        newAccelX = diffX / diffY * Config.MOTOR_Y_ACCELERATION;
                        newAccelY = Config.MOTOR_Y_ACCELERATION;
                    }
                    else
                    {
                        newAccelX = diffX / diffY * Config.MOTOR_Y_ACCELERATION;
                        if (newAccelX > Config.MOTOR_X_ACCELERATION)
                        {
                            newAccelY = Config.MOTOR_Y_ACCELERATION * (Config.MOTOR_X_ACCELERATION / newAccelX);
                            newAccelX = Config.MOTOR_X_ACCELERATION;
                        }
                    }
                }

                StepperX.Acceleration = newAccelX;
                StepperY.Acceleration = newAccelY;
            }

            MoveToOffset(offset);
        }

        private void initializeMotors()
        {
            /* Initialize motor pins */

            MotorStepPin_X = controller.OpenPin(MOTOR_STEP_PIN_X);
            MotorDirPin_X = controller.OpenPin(MOTOR_DIR_PIN_X);
            MotorStepPin_Y = controller.OpenPin(MOTOR_STEP_PIN_Y);
            MotorDirPin_Y = controller.OpenPin(MOTOR_DIR_PIN_Y);

            MotorStepPin_X.Write(GpioPinValue.High);
            MotorDirPin_X.Write(GpioPinValue.High);
            MotorStepPin_Y.Write(GpioPinValue.High);
            MotorDirPin_Y.Write(GpioPinValue.High);

            MotorStepPin_X.SetDriveMode(GpioPinDriveMode.Output);
            MotorDirPin_X.SetDriveMode(GpioPinDriveMode.Output);
            MotorStepPin_Y.SetDriveMode(GpioPinDriveMode.Output);
            MotorDirPin_Y.SetDriveMode(GpioPinDriveMode.Output);

            StepperX = new StepperLib(MotorStepPin_X, MotorDirPin_X, GpioPinValue.Low);
            StepperY = new StepperLib(MotorStepPin_Y, MotorDirPin_Y, GpioPinValue.High);
        }

        private void initializeLimitSwitches()
        {
            /* Initialize limit switches */
            LimitSwitchPin_X1 = controller.OpenPin(LIMIT_SWITCH_PIN_X1);
            LimitSwitchPin_X2 = controller.OpenPin(LIMIT_SWITCH_PIN_X2);
            LimitSwitchPin_Y1 = controller.OpenPin(LIMIT_SWITCH_PIN_Y1);
            LimitSwitchPin_Y2 = controller.OpenPin(LIMIT_SWITCH_PIN_Y2);

            // MBM drive modes
            LimitSwitchPin_X1.SetDriveMode(GpioPinDriveMode.Input);
            LimitSwitchPin_X2.SetDriveMode(GpioPinDriveMode.Input);
            LimitSwitchPin_Y1.SetDriveMode(GpioPinDriveMode.Input);
            LimitSwitchPin_Y2.SetDriveMode(GpioPinDriveMode.Input);
        }

        private void initializeGoalSensors()
        {
            GoalSensorPin_Human = controller.OpenPin(GOAL_SENSOR_PIN_HUMAN);
            GoalSensorPin_Robot = controller.OpenPin(GOAL_SENSOR_PIN_ROBOT);

            GoalSensorPin_Human.SetDriveMode(GpioPinDriveMode.Input);
            GoalSensorPin_Robot.SetDriveMode(GpioPinDriveMode.Input);

            GoalSensorPin_Human.ValueChanged += GoalSensorPin_Player_ValueChanged;
            GoalSensorPin_Robot.ValueChanged += GoalSensorPin_Robot_ValueChanged;
        }

        private void LimitSwitchPin_Y2_ValueChanged(GpioPin sender, GpioPinValueChangedEventArgs args)
        {
            // At YMax
            if (args.Edge == GpioPinEdge.FallingEdge)
            {
                StepperY.SetCurrentPosition(Config.MAX_MALLET_OFFSET_Y);
            }
        }

        private void LimitSwitchPin_Y1_ValueChanged(GpioPin sender, GpioPinValueChangedEventArgs args)
        {
            // At Y0
            if (args.Edge == GpioPinEdge.FallingEdge)
            {
                StepperY.SetCurrentPosition(0);
            }
        }

        private void LimitSwitchPin_X2_ValueChanged(GpioPin sender, GpioPinValueChangedEventArgs args)
        {
            // At XMax
            if (args.Edge == GpioPinEdge.FallingEdge)
            {
                StepperX.SetCurrentPosition(Config.MAX_MALLET_OFFSET_X);
            }
        }

        private void LimitSwitchPin_X1_ValueChanged(GpioPin sender, GpioPinValueChangedEventArgs args)
        {
            // At X0
            if (args.Edge == GpioPinEdge.FallingEdge)
            {
                StepperX.SetCurrentPosition(0);
            }
        }

        private void GoalSensorPin_Robot_ValueChanged(GpioPin sender, GpioPinValueChangedEventArgs args)
        {
            if (args.Edge == GpioPinEdge.FallingEdge)
            {
                OnRobotGoalSensorTriggered(new EventArgs());
            }
        }

        private void GoalSensorPin_Player_ValueChanged(GpioPin sender, GpioPinValueChangedEventArgs args)
        {
            if (args.Edge == GpioPinEdge.FallingEdge)
            {
                OnHumanGoalSensorTriggered(new EventArgs());
            }
        }

        protected virtual void OnRobotGoalSensorTriggered(EventArgs e)
        {
            EventHandler<EventArgs> handler = RobotGoalSensorTriggered;
            if (handler != null)
            {
                handler(this, e);
            }
        }

        protected virtual void OnHumanGoalSensorTriggered(EventArgs e)
        {
            EventHandler<EventArgs> handler = HumanGoalSensorTriggered;
            if (handler != null)
            {
                handler(this, e);
            }
        }

    }
}
