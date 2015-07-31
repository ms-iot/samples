// Copyright (c) Microsoft. All rights reserved.

// This code has been adapted from AccelStepper.h taken from http://www.airspayce.com/mikem/arduino/AccelStepper/AccelStepper_8h_source.html
//Copyright (C) 2009-2013 Mike McCauley  200 // $Id: AccelStepper.h,v 1.21 2014/10/31 06:05:30 mikem Exp mikem $

using System;
using System.Diagnostics;
using Windows.Devices.Gpio;

namespace AirHockeyHelper2
{
    public class AccelStepper
    {
        Stopwatch stopwatch;
        Direction direction;

        public bool Debug = false;

        #region Private variables

        GpioPin motorPin;

        GpioPin directionPin;

        GpioPinValue clockwiseValue;

        /// The current absolution position in steps.
        long currentPos;    // Steps

        /// The target position in steps. The AccelStepper library will move the
        /// motor from the currentPos to the targetPos, taking into account the
        /// max speed, acceleration and deceleration
        long targetPos;     // Steps

        /// The current motos speed in steps per second
        /// Positive is clockwise
        float speed;         // Steps per second

        /// The maximum permitted speed in steps per second. Must be > 0.
        float maxSpeed;

        /// The acceleration to use to accelerate or decelerate the motor in steps
        /// per second per second. Must be > 0
        float acceleration;

        /// The current interval between steps in ticks.
        /// 0 means the motor is currently stopped with speed == 0
        long stepInterval;

        /// The last step time in ticks
        long lastStepTime;

        /// The step counter for speed calculations
        long n;

        /// Initial step size in ticks
        float c0;

        /// Last step size in ticks
        float cn;

        /// Min step size in ticks based on maxSpeed
        float cmin; // at max speed

        object runLock = new object();

        #endregion

        enum Direction
        {
            Clockwise,
            CounterClockwise
        }

        public AccelStepper(GpioPin motorPinEntity, GpioPin directionPinEntity, GpioPinValue clockwiseVal)
        {
            motorPin = motorPinEntity;
            directionPin = directionPinEntity;
            clockwiseValue = clockwiseVal;

            stopwatch = new Stopwatch();
            stopwatch.Start();
        }

        public void MoveTo(long absolute)
        {
            if (targetPos != absolute)
            {
                targetPos = absolute;
                computeNewSpeed();
            }
        }

        public void Move(long relative)
        {
            MoveTo(currentPos + relative);
        }

        public bool Run()
        {
            lock (runLock)
            {
                if (RunSpeed())
                {
                    computeNewSpeed();
                }

                return speed != 0 || DistanceToGo() != 0;
            }
        }

        public bool RunSpeed()
        {
            if (stepInterval == 0)
            {
                return false;
            }

            long time = stopwatch.ElapsedTicks;
            long nextStepTime = lastStepTime + stepInterval;

            if (((nextStepTime >= lastStepTime) && ((time >= nextStepTime) || (time < lastStepTime)))
                   || ((nextStepTime < lastStepTime) && ((time >= nextStepTime) && (time < lastStepTime))))
            {
                if (direction == Direction.Clockwise)
                {
                    // Clockwise
                    currentPos += 1;
                }
                else
                {
                    // Anticlockwise  
                    currentPos -= 1;
                }
                Step(currentPos);

                lastStepTime = time;
                return true;
            }
            else
            {
                return false;
            }
        }

        public void Step(long step)
        {
            if (!Debug)
            {
                if (speed > 0)
                {
                    directionPin.Write(clockwiseValue);
                }
                else
                {
                    directionPin.Write((clockwiseValue == GpioPinValue.High) ? GpioPinValue.Low : GpioPinValue.High);
                }

                motorPin.Write(GpioPinValue.Low);
                motorPin.Write(GpioPinValue.High);
            }
        }

        public void SetMaxSpeed(float speedValue)
        {
            if (maxSpeed != speedValue)
            {
                maxSpeed = speedValue;
                cmin = TimeSpan.TicksPerSecond / speedValue;
                // Recompute n from current speed and adjust speed if accelerating or cruising
                if (n > 0)
                {
                    n = (long)((speed * speed) / (2.0 * acceleration)); // Equation 16
                    computeNewSpeed();
                }
            }
        }

        public float Acceleration()
        {
            return acceleration;
        }

        public void SetAcceleration(float accelerationValue)
        {
            if (accelerationValue == 0.0)
                return;
            if (acceleration != accelerationValue)
            {
                // Recompute n per Equation 17
                n = (long)(n * (acceleration / accelerationValue));
                // New c0 per Equation 7, with correction per Equation 15
                c0 = (float)(0.676 * Math.Sqrt(2.0 / accelerationValue) * TimeSpan.TicksPerSecond); // Equation 15
                acceleration = accelerationValue;
                computeNewSpeed();
            }
        }

        public static float Constrain(float x, float a, float b)
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

        public void SetSpeed(float speedValue)
        {
            if (speed == speedValue)
                return;
            speedValue = Constrain(speedValue, -maxSpeed, maxSpeed);
            if (speedValue == 0.0)
                stepInterval = 0;
            else
            {
                stepInterval = (long)Math.Abs(TimeSpan.TicksPerSecond / speedValue);
                direction = (speedValue > 0.0) ? Direction.Clockwise : Direction.CounterClockwise;
            }
            speed = speedValue;
        }

        public float Speed()
        {
            return speed;
        }

        public long DistanceToGo()
        {
            return targetPos - currentPos;
        }

        public long TargetPosition()
        {
            return targetPos;
        }

        public long CurrentPosition()
        {
            return currentPos;
        }

        public float MaxSpeed()
        {
            return maxSpeed;
        }

        public void SetCurrentPosition(long position)
        {
            targetPos = currentPos = position;
            n = 0;
            stepInterval = 0;
        }

        public void RunToPosition()
        {
            while (Run()) ;
        }

        public void RunToNewPosition(long position)
        {
            MoveTo(position);
            RunToPosition();
        }

        public void RunSpeedToNewPosition(long position, float speed)
        {
            MoveTo(position);
            SetSpeed(speed);
            while (DistanceToGo() != 0)
            {
                RunSpeed();
            }
        }

        public bool RunSpeedToPosition()
        {
            if (targetPos == currentPos)
                return false;
            if (targetPos > currentPos)
                direction = Direction.Clockwise;
            else
                direction = Direction.CounterClockwise;
            return RunSpeed();
        }

        public void Stop()
        {
            if (speed != 0.0)
            {
                long stepsToStop = (long)((speed * speed) / (2.0 * acceleration)) + 1; // Equation 16 (+integer rounding)
                if (speed > 0)
                    Move(stepsToStop);
                else
                    Move(-stepsToStop);
            }
        }

        protected void computeNewSpeed()
        {
            long distanceTo = DistanceToGo(); // +ve is clockwise from curent location

            long stepsToStop = (long)((speed * speed) / (2.0 * acceleration)); // Equation 16

            if (distanceTo == 0 && stepsToStop <= 1)
            {
                // We are at the target and its time to stop
                stepInterval = 0;
                speed = 0;
                n = 0;
                return;
            }

            if (distanceTo > 0)
            {
                // We are anticlockwise from the target
                // Need to go clockwise from here, maybe decelerate now
                if (n > 0)
                {
                    // Currently accelerating, need to decel now? Or maybe going the wrong way?
                    if ((stepsToStop >= distanceTo) || direction == Direction.CounterClockwise)
                        n = -stepsToStop; // Start deceleration
                }
                else if (n < 0)
                {
                    // Currently decelerating, need to accel again?
                    if ((stepsToStop < distanceTo) && direction == Direction.Clockwise)
                        n = -n; // Start accceleration
                }
            }
            else if (distanceTo < 0)
            {
                // We are clockwise from the target
                // Need to go anticlockwise from here, maybe decelerate
                if (n > 0)
                {
                    // Currently accelerating, need to decel now? Or maybe going the wrong way?
                    if ((stepsToStop >= -distanceTo) || direction == Direction.Clockwise)
                        n = -stepsToStop; // Start deceleration
                }
                else if (n < 0)
                {
                    // Currently decelerating, need to accel again?
                    if ((stepsToStop < -distanceTo) && direction == Direction.CounterClockwise)
                        n = -n; // Start accceleration
                }
            }

            // Need to accelerate or decelerate
            if (n == 0)
            {
                // First step from stopped
                cn = c0;
                direction = (distanceTo > 0) ? Direction.Clockwise : Direction.CounterClockwise;
            }
            else
            {
                // Subsequent step. Works for accel (n is +_ve) and decel (n is -ve).
                cn = cn - ((2.0f * cn) / ((4.0f * n) + 1)); // Equation 13
                cn = Math.Max(cn, cmin);
            }
            n++;
            stepInterval = (long)cn;
            speed = TimeSpan.TicksPerSecond / cn;
            if (direction == Direction.CounterClockwise)
                speed = -speed;
        }
    }
}
