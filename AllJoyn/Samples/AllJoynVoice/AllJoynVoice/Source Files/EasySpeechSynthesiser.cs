// Copyright (c) 2015, Microsoft Corporation
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
// SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
// IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//

using System;
using System.Threading.Tasks;

using Windows.Media.SpeechSynthesis;
using Windows.UI.Core;
using Windows.UI.Xaml.Controls;
using System.Collections.Concurrent;
using System.Threading;
using Windows.UI.Xaml;
using Windows.Foundation;

namespace AllJoynVoice
{
    /// <summary>
    /// Handles speech synthesis and output. Multiple messages to be
    /// spoken can be enqueued asynchronously and will be spoken one by one.
    /// </summary>
    public class EasySpeechSynthesizer
    {
        public delegate void SpeechEvent();

        public SpeechEvent StartedSpeaking;
        public SpeechEvent StoppedSpeaking;

        private object speakingStateLock = new object();
        private bool speakingState;
        private bool isSpeaking
        {
            get { return speakingState; }
            set
            {
                lock (speakingStateLock)
                {
                    if (value != speakingState)
                    {
                        if (value && StartedSpeaking != null)
                        {
                            StartedSpeaking();
                        }
                        else if (!value && StoppedSpeaking != null)
                        {
                            StoppedSpeaking();
                        }
                    }

                    speakingState = value;
                }
            }
        }

        private SpeechSynthesizer synthesizer;
        /// <summary>
        /// A MediaElement in the main page that will output the synthesized speech.
        /// </summary>
        private MediaElement mediaOutput;
        /// <summary>
        /// A dipatcher for the UI thread,
        /// since interacting with MediaElements needs to be done from there.
        /// </summary>
        private CoreDispatcher uiDispatcher;

        /// <summary>The audio streams to be spoken.</summary>
        private BlockingCollection<SpeechSynthesisStream> toSpeak;
        /// <summary>Cancelling this stops processing the queue.</summary>
        private CancellationTokenSource speechCancellationSource;

        public EasySpeechSynthesizer(CoreDispatcher uiDispatcher, MediaElement mediaOutput)
        {
            synthesizer = new SpeechSynthesizer();
            this.uiDispatcher = uiDispatcher;
            this.mediaOutput = mediaOutput;

            toSpeak = new BlockingCollection<SpeechSynthesisStream>(new ConcurrentQueue<SpeechSynthesisStream>());
        }

        /// <summary>
        /// Start processing the speech queue.
        /// </summary>
        public void StartSpeaking()
        {
            // Already speaking.
            if (speechCancellationSource?.IsCancellationRequested == false)
            {
                throw Logger.LogException(this.GetType().Name, new InvalidOperationException("StartSpeaking() was already called."));
            }

            speechCancellationSource = new CancellationTokenSource();
            CancellationToken speechCancellationToken = speechCancellationSource.Token;

            Task.Run(async () =>
            {
                // Blocks until speech has finished being output.
                AutoResetEvent mediaEnd = new AutoResetEvent(false);

                // Subscribing to the MediaEnded event has to be done on the UI thread...
                bool handlerReady = false;

                // Resets the mediaEnd when speech output is finished.
                RoutedEventHandler handler = (s, e) => mediaEnd.Set();

                try
                {
                    // The only way to stop iterating this BlockingCollection is via an exception.
                    foreach (SpeechSynthesisStream stream in toSpeak.GetConsumingEnumerable(speechCancellationToken))
                    {
                        Logger.LogInfo("Speaking next message in the queue.");

                        await uiDispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
                        {
                            if (!handlerReady)
                            {
                                mediaOutput.MediaEnded += handler;
                                handlerReady = true;
                            }

                            try
                            {
                                mediaOutput.SetSource(stream, stream.ContentType);
                            }
                            catch (Exception ex)
                            {
                                Logger.LogException(this.GetType().Name, ex);
                            }

                            mediaOutput.AutoPlay = true;

                            isSpeaking = true;

                            mediaOutput.Play();
                        });

                        // Blocks until the media is finished playing and the MediaEnded handler is called.
                        mediaEnd.WaitOne();

                        Logger.LogInfo("Done speaking.");

                        isSpeaking = false;
                    }
                }
                catch (OperationCanceledException)
                {
                    await uiDispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
                    {
                        if (handlerReady)
                        {
                            mediaOutput.MediaEnded -= handler;
                        }
                    });
                }
            }
            );
        }

        /// <summary>
        /// Adds a message to the speech queue.
        /// </summary>
        /// <param name="message">The message to be spoken.</param>
        /// <returns></returns>
        public async Task Speak(string message)
        {
            Logger.LogInfo(string.Format("Adding \"{0}\" to the speech queue.", message));

            SpeechSynthesisStream stream = await synthesizer.SynthesizeTextToStreamAsync(message);

            if (stream == null)
            {
                Logger.LogError("Unable to synthesize speech.");
                return;
            }

            toSpeak.Add(stream);
        }

        /// <summary>
        /// Stops speaking, clears out the speech queue and stops processing the speech queue.
        /// Messages sent before this method returns will not be enqueued.
        /// </summary>
        public async Task StopSpeaking()
        {
            if (speechCancellationSource?.IsCancellationRequested != false)
            {
                throw Logger.LogException(this.GetType().Name, new InvalidOperationException("StartSpeaking() must be called before StopSpeaking()."));
            }

            speechCancellationSource.Cancel();

            IAsyncAction stop = uiDispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                mediaOutput.Stop();
                isSpeaking = false;
            });

            // Clear out the queue.
            while (toSpeak.Count > 0)
            {
                SpeechSynthesisStream item;
                try
                {
                    toSpeak.TryTake(out item);
                }
                catch (InvalidOperationException) { }
            }

            await stop;
        }

        /// <summary>
        /// Stops speaking and clears out the speech queue.
        /// </summary>
        public async Task ClearQueue()
        {
            await StopSpeaking();
            StartSpeaking();
        }
    }
}