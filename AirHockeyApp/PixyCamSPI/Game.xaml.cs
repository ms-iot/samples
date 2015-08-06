// Copyright (c) Microsoft. All rights reserved.

using System;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using System.Threading.Tasks;
using Windows.UI.Xaml.Navigation;
using Windows.Foundation;
using Windows.UI.Xaml.Shapes;
using Windows.UI;
using Windows.UI.Xaml.Media;
using System.Collections.Generic;
using Windows.System.Threading;
using AirHockeyHelper;

#pragma warning disable CS4014 // Because this call is not awaited, execution of the current method continues before the call is completed

namespace AirHockeyApp
{
    public enum GameMode
    {
        Diagnostics,
        Game,
        Mirror,
        Test
    }

    public sealed partial class DiagnosticsPage : Page
    {
        GameMode gameMode = GameMode.Diagnostics;

        PixyCam pixyCam;
        Robot robot;

        Line trajectoryLine;
        List<Line> lineList;
        List<Ellipse> dotList;

        private long robotScore = 0, playerScore = 0;

        private double virtualWidth = 1366, virtualHeight = 766;

        private bool mirrorMode = false;

        private bool stopThread = false;
        private IAsyncAction mainThread;

        public DiagnosticsPage()
        {
            this.InitializeComponent();

            CoordinateHelper.Initialize(mainCanvas.Width, mainCanvas.Height);

            pixyCam = new PixyCam();
            robot = new Robot();

            // Initialize robot's max speed and acceleration
            robot.StepperX.SetMaxSpeed(100000);
            robot.StepperY.SetMaxSpeed(30000);

            robot.StepperX.SetAcceleration(Config.MOTOR_X_ACCELERATION);
            robot.StepperY.SetAcceleration(Config.MOTOR_Y_ACCELERATION);

            // Adds event listeners for when a goal is scored
            robot.HumanGoalSensorTriggered += Robot_HumanGoalSensorTriggered;
            robot.RobotGoalSensorTriggered += Robot_RobotGoalSensorTriggered;

            initializeUI();
        }

        // Draw scoreboard on screen
        private void initializeUI()
        {
            popUpTextBlock.Text = "";

            // Set score
            setScoreText("Robot: " + robotScore + ", Player: " + playerScore);

            // Create trajectory line
            trajectoryLine = new Line();
            trajectoryLine.Stroke = new SolidColorBrush(Colors.Green);
            trajectoryLine.StrokeThickness = 3;
            mainCanvas.Children.Add(trajectoryLine);

            // Move slider to change brightness setting for camera
            slider.Value = 145;

            statusTextBlock.Text = "";

            lineList = new List<Line>();
            dotList = new List<Ellipse>();

            hideTextButton.Content = (textCanvas.Visibility == Visibility.Visible) ? "Hide Text" : "Show Text";
            textCanvas.Visibility = Visibility.Visible;
            centerOfMass.Visibility = Visibility.Collapsed;
            mallet.Visibility = Visibility.Collapsed;

            Line robotRangeLine = new Line();
            robotRangeLine.Stroke = new SolidColorBrush(Colors.Yellow);
            robotRangeLine.StrokeThickness = 3;
            robotRangeLine.Y1 = 0;
            robotRangeLine.Y2 = virtualHeight;
            robotRangeLine.X1 = robotRangeLine.X2 = Config.TABLE_MID_X_COORDINATE;
            mainCanvas.Children.Add(robotRangeLine);

            Rectangle rect = new Rectangle();
            rect.Stroke = new SolidColorBrush(Colors.Red);
            rect.StrokeThickness = 3;
            rect.Width = virtualWidth;
            rect.Height = virtualHeight;
            Canvas.SetLeft(rect, 0);
            Canvas.SetTop(rect, 0);
            mainCanvas.Children.Add(rect);

            Line midLine = new Line();
            midLine.Stroke = new SolidColorBrush(Colors.Red);
            midLine.StrokeThickness = 3;
            midLine.Y1 = 0;
            midLine.Y2 = virtualHeight;
            midLine.X1 = midLine.X2 = virtualWidth / 2;
            mainCanvas.Children.Add(midLine);

            Ellipse midCircle = new Ellipse();
            midCircle.Stroke = new SolidColorBrush(Colors.Red);
            midCircle.StrokeThickness = 3;
            midCircle.Width = midCircle.Height = 300;
            Canvas.SetLeft(midCircle, virtualWidth / 2 - midCircle.Width / 2);
            Canvas.SetTop(midCircle, virtualHeight / 2 - midCircle.Height / 2);
            mainCanvas.Children.Add(midCircle);
        }

        private void Robot_RobotGoalSensorTriggered(object sender, EventArgs e)
        {
            playerScore++;
            updateScores();
        }

        private void Robot_HumanGoalSensorTriggered(object sender, EventArgs e)
        {
            robotScore++;
            updateScores();
        }

        protected async override void OnNavigatedTo(NavigationEventArgs e)
        {
            gameMode = (GameMode)e.Parameter;

            mainCanvas.Visibility = gameCanvas.Visibility = Visibility.Collapsed;

            await pixyCam.Initialize();
            outputTextBlock.Text = "Initialized" + Environment.NewLine;

            switch (gameMode)
            {
                case GameMode.Test:
                    mainCanvas.Visibility = Visibility.Visible;
                    robot.StepperX.Debug = true;
                    robot.StepperY.Debug = true;
                    mainCanvas.PointerMoved += MainCanvas_PointerMoved;
                    startTestThread();
                    break;
                case GameMode.Diagnostics:
                    mainCanvas.Visibility = Visibility.Visible;
                    robot.StepperX.Debug = true;
                    robot.StepperY.Debug = true;
                    startProcessingThread();
                    break;
                case GameMode.Game:
                    gameCanvas.Visibility = Visibility.Visible;
                    scoreCanvas.Visibility = Visibility.Collapsed;
                    gameOutputTextBlock.Visibility = Visibility.Collapsed;
                    startProcessingThread();
                    break;
                case GameMode.Mirror:
                    startProcessingThread();
                    break;
            }
        }

        protected override void OnNavigatingFrom(NavigatingCancelEventArgs e)
        {
            base.OnNavigatingFrom(e);

            CloseAll();
        }

        private void CloseAll()
        {
            stopThread = true;

            if (robot != null)
            {
                robot.Close();
            }

            if (pixyCam != null)
            {
                pixyCam.Close();
            }
        }

        private void warmUp()
        {
            try
            {
                // Move robot to starting position at table corner
                showMessage("Resetting motors...");
                robot.MoveMotorsToZero();
            }
            catch (Exception) { }
        }

        private void runDecisionThread(Point puckPosition)
        {
            ThreadPool.RunAsync((s) =>
            {
                Point malletOffset;

                if (gameMode == GameMode.Mirror || mirrorMode)
                {
                    // Mirror the puck
                    double yOffset = MotorHelper.GetOffsetYFromCoordinateX(virtualWidth - puckPosition.X);
                    double xOffset = MotorHelper.GetOffsetXFromCoordinateY(puckPosition.Y);

                    if (Math.Abs(xOffset - robot.StepperX.CurrentPosition()) < 50)
                    {
                        xOffset = robot.StepperX.CurrentPosition();
                    }

                    if (Math.Abs(yOffset - robot.StepperY.CurrentPosition()) < 50)
                    {
                        yOffset = robot.StepperY.CurrentPosition();
                    }

                    malletOffset = new Point(xOffset, yOffset); //MotorHelper.GetOffsetFromCoordinates(puckPosition);
                }
                else
                {
                    // Figure out where the mallet should move
                    malletOffset = robot.AI.calculateMalletTarget(puckPosition, Global.Stopwatch.ElapsedMilliseconds);
                }

                if (malletOffset != CoordinateHelper.INVALID_POINT)
                {
                    // Set the destination for stepper motors

                    if (!robot.AI.DoNotInterrupt)
                    {
                        switch (robot.AI.Move)
                        {
                            case MoveType.Fast:
                                robot.MoveFastToOffset(malletOffset);
                                break;
                            case MoveType.Straight:
                                robot.MoveStraightToOffset(malletOffset);
                                break;
                        }
                    }
                }
            });
        }

        #region Test Methods

        // Use the mouse to simulate puck movement
        private void MainCanvas_PointerMoved(object sender, Windows.UI.Xaml.Input.PointerRoutedEventArgs e)
        {
            var puckPosition = e.GetCurrentPoint(mainCanvas).Position;
            runDecisionThread(puckPosition);
        }

        // Testing without an airhockey table
        private void startTestThread()
        {
            mainThread = ThreadPool.RunAsync((s) =>
            {
                while (!stopThread)
                {
                    // Run the motors
                    robot.StepperX.Run();
                    robot.StepperY.Run();

                    // If we're finished moving, go back to defense mode
                    if (robot.StepperX.DistanceToGo() == 0 && robot.StepperY.DistanceToGo() == 0)
                    {
                        robot.AI.DoNotInterrupt = false;
                        robot.AI.Mode = RobotMode.Defense;
                    }
                }
            }, WorkItemPriority.High);

            runUIThread();
        }

        #endregion

        // Draw the UI
        private void runUIThread()
        {
            ThreadPool.RunAsync(async (s) =>
            {
                while (!stopThread)
                {
                    var puckPosition = robot.AI.GetPuckPosition();

                    clearLines();
                    //drawDot(puckPosition);
                    if (puckPosition != CoordinateHelper.INVALID_POINT)
                    {
                        drawPuck(puckPosition);
                    }

                    drawMallet(robot.GetOffsets());
                    drawCenterOfMass(robot.AI.GetPuckCenterOfMass());

                    // Draw line from mallet to destination
                    drawLine(MotorHelper.GetCoordinatesFromOffset(robot.GetOffsets()),
                        MotorHelper.GetCoordinatesFromOffset(new Point(robot.StepperX.TargetPosition(), robot.StepperY.TargetPosition())));

                    var bouncePts = robot.AI.GetPreviousBouncePoints();

                    if (bouncePts != null && bouncePts.Count > 0)
                    {
                        // Draw lines for bounces
                        drawBounces(puckPosition, bouncePts);
                    }
                    else if (robot.AI.GetPreviousTrajectory() != CoordinateHelper.INVALID_POINT)
                    {
                        // Draw line for trajectory
                        drawTrajectory(puckPosition, robot.AI.GetPreviousTrajectory());
                    }

                    setOutputText("Speed: " + robot.AI.GetPuckSpeed());

                    await Task.Delay(1);
                }
            });
        }

        private void startProcessingThread()
        {
            // Run thread to update UI      
            if (gameMode == GameMode.Diagnostics)
            {
                runUIThread();
            }

            // Run thread to read camera and make decisions
            mainThread = ThreadPool.RunAsync((s) =>
            {
                if (gameMode != GameMode.Diagnostics)
                {
                    showMessage("Warming up...");
                    warmUp();
                    hideMessage();
                }

                long currentTimeMilliseconds = -1, previousTimeMilliseconds = -1;
                bool xMoved = false, yMoved = false;

                if (gameMode == GameMode.Game)
                {
                    showScoreCanvas();
                }
                else if (gameMode == GameMode.Diagnostics)
                {
                    setOutputText("Ready");
                }
                else if (gameMode == GameMode.Mirror)
                {
                    showMessage("Mirror Mode");
                }

                while (!stopThread)
                {
                    currentTimeMilliseconds = Global.Stopwatch.ElapsedMilliseconds;

                    // Check for new block every 20 ms, since the camera is 50hz
                    if ((currentTimeMilliseconds - previousTimeMilliseconds) > 20 || previousTimeMilliseconds == -1)
                    {
                        // Start new thread to poll camera so that we reduce the chance of getting jitter
                        ThreadPool.RunAsync((s1) =>
                        {
                            try
                            {
                                var blocks = pixyCam.GetBlocks(10);

                                if (blocks != null && blocks.Count > 0)
                                {
                                    foreach (ObjectBlock block in blocks)
                                    {
                                        var position = getPuckPosition(block);

                                        // Check if we got a valid puck position
                                        if (position != CoordinateHelper.INVALID_POINT)
                                        {
                                            runDecisionThread(position);
                                        }
                                    }
                                }
                                else
                                {
                                    if (robot.AI.GetPuckPosition() != null)
                                    {
                                        runDecisionThread(robot.AI.GetPuckPosition());
                                    }
                                }
                            }
                            catch (Exception err)
                            {
                                setOutputText("Error: " + err.Message);
                            }
                        }, WorkItemPriority.Low);

                        previousTimeMilliseconds = currentTimeMilliseconds;
                    }

                    // Run the motors
                    xMoved = robot.StepperX.Run();
                    yMoved = robot.StepperY.Run();

                    // If we're finished moving, go back to defense mode
                    if (robot.StepperX.DistanceToGo() == 0 && robot.StepperY.DistanceToGo() == 0)
                    {
                        robot.AI.DoNotInterrupt = false;
                        robot.AI.Mode = RobotMode.Defense;
                    }
                }
            }, WorkItemPriority.High);
        }

        private Point getPuckPosition(ObjectBlock block)
        {
            if (block != null)
            {
                if (block.Signature == 1 && block.Width >= 5 && block.Height >= 5)
                {
                    return CoordinateHelper.TranslatePoint(block.X, block.Y);
                }
            }

            return CoordinateHelper.INVALID_POINT;
        }

        #region UI Functions

        private void showMessage(string text)
        {
            Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Low, () =>
            {
                popUpTextBlock.Text = text;
                popUpCanvas.Visibility = Visibility.Visible;
            });
        }

        private void hideMessage()
        {
            Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Low, () =>
            {
                popUpTextBlock.Text = "";
                popUpCanvas.Visibility = Visibility.Collapsed;
            });
        }

        private void updateScores()
        {
            Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Low, () =>
            {
                if (gameMode == GameMode.Diagnostics)
                {
                    setScoreText("Robot: " + robotScore + ", Player: " + playerScore);
                }
                else if (gameMode == GameMode.Game)
                {
                    humanScoreTextBlock.Text = playerScore.ToString();
                    robotScoreTextBlock.Text = robotScore.ToString();
                }
            });
        }

        private void showScoreCanvas()
        {
            Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Low, () =>
            {
                gameOutputTextBlock.Visibility = Visibility.Collapsed;
                scoreCanvas.Visibility = Visibility.Visible;
            });
        }

        private void setScoreText(string text)
        {
            Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Low, () =>
            {
                scoreTextBlock.Text = text;
            });
        }

        private void drawCenterOfMass(Point point)
        {
            Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Low, () =>
            {
                centerOfMassTextBlock.Text = "Center of Mass: (" + point.X.ToString("0.##") + "," + point.Y.ToString("0.##") + ")";
                Canvas.SetLeft(centerOfMass, point.X - centerOfMass.Width / 2);
                Canvas.SetTop(centerOfMass, point.Y - centerOfMass.Height / 2);
                centerOfMass.Visibility = Visibility.Visible;
            });
        }

        private void drawTrajectory(Point src, Point dest)
        {
            Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Low, () =>
            {
                try
                {
                    if (dest != new Point(-1, -1))
                    {
                        trajectoryLine.X1 = src.X;
                        trajectoryLine.Y1 = src.Y;
                        trajectoryLine.X2 = dest.X;
                        trajectoryLine.Y2 = dest.Y;
                        trajectoryLine.Visibility = Visibility.Visible;
                    }
                    else
                    {
                        trajectoryLine.Visibility = Visibility.Collapsed;
                    }
                }
                catch (Exception)
                {

                }
            });
        }

        private void drawLine(Point p1, Point p2)
        {
            if (Double.IsNaN(p1.X) || Double.IsNaN(p2.X) || Double.IsNaN(p1.Y) || Double.IsNaN(p1.Y))
                return;

            Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Low, () =>
            {
                Line line = new Line();
                line.Stroke = new SolidColorBrush(Colors.Yellow);
                line.StrokeThickness = 3;
                line.X1 = p1.X;
                line.Y1 = p1.Y;
                line.X2 = p2.X;
                line.Y2 = p2.Y;
                mainCanvas.Children.Add(line);
                lineList.Add(line);
            });
        }

        private void drawBounces(Point src, List<Point> bouncePoints)
        {
            Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Low, () =>
            {
                drawLine(src, bouncePoints[0]);

                for (int i = 1; i < bouncePoints.Count; i++)
                {
                    drawLine(bouncePoints[i - 1], bouncePoints[i]);
                }
            });
        }

        private void clearLines()
        {
            Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Low, () =>
            {
                foreach (Line line in lineList)
                {
                    mainCanvas.Children.Remove(line);
                }

                lineList.Clear();

                trajectoryLine.Visibility = Visibility.Collapsed;
            });
        }

        private void drawPuck(Point point)
        {
            Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () =>
            {
                Canvas.SetLeft(puck, point.X - puck.Width / 2);
                Canvas.SetTop(puck, point.Y - puck.Height / 2);
                translatedTextBlock.Text = "Translated (X,Y): (" + point.X.ToString("0.##") + ", " + point.Y.ToString("0.##") + ")";
            });
        }

        private void drawMallet(Point offset)
        {
            Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () =>
            {
                // Draw mallet location
                var malletLoc = MotorHelper.GetCoordinatesFromOffset(offset);
                malletLocationTextBlock.Text = "Mallet (X,Y): (" + malletLoc.X + ", " + malletLoc.Y + ")";
                Canvas.SetLeft(mallet, malletLoc.X - mallet.Width / 2);
                Canvas.SetTop(mallet, malletLoc.Y - mallet.Height / 2);
                mallet.Visibility = Visibility.Visible;
            });
        }

        private void setOutputText(string text)
        {
            Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () =>
            {
                if (gameMode == GameMode.Diagnostics || gameMode == GameMode.Test)
                {
                    outputTextBlock.Text = text;
                }
                else if (gameMode == GameMode.Game)
                {
                    gameOutputTextBlock.Text = text;
                    gameOutputTextBlock.Visibility = Visibility.Visible;
                }
            });
        }

        #endregion

        #region UI Event Handlers

        private async void moveMalletButton_Click(object sender, RoutedEventArgs e)
        {
            stopThread = true;
            await Task.Delay(500);
            robot.StepperX.RunToNewPosition(0);
            robot.StepperY.RunToNewPosition(0);
        }

        private void homeButton_Click(object sender, RoutedEventArgs e)
        {
            showMessage("Returning to Main Menu...");
            stopThread = true;
            this.Frame.Navigate(typeof(MainPage), null);
        }

        private void mirrorButton_Click(object sender, RoutedEventArgs e)
        {
            mirrorMode = !mirrorMode;
            statusTextBlock.Text = "Mirror Mode: " + (mirrorMode ? "On" : "Off");
        }

        private void slider_ValueChanged(object sender, Windows.UI.Xaml.Controls.Primitives.RangeBaseValueChangedEventArgs e)
        {
            byte brightByte = Convert.ToByte(e.NewValue);
            pixyCam.SetBrightness(brightByte);
            brightnessTextBlock.Text = brightByte.ToString();
        }

        private void hideTextButton_Click(object sender, RoutedEventArgs e)
        {
            if (textCanvas.Visibility == Visibility.Visible)
            {
                textCanvas.Visibility = Visibility.Collapsed;
                hideTextButton.Content = "Show Text";
            }
            else
            {
                textCanvas.Visibility = Visibility.Visible;
                hideTextButton.Content = "Hide Text";
            }
        }

        #endregion

    }

}
