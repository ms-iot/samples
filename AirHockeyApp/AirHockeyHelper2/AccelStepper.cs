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

   This code has been adapted from AccelStepper.h taken from http://www.airspayce.com/mikem/arduino/AccelStepper/AccelStepper_8h_source.html
   Copyright (C) 2009-2013 Mike McCauley  200 // $Id: AccelStepper.h,v 1.21 2014/10/31 06:05:30 mikem Exp mikem $
**/

using System;
using System.Diagnostics;
using Windows.Devices.Gpio;

namespace AirHockeyHelper2
{
    public class AccelStepper
    {
        Stopwatch stopwatch;
        Direction _direction;

        public bool Debug = false;

        #region Private variables

        GpioPin _motorPin;

        GpioPin _directionPin;

        GpioPinValue _clockwiseValue;

        /// The current absolution position in steps.
        long _currentPos;    // Steps

        /// The target position in steps. The AccelStepper library will move the
        /// motor from the _currentPos to the _targetPos, taking into account the
        /// max speed, acceleration and deceleration
        long _targetPos;     // Steps

        /// The current motos speed in steps per second
        /// Positive is clockwise
        float _speed;         // Steps per second

        /// The maximum permitted speed in steps per second. Must be > 0.
        float _maxSpeed;

        /// The acceleration to use to accelerate or decelerate the motor in steps
        /// per second per second. Must be > 0
        float _acceleration;

        /// The current interval between steps in ticks.
        /// 0 means the motor is currently stopped with _speed == 0
        long _stepInterval;

        /// The last step time in ticks
        long _lastStepTime;

        /// The step counter for speed calculations
        long _n;

        /// Initial step size in ticks
        float _c0;

        /// Last step size in ticks
        float _cn;

        /// Min step size in ticks based on maxSpeed
        float _cmin; // at max speed

        object runLock = new object();

        #endregion

        enum Direction
        {
            Clockwise,
            CounterClockwise
        }

        public AccelStepper(GpioPin motorPin, GpioPin directionPin, GpioPinValue clockwiseValue)
        {
            _motorPin = motorPin;
            _directionPin = directionPin;
            _clockwiseValue = clockwiseValue;

            stopwatch = new Stopwatch();
            stopwatch.Start();
        }

        public void MoveTo(long absolute)
        {
            if (_targetPos != absolute)
            {
                _targetPos = absolute;
                computeNewSpeed();
            }
        }

        public void Move(long relative)
        {
            MoveTo(_currentPos + relative);
        }

        public bool Run()
        {
            lock (runLock)
            {
                if (RunSpeed())
                {
                    computeNewSpeed();
                }

                return _speed != 0 || DistanceToGo() != 0;
            }
        }

        public bool RunSpeed()
        {
            if (_stepInterval == 0)
            {
                return false;
            }

            long time = stopwatch.ElapsedTicks;
            long nextStepTime = _lastStepTime + _stepInterval;

            if (((nextStepTime >= _lastStepTime) && ((time >= nextStepTime) || (time < _lastStepTime)))
                   || ((nextStepTime < _lastStepTime) && ((time >= nextStepTime) && (time < _lastStepTime))))
            {
                if (_direction == Direction.Clockwise)
                {
                    // Clockwise
                    _currentPos += 1;
                }
                else
                {
                    // Anticlockwise  
                    _currentPos -= 1;
                }
                Step(_currentPos);

                _lastStepTime = time;
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
                if (_speed > 0)
                {
                    _directionPin.Write(_clockwiseValue);
                }
                else
                {
                    _directionPin.Write((_clockwiseValue == GpioPinValue.High) ? GpioPinValue.Low : GpioPinValue.High);
                }

                _motorPin.Write(GpioPinValue.Low);
                _motorPin.Write(GpioPinValue.High);
            }
        }

        public void SetMaxSpeed(float speed)
        {
            if (_maxSpeed != speed)
            {
                _maxSpeed = speed;
                _cmin = TimeSpan.TicksPerSecond / speed;
                // Recompute _n from current speed and adjust speed if accelerating or cruising
                if (_n > 0)
                {
                    _n = (long)((_speed * _speed) / (2.0 * _acceleration)); // Equation 16
                    computeNewSpeed();
                }
            }
        }

        public float Acceleration()
        {
            return _acceleration;
        }

        public void SetAcceleration(float acceleration)
        {
            if (acceleration == 0.0)
                return;
            if (_acceleration != acceleration)
            {
                // Recompute _n per Equation 17
                _n = (long)(_n * (_acceleration / acceleration));
                // New c0 per Equation 7, with correction per Equation 15
                _c0 = (float)(0.676 * Math.Sqrt(2.0 / acceleration) * TimeSpan.TicksPerSecond); // Equation 15
                _acceleration = acceleration;
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

        public void SetSpeed(float speed)
        {
            if (speed == _speed)
                return;
            speed = Constrain(speed, -_maxSpeed, _maxSpeed);
            if (speed == 0.0)
                _stepInterval = 0;
            else
            {
                _stepInterval = (long)Math.Abs(TimeSpan.TicksPerSecond / speed);
                _direction = (speed > 0.0) ? Direction.Clockwise : Direction.CounterClockwise;
            }
            _speed = speed;
        }

        public float Speed()
        {
            return _speed;
        }

        public long DistanceToGo()
        {
            return _targetPos - _currentPos;
        }

        public long TargetPosition()
        {
            return _targetPos;
        }

        public long CurrentPosition()
        {
            return _currentPos;
        }

        public float MaxSpeed()
        {
            return _maxSpeed;
        }

        public void SetCurrentPosition(long position)
        {
            _targetPos = _currentPos = position;
            _n = 0;
            _stepInterval = 0;
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
            if (_targetPos == _currentPos)
                return false;
            if (_targetPos > _currentPos)
                _direction = Direction.Clockwise;
            else
                _direction = Direction.CounterClockwise;
            return RunSpeed();
        }

        public void Stop()
        {
            if (_speed != 0.0)
            {
                long stepsToStop = (long)((_speed * _speed) / (2.0 * _acceleration)) + 1; // Equation 16 (+integer rounding)
                if (_speed > 0)
                    Move(stepsToStop);
                else
                    Move(-stepsToStop);
            }
        }

        protected void computeNewSpeed()
        {
            long distanceTo = DistanceToGo(); // +ve is clockwise from curent location

            long stepsToStop = (long)((_speed * _speed) / (2.0 * _acceleration)); // Equation 16

            if (distanceTo == 0 && stepsToStop <= 1)
            {
                // We are at the target and its time to stop
                _stepInterval = 0;
                _speed = 0;
                _n = 0;
                return;
            }

            if (distanceTo > 0)
            {
                // We are anticlockwise from the target
                // Need to go clockwise from here, maybe decelerate now
                if (_n > 0)
                {
                    // Currently accelerating, need to decel now? Or maybe going the wrong way?
                    if ((stepsToStop >= distanceTo) || _direction == Direction.CounterClockwise)
                        _n = -stepsToStop; // Start deceleration
                }
                else if (_n < 0)
                {
                    // Currently decelerating, need to accel again?
                    if ((stepsToStop < distanceTo) && _direction == Direction.Clockwise)
                        _n = -_n; // Start accceleration
                }
            }
            else if (distanceTo < 0)
            {
                // We are clockwise from the target
                // Need to go anticlockwise from here, maybe decelerate
                if (_n > 0)
                {
                    // Currently accelerating, need to decel now? Or maybe going the wrong way?
                    if ((stepsToStop >= -distanceTo) || _direction == Direction.Clockwise)
                        _n = -stepsToStop; // Start deceleration
                }
                else if (_n < 0)
                {
                    // Currently decelerating, need to accel again?
                    if ((stepsToStop < -distanceTo) && _direction == Direction.CounterClockwise)
                        _n = -_n; // Start accceleration
                }
            }

            // Need to accelerate or decelerate
            if (_n == 0)
            {
                // First step from stopped
                _cn = _c0;
                _direction = (distanceTo > 0) ? Direction.Clockwise : Direction.CounterClockwise;
            }
            else
            {
                // Subsequent step. Works for accel (n is +_ve) and decel (n is -ve).
                _cn = _cn - ((2.0f * _cn) / ((4.0f * _n) + 1)); // Equation 13
                _cn = Math.Max(_cn, _cmin);
            }
            _n++;
            _stepInterval = (long)_cn;
            _speed = TimeSpan.TicksPerSecond / _cn;
            if (_direction == Direction.CounterClockwise)
                _speed = -_speed;
        }
    }
}
