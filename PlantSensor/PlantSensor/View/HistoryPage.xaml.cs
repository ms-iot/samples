// Copyright (c) Microsoft. All rights reserved.
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Linq;
using System.Threading.Tasks;
using Windows.System.Threading;
using Windows.UI;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;
using WinRTXamlToolkit.Controls.DataVisualization.Charting;

//If the sensors are meausuring, does the history page update when the next hour comes with the sensor measurements?
namespace PlantSensor
{
    public class ChartDataPoint
    {
        public string Name { get; set; }
        public float Amount { get; set; }
     }
    public sealed partial class HistoryPage : Page
    {
        //The Observable Collection Lists are the lists directly connected to the UI
        private ObservableCollection<ChartDataPoint> SoilMoistureList;
        private ObservableCollection<ChartDataPoint> TemperatureList;
        private ObservableCollection<ChartDataPoint> BrightnessList;

        private ObservableCollection<ChartDataPoint> IdealTemperatureList;
        private ObservableCollection<ChartDataPoint> IdealBrightnessList;
        private ObservableCollection<ChartDataPoint> IdealSoilMoistureList;


        //These lists determine what data to show based on time frame and type of sensor
        private List<ChartDataPoint> masterDayBrightnessList;
        private List<ChartDataPoint> masterHourBrightnessList;
        private List<ChartDataPoint> masterWeekBrightnessList;
        private List<ChartDataPoint> masterDayTemperatureList;
        private List<ChartDataPoint> masterHourTemperatureList;
        private List<ChartDataPoint> masterWeekTemperatureList;
        private List<ChartDataPoint> masterDaySoilMoistureList;
        private List<ChartDataPoint> masterHourSoilMoistureList;
        private List<ChartDataPoint> masterWeekSoilMoistureList;

        //the number of data points required for each of these time frames
        int dataPointsForDay;
        int dataPointsForWeek;
        int dataPointsForTwoWeeks;
        int dataPointsForFourWeeks;
        int dataPointsForEightWeeks;

        //These colors dictate what the tabs that determine time span look like
        Color colorBlue;
        Color colorlightBlue;
        Color colorWhite;
        Color colorlightRed;
        Color colorRed;
        Color colorGrey;
        SolidColorBrush SolidColorBrushGrey;
        SolidColorBrush SolidColorBrushBlue;
        SolidColorBrush SolidColorBrushLightBlue;
        SolidColorBrush SolidColorBrushWhite;
        SolidColorBrush SolidColorBrushLightRed;
        SolidColorBrush SolidColorBrushRed;
        SolidColorBrush Black = new SolidColorBrush(Colors.Black);

        //the delimeter (",") which will split up the data from the date
        Char delimiter;

        /**
         * sets all of the variables that were described above
         **/
        public HistoryPage()
        {
            this.InitializeComponent();

            SoilMoistureList = new ObservableCollection<ChartDataPoint>();
            TemperatureList = new ObservableCollection<ChartDataPoint>();
            BrightnessList = new ObservableCollection<ChartDataPoint>();

            IdealTemperatureList = new ObservableCollection<ChartDataPoint>();
            IdealBrightnessList = new ObservableCollection<ChartDataPoint>();
            IdealSoilMoistureList = new ObservableCollection<ChartDataPoint>();

            masterDayBrightnessList = new List<ChartDataPoint>();
            masterHourBrightnessList = new List<ChartDataPoint>();
            masterWeekBrightnessList = new List<ChartDataPoint>();

            masterDayTemperatureList = new List<ChartDataPoint>();
            masterHourTemperatureList = new List<ChartDataPoint>();
            masterWeekTemperatureList = new List<ChartDataPoint>();

            masterDaySoilMoistureList = new List<ChartDataPoint>();
            masterHourSoilMoistureList = new List<ChartDataPoint>();
            masterWeekSoilMoistureList = new List<ChartDataPoint>();

            dataPointsForDay = 24;
            dataPointsForWeek = 7;
            dataPointsForTwoWeeks = 14;
            dataPointsForFourWeeks = 28;
            dataPointsForEightWeeks = 56;

            colorBlue = Color.FromArgb(255, 105, 210, 231);
            colorlightBlue = Color.FromArgb(255, 167, 219, 216);
            colorWhite = Color.FromArgb(255, 224, 228, 204);
            colorlightRed = Color.FromArgb(255, 243, 134, 48);
            colorRed = Color.FromArgb(255, 250, 105, 0);
            colorGrey = Color.FromArgb(255, 50, 50, 50);
            SolidColorBrushGrey = new SolidColorBrush(colorGrey);
            SolidColorBrushBlue = new SolidColorBrush(colorBlue);
            SolidColorBrushLightBlue = new SolidColorBrush(colorlightBlue);
            SolidColorBrushWhite = new SolidColorBrush(colorWhite);
            SolidColorBrushLightRed = new SolidColorBrush(colorlightRed);
            SolidColorBrushRed = new SolidColorBrush(colorRed);

            delimiter = ',';
        }

        /**
         * This method is reached when the history page is reached
         * It sets up all of the lists for the various timeframes.
         * It also automatically loads one week as the default time frame
         **/
        protected async override void OnNavigatedTo(NavigationEventArgs navArgs)
        {
            Stopwatch stopWatch = new Stopwatch();
            stopWatch.Start();
            await setUpChart();
            stopWatch.Stop();
            Debug.WriteLine("Set up chart" + stopWatch.Elapsed);

            stopWatch.Start();
            await resetPreviousUI();

            //These run simultaneously so should not be awaited
            ThreadPool.RunAsync(async (s) =>
            {
                Stopwatch sw = new Stopwatch();
                sw.Start();
                var generalHourDictionary = setUpGeneralMasterListPerHour(App.Brightnessresult, masterHourBrightnessList);
                setUpGeneralMasterListPerDay(generalHourDictionary, masterDayBrightnessList);
                setUpGeneralMasterListPerWeek(masterDayBrightnessList, masterWeekBrightnessList);
                sw.Stop();
                Debug.WriteLine("Set up brightness chart" + stopWatch.Elapsed);
                await reloadChart(BrightnessList, masterDayBrightnessList, Math.Min(masterDayBrightnessList.Count, dataPointsForWeek));
                await createUI(IdealBrightnessList, BrightnessList, App.PlantSettings.IdealBright);
                await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
                {
                    HistoryBrightnessProgressRing.IsActive = false;
                });
            });

            ThreadPool.RunAsync(async (s) =>
            {
                Stopwatch sw = new Stopwatch();
                sw.Start();
                var generalHourDictionary = setUpGeneralMasterListPerHour(App.Temperatureresult, masterHourTemperatureList);
                setUpGeneralMasterListPerDay(generalHourDictionary, masterDayTemperatureList);
                setUpGeneralMasterListPerWeek(masterDayTemperatureList, masterWeekTemperatureList);
                sw.Stop();
                Debug.WriteLine("Set up temperature chart" + stopWatch.Elapsed);
                await reloadChart(TemperatureList, masterDayTemperatureList, Math.Min(masterDayTemperatureList.Count, dataPointsForWeek));
                await createUI(IdealTemperatureList, TemperatureList, App.PlantSettings.IdealTemp);
                await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
                {
                    HistoryTemperatureProgressRing.IsActive = false;
                });
            });

            ThreadPool.RunAsync(async (s) =>
            {
                Stopwatch sw = new Stopwatch();
                sw.Start();
                var generalHourDictionary = setUpGeneralMasterListPerHour(App.SoilMoistureresult, masterHourSoilMoistureList);
                setUpGeneralMasterListPerDay(generalHourDictionary, masterDaySoilMoistureList);
                setUpGeneralMasterListPerWeek(masterDaySoilMoistureList, masterWeekSoilMoistureList);
                sw.Stop();
                Debug.WriteLine("Set up soil chart" + stopWatch.Elapsed);
                await reloadChart(SoilMoistureList, masterDaySoilMoistureList, Math.Min(masterDaySoilMoistureList.Count, dataPointsForWeek));
                await createUI(IdealSoilMoistureList, SoilMoistureList, App.PlantSettings.IdealSoilMoist);
                await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
                {
                    HistorySoilMoistProgressRing.IsActive = false;
                });
            });

            stopWatch.Stop();
            Debug.WriteLine("Finished!" + stopWatch.Elapsed);

            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, async () =>
            {
                await resetTemperatureButtonColor();
                await resetBrightnessButtonColor();
                await resetSoilMoistureButtonColor();
                d3BrightnessButton.Foreground = Black;
                d3BrightnessButton.Background = SolidColorBrushLightBlue;
                d3TemperatureButton.Foreground = Black;
                d3TemperatureButton.Background = SolidColorBrushLightBlue;
                d3SoilMoistureButton.Foreground = Black;
                d3SoilMoistureButton.Background = SolidColorBrushLightBlue;
            });
        }

        /**
         * attaches the list data structures to the chart
         * */
        public async Task setUpChart()
        {
            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                (SoilMoistureChart.Series[0] as LineSeries).ItemsSource = SoilMoistureList;
                (TemperatureChart.Series[0] as LineSeries).ItemsSource = TemperatureList;
                (BrightnessChart.Series[0] as LineSeries).ItemsSource = BrightnessList;

                (TemperatureChart.Series[1] as LineSeries).ItemsSource = IdealTemperatureList;
                (BrightnessChart.Series[1] as LineSeries).ItemsSource = IdealBrightnessList;
                (SoilMoistureChart.Series[1] as LineSeries).ItemsSource = IdealSoilMoistureList;
            });

        }

/**
    * sets up the list by hour
    * returns hourDictionary: the hour dictionary will be used in the day sorting method 
    * */
public Dictionary<DateTime, List<float>> setUpGeneralMasterListPerHour(IList<string> fileList, List<ChartDataPoint> masterList)
{
    //creates a dictionary where all of the information is stored.
    Dictionary<DateTime, List<float>>  hourDictionary = new Dictionary<DateTime, List<float>>();
    DateTime upperBound = DateTime.Now;
    TimeSpan fiftySixDays = new TimeSpan(56,0, 0, 0);
    TimeSpan twentyFourHours = new TimeSpan(24, 0, 0);
    //the earliest date that the chart will show is in the lowerBound variable
    DateTime lowerBound = upperBound - fiftySixDays;
    DateTime currentDate;
    DateTime comparedDate = DateTime.Now;
    DateTime oldDate;
    //goes through every data point that the sensors have collected
    for(int ii = 0; ii < fileList.Count; ii++)
    {
        String nextString = fileList[ii];
        String[] subStrings = nextString.Split(delimiter);
        //nextFloat holds the sensor data
        float nextFloat = float.Parse(subStrings[0]);
        //currentDate holds the time that the nextFloat value was measured
        currentDate = DateTime.Parse(subStrings[1]);
        if (currentDate > lowerBound && currentDate <= upperBound)
        {
            //the minute and the second data is not valuable in the hour view
            comparedDate = new DateTime(currentDate.Year, currentDate.Month, currentDate.Day, currentDate.Hour, 0, 0);
            if (hourDictionary.ContainsKey(comparedDate))
            {
                hourDictionary[comparedDate].Add(nextFloat);
                //changes the date
                oldDate = comparedDate;
            }
            else
            {
                List<float> newList = new List<float>();
                newList.Add(nextFloat);
                hourDictionary.Add(comparedDate, newList);
            }
        }
    }
    foreach(DateTime hour in hourDictionary.Keys)
    {
        //we only need the last twenty four hours
        lowerBound = comparedDate - twentyFourHours;
        if (hour > lowerBound && hour <= comparedDate)
        {
            masterList.Add(new ChartDataPoint() { Name = hour.ToString("HH:00"), Amount = hourDictionary[hour].Average() });
        }
    }
    return hourDictionary;
}

        /**
         * sets up the list by day
         * */
        public void setUpGeneralMasterListPerDay(Dictionary<DateTime, List<float>> hourDictionary, List<ChartDataPoint> masterList)
        {
            //the DateTime in the dictionary is the day the sensors got their measurements
            //the list is the list of values that the sensor measured during that day
            var dayDictionary = new Dictionary<DateTime, List<float>>();
            DateTime upperBound = DateTime.Now;
            //my graph does not display more than 56 days, so there is no reason to be reading
            //data from before that time
            TimeSpan fiftySixDays = new TimeSpan(56, 0, 0, 0);
            DateTime lowerBound = upperBound - fiftySixDays;
            DateTime currentDate;
            foreach(DateTime comparedDate in hourDictionary.Keys)
            {
                currentDate = new DateTime(comparedDate.Year, comparedDate.Month, comparedDate.Day, 0, 0, 0);
                if (currentDate >= lowerBound && currentDate <= upperBound)
                {
                    //checks to see if its a new day. If not then, the sensor value will be added onto the
                    //already existing day. If it is a new day, then a new key is made in the dictionary
                    if (dayDictionary.ContainsKey(currentDate))
                    {
                        dayDictionary[currentDate].Add(hourDictionary[comparedDate].Average());
                    }
                    else
                    {
                        List<float> newList = new List<float>();
                        newList.Add(hourDictionary[comparedDate].Average());
                        dayDictionary.Add(currentDate, newList);
                    }
                }
            }
            foreach (DateTime day in dayDictionary.Keys)
            {
                masterList.Add(new ChartDataPoint() { Name = day.ToString("MM/dd/yy"), Amount = dayDictionary[day].Average() });
            }

        }
        /**
         * sets up the list by week
         * */
        public void setUpGeneralMasterListPerWeek(List<ChartDataPoint> masterDayList, List<ChartDataPoint> masterWeekList)
        {
            int daysInWeek = 7;
            int numberOfWeeks = dataPointsForEightWeeks / daysInWeek;
            //the number of weeks is set, but the number of data points per week
            //is not, so an array of lists is used. 
            List<float>[] weekList = new List<float>[numberOfWeeks];

            TimeSpan oneWeek = new TimeSpan(7, 0, 0, 0);
            DateTime firstDate = DateTime.Parse(masterDayList[0].Name);
            DateTime[] weekDates = new DateTime[numberOfWeeks];
            //loops through the number of weeks and assign lists and floats
            for (int ii=0; ii< numberOfWeeks; ii++)
            {
                weekList[ii] = new List<float>();
                weekDates[ii] = firstDate + TimeSpan.FromTicks(oneWeek.Ticks * (ii+1)); 
            }
            int weekCounter = 0;
            //loops through the masterDayList(the average value of the sensors per day)
            //and identifies the date into the week
            for(int ii=0; ii<masterDayList.Count(); ii++)
            {
                DateTime comparedDate = DateTime.Parse(masterDayList[ii].Name);
                if(comparedDate>weekDates[weekCounter])
                {
                    for (int jj = weekCounter; jj < weekDates.Count(); jj++)
                    {
                        if (comparedDate < weekDates[jj])
                        {
                            weekCounter = jj;
                            break;
                        }
                    }
                }
                //adds the value to that week
                float nextFloat = masterDayList[ii].Amount;
                weekList[weekCounter].Add(nextFloat);
            }

            for(int ii=0; ii<weekList.Count();ii++)
            {
                //if size of list is zero, then it shouldnt be on the list
                if (weekList[ii].Count()!=0)
                {
                    DateTime beginningOfWeek = weekDates[ii] - oneWeek;
                    String weekName = beginningOfWeek.ToString("MM/dd") + "-" + weekDates[ii].ToString("MM/dd");
                    masterWeekList.Add(new ChartDataPoint() { Name = weekName, Amount = weekList[ii].Average() });
                }
            }
        }

        /**
         * when the user wants to change time frames, then the UI gets reset to make it look like the user has changed time
         * */
        public async Task resetPreviousUI()
        {
            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                BrightnessList.Clear();
                TemperatureList.Clear();
                SoilMoistureList.Clear();
            });

        }

        /**
         * When the user wants to change time frames, the chart reloads
         * */
        public async Task reloadChart(ObservableCollection<ChartDataPoint> list, List<ChartDataPoint> masterList, int subtraction)
        {
            Debug.WriteLine("reloadChart");
            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                list.Clear();

                for (int ii = masterList.Count - subtraction; ii < masterList.Count; ii++)
                {
                    list.Add(masterList[ii]);
                }
            });
        }

        //should be float
        public async Task createUI(ObservableCollection<ChartDataPoint> idealList, ObservableCollection<ChartDataPoint> list, float settings)
        {
            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                idealList.Clear();
            foreach (ChartDataPoint point in list)
                {
                    idealList.Add(new ChartDataPoint()
                    {
                        Name = point.Name,
                        Amount = settings
                    });
                }
            });
        }
        /**
         * These reset the UI for the different sensors when user changes time frame
         * */
        public async Task resetTemperatureButtonColor()
        {
            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                d1TemperatureButton.Foreground = SolidColorBrushBlue;
                d1TemperatureButton.Background = SolidColorBrushGrey;
                d3TemperatureButton.Foreground = SolidColorBrushLightBlue;
                d3TemperatureButton.Background = SolidColorBrushGrey;
                d5TemperatureButton.Foreground = SolidColorBrushWhite;
                d5TemperatureButton.Background = SolidColorBrushGrey;
                d7TemperatureButton.Foreground = SolidColorBrushLightRed;
                d7TemperatureButton.Background = SolidColorBrushGrey;
                d9TemperatureButton.Foreground = SolidColorBrushRed;
                d9TemperatureButton.Background = SolidColorBrushGrey;
            });
        }

        public async Task resetBrightnessButtonColor()
        {
            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                d1BrightnessButton.Foreground = SolidColorBrushBlue;
                d1BrightnessButton.Background = SolidColorBrushGrey;
                d3BrightnessButton.Foreground = SolidColorBrushLightBlue;
                d3BrightnessButton.Background = SolidColorBrushGrey;
                d5BrightnessButton.Foreground = SolidColorBrushWhite;
                d5BrightnessButton.Background = SolidColorBrushGrey;
                d7BrightnessButton.Foreground = SolidColorBrushLightRed;
                d7BrightnessButton.Background = SolidColorBrushGrey;
                Brightness9dButton.Foreground = SolidColorBrushRed;
                Brightness9dButton.Background = SolidColorBrushGrey;
            });
        }

        public async Task resetSoilMoistureButtonColor()
        {
            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                d1SoilMoistureButton.Foreground = SolidColorBrushBlue;
                d1SoilMoistureButton.Background = SolidColorBrushGrey;
                d3SoilMoistureButton.Foreground = SolidColorBrushLightBlue;
                d3SoilMoistureButton.Background = SolidColorBrushGrey;
                d5SoilMoistureButton.Foreground = SolidColorBrushWhite;
                d5SoilMoistureButton.Background = SolidColorBrushGrey;
                d7SoilMoistureButton.Foreground = SolidColorBrushLightRed;
                d7SoilMoistureButton.Background = SolidColorBrushGrey;
                d9SoilMoistureButton.Foreground = SolidColorBrushRed;
                d9SoilMoistureButton.Background = SolidColorBrushGrey;
            });
        }
        /**
         * All of these reset the list in the chart when the user changes tabs
         * */
        private async void d1BrightnessButton_Click(object sender, RoutedEventArgs e)
        {
            await reloadChart(BrightnessList, masterHourBrightnessList, Math.Min(masterHourBrightnessList.Count,dataPointsForDay));
            await createUI(IdealBrightnessList, BrightnessList, App.PlantSettings.IdealBright);
            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, async () =>
            {
                BrightnessUnits.Text = "Hours";
                await resetBrightnessButtonColor();
                d1BrightnessButton.Foreground = Black;
                d1BrightnessButton.Background = SolidColorBrushBlue;
            });
        }

        private async void d3BrightnessButton_Click(object sender, RoutedEventArgs e)
        {
            await reloadChart(BrightnessList, masterDayBrightnessList, Math.Min(masterDayBrightnessList.Count,dataPointsForWeek));
            await createUI(IdealBrightnessList, BrightnessList, App.PlantSettings.IdealBright);
            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, async () =>
            {
                BrightnessUnits.Text = "Days";
                await resetBrightnessButtonColor();
                d3BrightnessButton.Foreground = Black;
                d3BrightnessButton.Background = SolidColorBrushLightBlue;
            });
        }

        private async void d5BrightnessButton_Click(object sender, RoutedEventArgs e)
        {
            await reloadChart(BrightnessList, masterDayBrightnessList, Math.Min(masterDayBrightnessList.Count,dataPointsForTwoWeeks));
            await createUI(IdealBrightnessList, BrightnessList, App.PlantSettings.IdealBright);
            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, async () =>
            {
                BrightnessUnits.Text = "Days";
                await resetBrightnessButtonColor();
                d5BrightnessButton.Foreground = Black;
                d5BrightnessButton.Background = SolidColorBrushWhite;
            });
        }

        private async void d7BrightnessButton_Click(object sender, RoutedEventArgs e)
        {
            await reloadChart(BrightnessList, masterDayBrightnessList, Math.Min(masterDayBrightnessList.Count, dataPointsForFourWeeks));
            await createUI(IdealBrightnessList, BrightnessList, App.PlantSettings.IdealBright);
            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, async () =>
            {
                BrightnessUnits.Text = "Days";
                await resetBrightnessButtonColor();
                d7BrightnessButton.Foreground = Black;
                d7BrightnessButton.Background = SolidColorBrushLightRed;
            });
        }

        private async void Brightness9dButton_Click(object sender, RoutedEventArgs e)
        {
            await reloadChart(BrightnessList, masterWeekBrightnessList, Math.Min(masterWeekBrightnessList.Count,dataPointsForEightWeeks));
            await createUI(IdealBrightnessList, BrightnessList, App.PlantSettings.IdealBright);
            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, async () =>
            {
                BrightnessUnits.Text = "Weeks";
                await resetBrightnessButtonColor();
                Brightness9dButton.Foreground = Black;
                Brightness9dButton.Background = SolidColorBrushRed;
            });
        }

        private async void d1TemperatureButton_Click(object sender, RoutedEventArgs e)
        {
            await reloadChart(TemperatureList, masterHourTemperatureList, Math.Min(masterHourTemperatureList.Count,dataPointsForDay));
            await createUI(IdealTemperatureList, TemperatureList, App.PlantSettings.IdealTemp);
            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, async () =>
            {
                TemperatureUnits.Text = "Hours";
                await resetTemperatureButtonColor();
                d1TemperatureButton.Foreground = Black;
                d1TemperatureButton.Background = SolidColorBrushBlue;
            });
        }

        private async void d3TemperatureButton_Click(object sender, RoutedEventArgs e)
        {
            await reloadChart(TemperatureList, masterDayTemperatureList, Math.Min(masterDayTemperatureList.Count,dataPointsForWeek));
            await createUI(IdealTemperatureList, TemperatureList, App.PlantSettings.IdealTemp);
            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, async () =>
            {
                TemperatureUnits.Text = "Days";
                await resetTemperatureButtonColor();
                d3TemperatureButton.Foreground = Black;
                d3TemperatureButton.Background = SolidColorBrushLightBlue;
            });
        }

        private async void d5TemperatureButton_Click(object sender, RoutedEventArgs e)
        {
            await reloadChart(TemperatureList, masterDayTemperatureList, Math.Min(masterDayTemperatureList.Count,dataPointsForTwoWeeks));
            await createUI(IdealTemperatureList, TemperatureList, App.PlantSettings.IdealTemp);
            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, async () =>
            {
                TemperatureUnits.Text = "Days";
                await resetTemperatureButtonColor();
                d5TemperatureButton.Foreground = Black;
                d5TemperatureButton.Background = SolidColorBrushWhite;
            });
        }

        private async void d7TemperatureButton_Click(object sender, RoutedEventArgs e)
        {
            await reloadChart(TemperatureList, masterDayTemperatureList, Math.Min(masterDayTemperatureList.Count, dataPointsForFourWeeks));
            await createUI(IdealTemperatureList, TemperatureList, App.PlantSettings.IdealTemp);
            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, async () =>
            {
                TemperatureUnits.Text = "Days";
                await resetTemperatureButtonColor();
                d7TemperatureButton.Foreground = Black;
                d7TemperatureButton.Background = SolidColorBrushLightRed;
            });
        }

        private async void d9TemperatureButton_Click(object sender, RoutedEventArgs e)
        {
            await reloadChart(TemperatureList, masterWeekTemperatureList, Math.Min(masterWeekTemperatureList.Count,dataPointsForEightWeeks));
            await createUI(IdealTemperatureList, TemperatureList, App.PlantSettings.IdealTemp);
            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, async () =>
            {
                TemperatureUnits.Text = "Weeks";
                await resetTemperatureButtonColor();
                d9TemperatureButton.Foreground = Black;
                d9TemperatureButton.Background = SolidColorBrushRed;
            });
        }

        private async void d1SoilMoistureButton_Click(object sender, RoutedEventArgs e)
        {
            await reloadChart(SoilMoistureList, masterHourSoilMoistureList, Math.Min(masterHourSoilMoistureList.Count,dataPointsForDay));
            await createUI(IdealSoilMoistureList, SoilMoistureList, App.PlantSettings.IdealSoilMoist);
            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, async () =>
            {
                SoilMoistureUnits.Text = "Hours";
                await resetSoilMoistureButtonColor();
                d1SoilMoistureButton.Foreground = Black;
                d1SoilMoistureButton.Background = SolidColorBrushBlue;
            });
        }

        private async void d3SoilMoistureButton_Click(object sender, RoutedEventArgs e)
        {
            await reloadChart(SoilMoistureList, masterDaySoilMoistureList, Math.Min(masterDaySoilMoistureList.Count,dataPointsForWeek));
            await createUI(IdealSoilMoistureList, SoilMoistureList, App.PlantSettings.IdealSoilMoist);
            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, async () =>
            {
                SoilMoistureUnits.Text = "Days";
                await resetSoilMoistureButtonColor();
                d3SoilMoistureButton.Foreground = Black;
                d3SoilMoistureButton.Background = SolidColorBrushLightBlue;
            });
        }

        private async void d5SoilMoistureButton_Click(object sender, RoutedEventArgs e)
        {
            await reloadChart(SoilMoistureList, masterDaySoilMoistureList, Math.Min(masterDaySoilMoistureList.Count,dataPointsForTwoWeeks));
            await createUI(IdealSoilMoistureList, SoilMoistureList, App.PlantSettings.IdealSoilMoist);
            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, async () =>
            {
                SoilMoistureUnits.Text = "Days";
                await resetSoilMoistureButtonColor();
                d5SoilMoistureButton.Foreground = Black;
                d5SoilMoistureButton.Background = SolidColorBrushWhite;
            });
        }

        private async void d7SoilMoistureButton_Click(object sender, RoutedEventArgs e)
        {
            await reloadChart(SoilMoistureList, masterDaySoilMoistureList, Math.Min(masterDaySoilMoistureList.Count,dataPointsForFourWeeks));
            await createUI(IdealSoilMoistureList, SoilMoistureList, App.PlantSettings.IdealSoilMoist);
            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, async () =>
            {
                SoilMoistureUnits.Text = "Days";
                await resetSoilMoistureButtonColor();
                d7SoilMoistureButton.Foreground = Black;
                d7SoilMoistureButton.Background = SolidColorBrushLightRed;
            });
        }

        private async void d9SoilMoistureButton_Click(object sender, RoutedEventArgs e)
        {
            await reloadChart(SoilMoistureList, masterWeekSoilMoistureList, Math.Min(masterWeekSoilMoistureList.Count,dataPointsForEightWeeks));
            await createUI(IdealSoilMoistureList, SoilMoistureList, App.PlantSettings.IdealSoilMoist);
            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, async () =>
            {
                SoilMoistureUnits.Text = "Weeks";
                await resetSoilMoistureButtonColor();
                d9SoilMoistureButton.Foreground = Black;
                d9SoilMoistureButton.Background = SolidColorBrushRed;
            });
        }

        /**
         * When the user returns to the last page
         * */
        private void return_Click(object sender, RoutedEventArgs e)
        {
            Frame.Navigate(typeof(MainPage));
        }

        private void Setting_Click(object sender, RoutedEventArgs e)
        {
            Frame.Navigate(typeof(SettingsPage));
        }

        private void Twitter_Click(object sender, RoutedEventArgs e)
        {
            Frame.Navigate(typeof(TwitterPage));
        }

        private void AppBarButton_Click(object sender, RoutedEventArgs e)
        {

        }
    }
}
