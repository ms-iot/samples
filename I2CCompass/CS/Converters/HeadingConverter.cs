using I2CCompass.Sensors;
using System;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Data;

namespace I2CCompass.Converters
{
    public class HeadingConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, string language)
        {
            if (value is CompassReading)
            {
                return string.Format("{0:0.00}°", ((CompassReading)value).Heading);
            }
            return DependencyProperty.UnsetValue;
        }

        public object ConvertBack(object value, Type targetType, object parameter, string language)
        {
            return DependencyProperty.UnsetValue;
        }
    }
}
