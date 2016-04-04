using I2CCompass.Sensors;
using Microsoft.Practices.Prism.Commands;
using Microsoft.Practices.Prism.Mvvm;
using System;
using System.Windows.Input;
using Windows.ApplicationModel.Core;
using Windows.UI.Core;

namespace I2CCompass.ViewModels
{
    public class MainPageViewModel : BindableBase
    {
        private ICompass _compass;
        private CompassReading? _compassReading;

        private DelegateCommand _startContinuousMeasurementsCommand;
        private DelegateCommand _takeSingleMeasurementCommand;
        private DelegateCommand _stopCompassCommand;

        public MainPageViewModel(ICompass compass)
        {
            if (compass == null)
            {
                throw new ArgumentNullException(nameof(compass));
            }
            _compass = compass;
            _compass.CompassReadingChangedEvent += async (e, cr) =>
            {
                await CoreApplication.MainView.CoreWindow.Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () => { CompassReading = cr; });
            };

            _startContinuousMeasurementsCommand = DelegateCommand.FromAsyncHandler(
                async () =>
                {
                    await _compass.SetModeAsync(MagnetometerMeasurementMode.Continuous);
                    UpdateCommands();
                },
                () =>
                {
                    return _compass.MeasurementMode != MagnetometerMeasurementMode.Continuous;
                });

            _takeSingleMeasurementCommand = DelegateCommand.FromAsyncHandler(
                async () =>
                {
                    await _compass.SetModeAsync(MagnetometerMeasurementMode.Single);
                    UpdateCommands();
                },
                () =>
                {
                    return _compass.MeasurementMode != MagnetometerMeasurementMode.Continuous;
                });

            _stopCompassCommand = new DelegateCommand(
                async () =>
                {
                    await _compass.SetModeAsync(MagnetometerMeasurementMode.Idle);
                    UpdateCommands();
                },
                () =>
                {
                    return _compass.MeasurementMode == MagnetometerMeasurementMode.Continuous;
                });
        }

        public ICommand StartContinuousMeasurementsCommand { get { return _startContinuousMeasurementsCommand; } }

        public ICommand TakeSingleMeasurementCommand { get { return _takeSingleMeasurementCommand; } }

        public ICommand StopCompassCommand { get { return _stopCompassCommand; } }

        public CompassReading CompassReading
        {
            get { return _compassReading.HasValue ? _compassReading.Value : new CompassReading(); }
            set { SetProperty(ref _compassReading, value); }
        }

        private void UpdateCommands()
        {
            _startContinuousMeasurementsCommand.RaiseCanExecuteChanged();
            _takeSingleMeasurementCommand.RaiseCanExecuteChanged();
            _stopCompassCommand.RaiseCanExecuteChanged();
        }
    }
}
