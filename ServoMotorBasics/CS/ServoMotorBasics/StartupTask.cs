// Copyright (c) Microsoft. All rights reserved.


using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net.Http;
using Windows.ApplicationModel.Background;
using Windows.Devices.Gpio;
using System.Diagnostics;
using Windows.Foundation;
using Windows.System.Threading;
// The Background Application template is documented at http://go.microsoft.com/fwlink/?LinkID=533884&clcid=0x409

namespace ServoMotorBasics
{
    public sealed class StartupTask : IBackgroundTask
    {
        BackgroundTaskDeferral deferral;
        GpioPin servoPin;
        ThreadPoolTimer timer;


        //A pulse of 2ms moves the servo clockwise
        double ForwardPulseWidth = 2;
        //A pulse of 1ms moves the servo counterclockwise
        double BackwardPulseWidth = 1;
        double PulseFrequency = 20;

        double currentPulseWidth;
        Stopwatch stopwatch;
        

        public void Run(IBackgroundTaskInstance taskInstance)
        {
            deferral = taskInstance.GetDeferral();

            //Motor starts off
            currentPulseWidth = 0;

            //The stopwatch will be used to precisely time calls to pulse the motor.
            stopwatch = Stopwatch.StartNew();

            GpioController controller = GpioController.GetDefault();

       


            servoPin = controller.OpenPin(13);
            servoPin.SetDriveMode(GpioPinDriveMode.Output);

            timer = ThreadPoolTimer.CreatePeriodicTimer(this.Tick, TimeSpan.FromSeconds(2));
           

            //You do not need to await this, as your goal is to have this run for the lifetime of the application
            Windows.System.Threading.ThreadPool.RunAsync(this.MotorThread, Windows.System.Threading.WorkItemPriority.High);

        }

        private void Tick(ThreadPoolTimer timer)
        {
            if (currentPulseWidth != ForwardPulseWidth)
            {
                currentPulseWidth = ForwardPulseWidth;
            }else
            {
                currentPulseWidth = BackwardPulseWidth;
            }
        }

        private void MotorThread(IAsyncAction action)
        {
            //This motor thread runs on a high priority task and loops forever to pulse the motor as determined by the drive buttons
            while (true)
            {
                //If a button is pressed the pulsewidth is changed to cause the motor to spin in the appropriate direction
                //Write the pin high for the appropriate length of time
                if (currentPulseWidth != 0)
                {
                    servoPin.Write(GpioPinValue.High);
                }
                //Use the wait helper method to wait for the length of the pulse
                Wait(currentPulseWidth);
                //The pulse if over and so set the pin to low and then wait until it's time for the next pulse
                servoPin.Write(GpioPinValue.Low);
                Wait(PulseFrequency - currentPulseWidth);
            }
        }

       
        //A synchronous wait is used to avoid yielding the thread 
        //This method calculates the number of CPU ticks will elapse in the specified time and spins
        //in a loop until that threshold is hit. This allows for very precise timing.
        private void Wait(double milliseconds)
        {
            long initialTick = stopwatch.ElapsedTicks;
            long initialElapsed = stopwatch.ElapsedMilliseconds;
            double desiredTicks = milliseconds / 1000.0 * Stopwatch.Frequency;
            double finalTick = initialTick + desiredTicks;
            while (stopwatch.ElapsedTicks < finalTick)
            {

            }
        }
    }
}
