// Copyright (c) Microsoft. All rights reserved.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Devices.Gpio;

namespace AirHockeyHelper
{
    // This class needs to be implemented by the user
    public class StepperLib
    {
        public long CurrentPosition;
        public long TargetPosition;
        public float MaxSpeed;
        public float Acceleration;
        public float Speed;
        public bool Debug = false;

        public StepperLib(GpioPin stepPin, GpioPin dirPin, GpioPinValue clockwiseVal)
        {

        }

        /// <summary>
        /// Sets the target destination for the stepper motor to an absolute position
        /// </summary>
        /// <param name="absolute"></param>
        public void MoveTo(long absolute)
        {

        }

        /// <summary>
        /// Sets the target destination for the stepper motor to a relative position
        /// </summary>
        /// <param name="relative"></param>
        public void Move(long relative)
        {

        }

        /// <summary>
        /// Calculates whether it is time to move the motor one step and does so.  Takes into account acceleration and new target locations.
        /// </summary>
        /// <returns></returns>
        public bool Run()
        {
            return true;
        }

        /// <summary>
        /// Calculates whether it is time to move the motor one step and does so.  This method is used for constant speed movement.
        /// </summary>
        /// <returns></returns>
        public bool RunSpeed()
        {
            return true;
        }

        /// <summary>
        /// Steps the motors
        /// </summary>
        /// <param name="step"></param>
        public void Step(long step)
        {
            
        }
        
        /// <summary>
        /// Distance from current location to target location, in steps
        /// </summary>
        /// <returns></returns>
        public long DistanceToGo()
        {
            return 0;
        }

        /// <summary>
        /// Sets the current position
        /// </summary>
        /// <param name="position"></param>
        public void SetCurrentPosition(long position)
        {

        }

        /// <summary>
        /// Steps the motors according to acceleration and speed until it reaches the target position
        /// </summary>
        public void RunToPosition()
        {
            while (Run()) ;
        }

        /// <summary>
        /// Sets a new target position and moves the motors until it reaches the new destination
        /// </summary>
        /// <param name="position"></param>
        public void RunToNewPosition(long position)
        {
            MoveTo(position);
            RunToPosition();
        }

        /// <summary>
        /// Sets a new target position and speed, and moves the motors at constant speed until it reaches the new destination
        /// </summary>
        /// <param name="position"></param>
        /// <param name="speed"></param>
        public void RunSpeedToNewPosition(long position, float speed)
        {

        }

        /// <summary>
        /// Decelerates the motor to a stop
        /// </summary>
        public void Stop()
        {

        }
    }
}
