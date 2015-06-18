using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net.Http;
using Windows.ApplicationModel.Background;
using Windows.Devices.Gpio;
using System.Diagnostics;
using Windows.Foundation;
// The Background Application template is documented at http://go.microsoft.com/fwlink/?LinkID=533884&clcid=0x409

namespace ServoMotorBasics
{
    public sealed class StartupTask : IBackgroundTask
    {
        BackgroundTaskDeferral _deferral;
        GpioPin _servoPin;
        GpioPin _forwardButton;
        GpioPin _backgwardButton;
        
        
        double ForwardPulseWidth = 2;
        double BackwardPulseWidth = 1;
        double PulseFrequency = 20;

        double _currentPulseWidth;
        Stopwatch _stopwatch;
        

        public void Run(IBackgroundTaskInstance taskInstance)
        {
            _deferral = taskInstance.GetDeferral();

            //Motor starts off
            _currentPulseWidth = 0;

            //The stopwatch will be used to precisely time calls to pulse the motor.
            _stopwatch = Stopwatch.StartNew();

            GpioController controller = GpioController.GetDefault();

            //Buttons are attached to pins 5 and 6 to control which direction the motor should run in
            //Interrupts (ValueChanged) events are used to notify this app when the buttons are pressed
            _forwardButton = controller.OpenPin(5);
            _forwardButton.DebounceTimeout = new TimeSpan(0, 0, 0, 0, 250);
            _forwardButton.SetDriveMode(GpioPinDriveMode.Input);
            _forwardButton.ValueChanged += _forwardButton_ValueChanged;

            _backgwardButton = controller.OpenPin(6);
            _backgwardButton.SetDriveMode(GpioPinDriveMode.Input);
            _forwardButton.DebounceTimeout = new TimeSpan(0, 0, 0, 0, 250);
            _backgwardButton.ValueChanged += _backgwardButton_ValueChanged;


            _servoPin = controller.OpenPin(13);
            _servoPin.SetDriveMode(GpioPinDriveMode.Output);

            
           

            //You do not need to await this, as your goal is to have this run for the lifetime of the application
            Windows.System.Threading.ThreadPool.RunAsync(this.MotorThread, Windows.System.Threading.WorkItemPriority.High);
        }

        private void _backgwardButton_ValueChanged(GpioPin sender, GpioPinValueChangedEventArgs args)
        {
            if (_backgwardButton.Read() == GpioPinValue.Low) 
            {
                _currentPulseWidth = BackwardPulseWidth;
            }else
            {
                _currentPulseWidth = 0;
            }
            
        }

        private void _forwardButton_ValueChanged(GpioPin sender, GpioPinValueChangedEventArgs args)
        {
            if (_forwardButton.Read() == GpioPinValue.Low)
            {
                _currentPulseWidth = ForwardPulseWidth;
            }
            else
            {
                _currentPulseWidth = 0;
            }
        }

        private void MotorThread(IAsyncAction action)
        {
            while (true)
            {
                if (_currentPulseWidth != 0)
                {
                    _servoPin.Write(GpioPinValue.High);
                }
                Wait(_currentPulseWidth);
                _servoPin.Write(GpioPinValue.Low);
                Wait(PulseFrequency - _currentPulseWidth);
            }
        }

       
        //A synchronous wait is used to avoid yielding the thread 
        //This method calculates the number of CPU ticks will elapse in the specified time and spins
        //in a loop until that threshold is hit. This allows for very precise timing.
        private void Wait(double milliseconds)
        {
            long initialTick = _stopwatch.ElapsedTicks;
            long initialElapsed = _stopwatch.ElapsedMilliseconds;
            double desiredTicks = milliseconds / 1000.0 * Stopwatch.Frequency;
            double finalTick = initialTick + desiredTicks;
            while (_stopwatch.ElapsedTicks < finalTick)
            {

            }
        }
    }
}
