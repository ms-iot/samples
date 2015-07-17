// Copyright (c) Microsoft. All rights reserved.

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
