// Copyright (c) Microsoft. All rights reserved.

using System.Collections.Generic;
using System.Diagnostics;
using AirHockeyHelper2;

namespace AirHockeyApp
{
    public class Pitch
    {
        static Dictionary<string, double> pitchDictionary;
        public static double Rest = 0;
        public static double GetFrequency(string pitch)
        {
            if (pitchDictionary == null)
            {
                pitchDictionary = new Dictionary<string, double>();
                pitchDictionary.Add("C0", 16.35);
                pitchDictionary.Add("CSharp0", 17.32);
                pitchDictionary.Add("Db0", 17.32);
                pitchDictionary.Add("D0", 18.35);
                pitchDictionary.Add("DSharp0", 19.45);
                pitchDictionary.Add("Eb0", 19.45);
                pitchDictionary.Add("E0", 20.6);
                pitchDictionary.Add("F0", 21.83);
                pitchDictionary.Add("FSharp0", 23.12);
                pitchDictionary.Add("Gb0", 23.12);
                pitchDictionary.Add("G0", 24.5);
                pitchDictionary.Add("GSharp0", 25.96);
                pitchDictionary.Add("Ab0", 25.96);
                pitchDictionary.Add("A0", 27.5);
                pitchDictionary.Add("ASharp0", 29.14);
                pitchDictionary.Add("Bb0", 29.14);
                pitchDictionary.Add("B0", 30.87);
                pitchDictionary.Add("C1", 32.7);
                pitchDictionary.Add("CSharp1", 34.65);
                pitchDictionary.Add("Db1", 34.65);
                pitchDictionary.Add("D1", 36.71);
                pitchDictionary.Add("DSharp1", 38.89);
                pitchDictionary.Add("Eb1", 38.89);
                pitchDictionary.Add("E1", 41.2);
                pitchDictionary.Add("F1", 43.65);
                pitchDictionary.Add("FSharp1", 46.25);
                pitchDictionary.Add("Gb1", 46.25);
                pitchDictionary.Add("G1", 49);
                pitchDictionary.Add("GSharp1", 51.91);
                pitchDictionary.Add("Ab1", 51.91);
                pitchDictionary.Add("A1", 55);
                pitchDictionary.Add("ASharp1", 58.27);
                pitchDictionary.Add("Bb1", 58.27);
                pitchDictionary.Add("B1", 61.74);
                pitchDictionary.Add("C2", 65.41);
                pitchDictionary.Add("CSharp2", 69.3);
                pitchDictionary.Add("Db2", 69.3);
                pitchDictionary.Add("D2", 73.42);
                pitchDictionary.Add("DSharp2", 77.78);
                pitchDictionary.Add("Eb2", 77.78);
                pitchDictionary.Add("E2", 82.41);
                pitchDictionary.Add("F2", 87.31);
                pitchDictionary.Add("FSharp2", 92.5);
                pitchDictionary.Add("Gb2", 92.5);
                pitchDictionary.Add("G2", 98);
                pitchDictionary.Add("GSharp2", 103.83);
                pitchDictionary.Add("Ab2", 103.83);
                pitchDictionary.Add("A2", 110);
                pitchDictionary.Add("ASharp2", 116.54);
                pitchDictionary.Add("Bb2", 116.54);
                pitchDictionary.Add("B2", 123.47);
                pitchDictionary.Add("C3", 130.81);
                pitchDictionary.Add("CSharp3", 138.59);
                pitchDictionary.Add("Db3", 138.59);
                pitchDictionary.Add("D3", 146.83);
                pitchDictionary.Add("DSharp3", 155.56);
                pitchDictionary.Add("Eb3", 155.56);
                pitchDictionary.Add("E3", 164.81);
                pitchDictionary.Add("F3", 174.61);
                pitchDictionary.Add("FSharp3", 185);
                pitchDictionary.Add("Gb3", 185);
                pitchDictionary.Add("G3", 196);
                pitchDictionary.Add("GSharp3", 207.65);
                pitchDictionary.Add("Ab3", 207.65);
                pitchDictionary.Add("A3", 220);
                pitchDictionary.Add("ASharp3", 233.08);
                pitchDictionary.Add("Bb3", 233.08);
                pitchDictionary.Add("B3", 246.94);
                pitchDictionary.Add("C4", 261.63);
                pitchDictionary.Add("CSharp4", 277.18);
                pitchDictionary.Add("Db4", 277.18);
                pitchDictionary.Add("D4", 293.66);
                pitchDictionary.Add("DSharp4", 311.13);
                pitchDictionary.Add("Eb4", 311.13);
                pitchDictionary.Add("E4", 329.63);
                pitchDictionary.Add("F4", 349.23);
                pitchDictionary.Add("FSharp4", 369.99);
                pitchDictionary.Add("Gb4", 369.99);
                pitchDictionary.Add("G4", 392);
                pitchDictionary.Add("GSharp4", 415.3);
                pitchDictionary.Add("Ab4", 415.3);
                pitchDictionary.Add("A4", 440);
                pitchDictionary.Add("ASharp4", 466.16);
                pitchDictionary.Add("Bb4", 466.16);
                pitchDictionary.Add("B4", 493.88);
                pitchDictionary.Add("C5", 523.25);
                pitchDictionary.Add("CSharp5", 554.37);
                pitchDictionary.Add("Db5", 554.37);
                pitchDictionary.Add("D5", 587.33);
                pitchDictionary.Add("DSharp5", 622.25);
                pitchDictionary.Add("Eb5", 622.25);
                pitchDictionary.Add("E5", 659.25);
                pitchDictionary.Add("F5", 698.46);
                pitchDictionary.Add("FSharp5", 739.99);
                pitchDictionary.Add("Gb5", 739.99);
                pitchDictionary.Add("G5", 783.99);
                pitchDictionary.Add("GSharp5", 830.61);
                pitchDictionary.Add("Ab5", 830.61);
                pitchDictionary.Add("A5", 880);
                pitchDictionary.Add("ASharp5", 932.33);
                pitchDictionary.Add("Bb5", 932.33);
                pitchDictionary.Add("B5", 987.77);
                pitchDictionary.Add("C6", 1046.5);
                pitchDictionary.Add("CSharp6", 1108.73);
                pitchDictionary.Add("Db6", 1108.73);
                pitchDictionary.Add("D6", 1174.66);
                pitchDictionary.Add("DSharp6", 1244.51);
                pitchDictionary.Add("Eb6", 1244.51);
                pitchDictionary.Add("E6", 1318.51);
                pitchDictionary.Add("F6", 1396.91);
                pitchDictionary.Add("FSharp6", 1479.98);
                pitchDictionary.Add("Gb6", 1479.98);
                pitchDictionary.Add("G6", 1567.98);
                pitchDictionary.Add("GSharp6", 1661.22);
                pitchDictionary.Add("Ab6", 1661.22);
                pitchDictionary.Add("A6", 1760);
                pitchDictionary.Add("ASharp6", 1864.66);
                pitchDictionary.Add("Bb6", 1864.66);
                pitchDictionary.Add("B6", 1975.53);
                pitchDictionary.Add("C7", 2093);
                pitchDictionary.Add("CSharp7", 2217.46);
                pitchDictionary.Add("Db7", 2217.46);
                pitchDictionary.Add("D7", 2349.32);
                pitchDictionary.Add("DSharp7", 2489.02);
                pitchDictionary.Add("Eb7", 2489.02);
                pitchDictionary.Add("E7", 2637.02);
                pitchDictionary.Add("F7", 2793.83);
                pitchDictionary.Add("FSharp7", 2959.96);
                pitchDictionary.Add("Gb7", 2959.96);
                pitchDictionary.Add("G7", 3135.96);
                pitchDictionary.Add("GSharp7", 3322.44);
                pitchDictionary.Add("Ab7", 3322.44);
                pitchDictionary.Add("A7", 3520);
                pitchDictionary.Add("ASharp7", 3729.31);
                pitchDictionary.Add("Bb7", 3729.31);
                pitchDictionary.Add("B7", 3951.07);
                pitchDictionary.Add("C8", 4186.01);
                pitchDictionary.Add("CSharp8", 4434.92);
                pitchDictionary.Add("Db8", 4434.92);
                pitchDictionary.Add("D8", 4698.63);
                pitchDictionary.Add("DSharp8", 4978.03);
                pitchDictionary.Add("Eb8", 4978.03);
                pitchDictionary.Add("E8", 5274.04);
                pitchDictionary.Add("F8", 5587.65);
                pitchDictionary.Add("FSharp8", 5919.91);
                pitchDictionary.Add("Gb8", 5919.91);
                pitchDictionary.Add("G8", 6271.93);
                pitchDictionary.Add("GSharp8", 6644.88);
                pitchDictionary.Add("Ab8", 6644.88);
                pitchDictionary.Add("A8", 7040);
                pitchDictionary.Add("ASharp8", 7458.62);
                pitchDictionary.Add("Bb8", 7458.62);
                pitchDictionary.Add("B8", 7902.13);
                pitchDictionary.Add("Rest", 0);
            }

            return pitchDictionary[pitch];
        }
    }

    public struct Note
    {
        public string Name;
        public double Pitch;
        public double Length;
    }

    public class SoundPlayer
    {
        private AccelStepper stepper;
        private long position, maxPosition;

        public Note[] Notes;
        public int CurrentNote = 0;
        public double BeatsRemaining = 0;
        public int PitchCorrect = 20;

        public SoundPlayer(AccelStepper stepperEntity, long maxPos)
        {
            stepper = stepperEntity;
            maxPosition = maxPos;
            position = stepper.CurrentPosition();
        }

        public void SetSong(Note[] notes)
        {
            Notes = notes;
            SetNote(notes[0]);
        }

        public void Play()
        {
            // If we're not on a rest and we're at one of the ends, change direction
            if (stepper.Speed() != 0 && stepper.DistanceToGo() == 0)
            {
                // Change direction
                stepper.SetSpeed(-stepper.Speed());

                // Set position to other end
                if (stepper.TargetPosition() == maxPosition)
                {
                    stepper.MoveTo(0);
                }
                else
                {
                    stepper.MoveTo(maxPosition);
                }
            }

            stepper.RunSpeed();
        }

        public void SetNote(Note note)
        {
            long midPosition = maxPosition / 2;
            int speed = (int)note.Pitch * PitchCorrect;
            long currentPos = stepper.CurrentPosition();
            long target = currentPos;

            // Figure out which direction motor should move
            if (currentPos < midPosition)
            {
                target = maxPosition;
            }
            else
            {
                target = 0;
                speed = -speed;
            }

            stepper.MoveTo(target);
            stepper.SetSpeed(speed);  // multiply by 3 to increase octave

            BeatsRemaining = note.Length;
        }

        public bool GoToNextNote()
        {
            CurrentNote++;
            if (CurrentNote < Notes.Length)
            {
                SetNote(Notes[CurrentNote]);
                return true;
            }

            stepper.SetSpeed(0);
            return false;
        }

        public void PlayNote(Note note)
        {
            note.Length = note.Length * 0.02;
            float speed = (float)note.Pitch;

            if (note.Pitch != Pitch.Rest)
            {
                long steps = (long)((int)note.Pitch * note.Length);
                if (position + steps < maxPosition)
                {
                    position += steps;
                }
                else
                {
                    position -= steps;
                    speed = -speed;
                }

                stepper.RunSpeedToNewPosition(position, speed);
            }
            else
            {
                var start = Global.Stopwatch.ElapsedMilliseconds;
                while (Global.Stopwatch.ElapsedMilliseconds - start < note.Length * 1000) ;
            }
        }
    }
}
