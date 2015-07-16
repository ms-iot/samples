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
**/

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading.Tasks;
using Windows.Storage;

namespace AirHockeyApp
{
    public class MusicPlayer
    {
        private static bool stop = false;

        public static void Stop()
        {
            stop = true;
        }

        public static async Task<Note[]> ReadNotesFromFile(string filePath)
        {
            try
            {
                StorageFolder InstallationFolder = Windows.ApplicationModel.Package.Current.InstalledLocation;
                var file = await InstallationFolder.GetFileAsync(filePath);
                var read = await FileIO.ReadLinesAsync(file);

                List<Note> notes = new List<Note>();
                foreach (string str in read)
                {
                    var temp = str.Split(',');
                    if (temp.Length == 2)
                    {
                        double pitch = Pitch.GetFrequency(temp[0]);
                        double duration = Convert.ToDouble(temp[1]);
                        // Store notes read in list
                        notes.Add(new Note { Name = temp[0], Pitch = pitch, Length = duration });
                    }
                }

                return notes.ToArray();

            }
            catch (Exception)
            {
                return null;
            }
        }

        public static void PlaySong(int bpm, SoundPlayer[] players)
        {
            stop = false;

            Stopwatch stopwatch = new Stopwatch();

            // Calculate number of ticks per beat
            double n = 60.0 / bpm * TimeSpan.TicksPerSecond;

            stopwatch.Start();
            long prevElapsedTicks = 0, elapsedTicks = 0;

            while (!stop)
            {
                int finished = 0;

                elapsedTicks = stopwatch.ElapsedTicks - prevElapsedTicks;
                prevElapsedTicks = stopwatch.ElapsedTicks;

                foreach (SoundPlayer player in players)
                {
                    player.BeatsRemaining -= elapsedTicks / n;

                    // Switch note
                    if (player.BeatsRemaining <= 0)
                    {
                        if (!player.GoToNextNote())
                        {
                            finished++;
                        }
                    }
                    else if (player.BeatsRemaining >= 0.03125)
                    {
                        player.Play();
                    }
                }

                // Break out of loop once all the players are finished playing
                if (finished == players.Length)
                {
                    break;
                }
            }
        }
    }
}
