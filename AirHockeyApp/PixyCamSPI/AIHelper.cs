// Copyright (c) Microsoft. All rights reserved.

using System;
using System.Collections.Generic;
using System.Linq;
using Windows.Foundation;

namespace AirHockeyApp
{
    public enum RobotMode
    {
        Defense,
        Attack
    }

    public enum MoveType
    {
        Straight,   // Move straight to destination
        Fast    // Move to destination as fast as possible, using different speeds on each motor
    }

    public class PositionData
    {
        public PositionData(Point point, long time)
        {
            Point = point;
            Time = time;
        }

        public Point Point;
        public long Time;
    }

    public class AIHelper
    {
        public RobotMode Mode;
        public Queue<Point> CommandQueue = new Queue<Point>();

        private Robot robot;
        private Point previousTrajectoryPoint;
        private List<Point> previousBouncePoints = new List<Point>();
        private Point defenseOffset;
        private CenterOfMass com;

        private double speed;
        private Point puckPosition;

        private double alpha = 0, beta = 0;

        private List<PositionData> positionHistory = new List<PositionData>();
        private object positionHistoryLock = new object();

        // Flag that let's us know that we are in a move that can't be interrupted
        public bool DoNotInterrupt = false;

        // Type of movement
        public MoveType Move = MoveType.Straight;

        public AIHelper(Robot robotEntity)
        {
            robot = robotEntity;
            com = new CenterOfMass();
            previousTrajectoryPoint = CoordinateHelper.INVALID_POINT;
        }

        public Point GetPuckCenterOfMass()
        {
            return com.Position;
        }

        public Point GetPuckPosition()
        {
            return puckPosition;
        }

        public Point GetPreviousTrajectory()
        {
            return previousTrajectoryPoint;
        }

        public List<Point> GetPreviousBouncePoints()
        {
            return previousBouncePoints;
        }

        private bool addToHistory(Point position, long time)
        {
            bool result = false;
            lock (positionHistoryLock)
            {
                //We have more than one point in the history
                if (positionHistory.Count > 0)
                {
                    var last = positionHistory.Last();

                    // If the current point and the last point are far enough apart
                    if (Math.Abs(Helper.GetDistance(last.Point, position).H) > 30)
                    {
                        // If it's been at least 100 ms since the last point was seen
                        if (time - last.Time > 100)
                        {
                            positionHistory.Add(new PositionData(position, time));
                        }
                    }
                    // Previous and current point are close
                    else
                    {
                        // Add a new position, but use the previous position and the current time
                        positionHistory.Add(new PositionData(positionHistory.Last().Point, time));
                    }
                }
                // History is empty
                else
                {
                    positionHistory.Add(new PositionData(position, time));
                }

                // Limit history to last 20 points
                if (positionHistory.Count > 20)
                {
                    // Remove the first element
                    positionHistory.RemoveAt(0);
                }
            }

            return result;
        }

        public double GetPuckSpeed()
        {
            return speed;
        }

        private void calculateLine(Point[] points)
        {
            // Recalculate line
            double val1 = 0, val2 = 0, val3 = 0, val4 = 0;
            double n = points.Length;
            if (n != 0)
            {
                for (int i = 0; i < n; i++)
                {
                    val1 += points[i].X * points[i].Y;
                    val2 += points[i].X;
                    val3 += points[i].Y;
                    val4 += points[i].X * points[i].X;
                }

                beta = (val1 - (1 / n) * val2 * val3) / (val4 - (1 / n) * (val2 * val2));
                alpha = (1 / n) * val3 - beta * ((1 / n) * val3);
            }
        }

        //Get puck path based on point returned from this method
        public Point getPuckTrajectory(double x)
        {
            if (beta == 0)
            {
                return CoordinateHelper.INVALID_POINT;
            }

            double y = alpha + beta * x;
            if (y >= 0 && y <= CoordinateHelper.VirtualHeight)
            {
                return new Point(x, y);
            }
            else
            {
                y = Helper.Constrain(y, 0, CoordinateHelper.VirtualHeight);
                x = (y - alpha) / beta;
                return new Point(x, y);
            }
        }

        // Decision-making function; given puck position, decide where we should net move the mallet
        public Point calculateMalletTargetV3(Point currentPuckPosition, long currentTime)
        {
            puckPosition = currentPuckPosition;

            Move = MoveType.Straight;

            addToHistory(currentPuckPosition, currentTime);
            if (positionHistory.Count < 2)
            {
                return CoordinateHelper.INVALID_POINT;
            }

            // Set the default defense position
            defenseOffset = new Point(Config.MAX_MALLET_OFFSET_X / 2, 0);

            // Calculate center of mass for trajectory calculations
            // Get the previous center of mass position
            Point prevCenterOfMass = com.Position;
            // Add the new puck position to the queue for calculations
            com.SafeEnqueue(currentPuckPosition);
            // Calculate the new center of mass
            Point centerOfMass = com.Calculate();

            calculateLine(com.PointQueue.ToArray());

            // If we don't have the previous center of mass, we can't do trajectory calculations, so skip this
            if (prevCenterOfMass == CoordinateHelper.INVALID_POINT)
            {
                return CoordinateHelper.INVALID_POINT;
            }

            // For speed calculations
            var prevPosition = positionHistory[positionHistory.Count - 2];
            var currPosition = positionHistory.Last();

            // Convert everything to positions (coordinates in the coordinate plane) or offsets (steps for motors)
            Point malletTargetOffset = CoordinateHelper.INVALID_POINT;
            // Mallet offset
            Point currentMalletOffset = new Point(robot.StepperX.CurrentPosition(), robot.StepperY.CurrentPosition());
            // Mallet target in steps
            Point currentMalletTargetOffset = new Point(robot.StepperX.TargetPosition(), robot.StepperY.TargetPosition());
            // Mallet position coordinates
            Point currentMalletPosition = MotorHelper.GetCoordinatesFromOffset(currentMalletOffset);
            // Mallet target coordinates
            Point currentMalletTargetPosition = MotorHelper.GetCoordinatesFromOffset(currentMalletTargetOffset);
            // Distance from previous point
            Vector distanceFromPrevious = Helper.GetDistance(prevCenterOfMass, centerOfMass);
            speed = distanceFromPrevious.H;
            // Puck distance from mallet (coordinates)
            Vector distanceFromMallet = Helper.GetDistance(currentPuckPosition, currentMalletPosition);

            Point trajectoryPoint = CoordinateHelper.INVALID_POINT, bouncePoint = CoordinateHelper.INVALID_POINT;
            // List of points where we think the puck will bounce
            List<Point> bouncePoints = new List<Point>();

            // If this flag has been set, it means we're in the middle of a non-interruptable move
            if (DoNotInterrupt)
            {
                return CoordinateHelper.INVALID_POINT;
            }

            // The mallet and puck have probably collided, don't do anything yet
            if (Helper.GetDistance(currentMalletPosition, currentPuckPosition).H < 40)
            {
                return CoordinateHelper.INVALID_POINT;
            }

            // Puck is behind the mallet
            if (distanceFromMallet.X < 0)
            {
                // Defensive position in front of goal
                malletTargetOffset = defenseOffset;

                // Don't interrupt this move until we get to the destination
                DoNotInterrupt = true;

            }
            // Puck is moving away
            else if (prevCenterOfMass.X > centerOfMass.X)
            {
                // Defensive position in front of goal
                malletTargetOffset = defenseOffset;

            }
            else if (distanceFromPrevious.H < 5 && currentPuckPosition.X > Config.TABLE_MID_X_COORDINATE && currentMalletOffset == defenseOffset)
            {
                if (distanceFromMallet.H < 400)
                {
                    // Get new mallet position
                    var targetPoint = CoordinateHelper.CalculateTrajectoryPoint(currentMalletPosition, currentPuckPosition, currentPuckPosition.X - 100);
                    malletTargetOffset = MotorHelper.GetOffsetFromCoordinates(targetPoint);

                    DoNotInterrupt = true;
                }
            }
            // Puck is moving towards mallet
            else if (prevCenterOfMass.X < centerOfMass.X)
            {
                trajectoryPoint = CoordinateHelper.CalculateTrajectoryPoint(prevCenterOfMass, centerOfMass, currentMalletPosition.X);

                if (trajectoryPoint == CoordinateHelper.INVALID_POINT)
                {
                    malletTargetOffset = CoordinateHelper.INVALID_POINT;
                }
                // We can block the puck from our current location (no Y movement)
                else if (trajectoryPoint.X == currentMalletPosition.X)
                {
                    // We're already at the point
                    if (Math.Abs(trajectoryPoint.Y - currentMalletPosition.Y) < 50)// && currentPuckPosition.X > Config.TABLE_MID_X_COORDINATE)//Helper.GetDistance(currentPuckPosition, currentMalletPosition).H < 400)
                    {
                        var targetPoint = CoordinateHelper.CalculateTrajectoryPoint(currentMalletPosition, currentPuckPosition, currentPuckPosition.X - 100);
                        malletTargetOffset = MotorHelper.GetOffsetFromCoordinates(targetPoint);

                        DoNotInterrupt = true;
                    }
                    else
                    {
                        malletTargetOffset = new Point(MotorHelper.GetOffsetXFromCoordinateY(trajectoryPoint.Y), currentMalletOffset.Y);
                    }
                }
                else if (trajectoryPoint.X != currentMalletPosition.X)
                {
                    if (speed > 15)
                    {
                        double angleAdjust = 1;
                        double adjustment = .1;

                        if (trajectoryPoint.X != currentMalletPosition.X)
                        {
                            bouncePoints.Add(trajectoryPoint);
                            bouncePoint = CoordinateHelper.CalculateBouncePoint(centerOfMass, trajectoryPoint, currentMalletPosition.X, angleAdjust += adjustment);
                            Point prevBouncePoint = trajectoryPoint;
                            while (bouncePoint.X != currentMalletPosition.X && bouncePoints.Count < 1)
                            {
                                bouncePoints.Add(bouncePoint);
                                bouncePoint = CoordinateHelper.CalculateBouncePoint(prevBouncePoint, bouncePoint, currentMalletPosition.X, angleAdjust += adjustment);
                                prevBouncePoint = bouncePoint;
                            }

                            bouncePoints.Add(bouncePoint);
                        }

                        if (bouncePoint.X == currentMalletPosition.X)
                        {
                            // Don't react to bounce unless we think it's going to be near the goal
                            if (bouncePoint.Y > Config.TABLE_GOAL_Y_TOP && bouncePoint.X < Config.TABLE_GOAL_Y_BOTTOM)
                            {
                                malletTargetOffset = MotorHelper.GetOffsetFromCoordinates(bouncePoint);
                            }
                        }
                    }
                }
                else
                {
                    // Defensive position in front of goal
                    malletTargetOffset = defenseOffset;
                }
            }

            // We have a valid target destination for the mallet
            if (malletTargetOffset != CoordinateHelper.INVALID_POINT)
            {
                // Constrain offset so that mallet doesn't move too far
                malletTargetOffset.X = Helper.Constrain(malletTargetOffset.X, 0, Config.MAX_MALLET_OFFSET_X);
                malletTargetOffset.Y = Helper.Constrain(malletTargetOffset.Y, 0, Config.MAX_MALLET_OFFSET_Y);
            }

            previousTrajectoryPoint = trajectoryPoint;
            previousBouncePoints = bouncePoints;

            return malletTargetOffset;
        }

        // Handles calculation of center of mass of group of points
        public class CenterOfMass
        {
            public Queue<Point> PointQueue = new Queue<Point>();
            public Point Position = new Point();
            private object queueLock = new object();

            public void SafeEnqueue(Point point)
            {
                lock (queueLock)
                {
                    PointQueue.Enqueue(point);
                }
            }

            public Point Calculate()
            {
                lock (queueLock)
                {
                    if (PointQueue.Count < 1)
                    {
                        return CoordinateHelper.INVALID_POINT;
                    }
                    // Store up to 10 points at a time in the queue
                    else if (PointQueue.Count > 10)
                    {
                        PointQueue.Dequeue();
                    }

                    double xSum = 0, ySum = 0;
                    foreach (Point p in PointQueue)
                    {
                        xSum += p.X;
                        ySum += p.Y;
                    }

                    // New position calculated should be at least 5 points from the previous position
                    var newPosition = new Point(xSum / PointQueue.Count, ySum / PointQueue.Count);
                    if (Math.Abs(Helper.GetDistance(newPosition, Position).H) > 5)
                    {
                        Position = newPosition;
                    }

                    return Position;
                }
            }
        }
    }
}
