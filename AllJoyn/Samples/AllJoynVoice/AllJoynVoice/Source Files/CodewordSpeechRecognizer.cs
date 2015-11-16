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

using Windows.Media.SpeechRecognition;
using System.Collections.Generic;
using System;
using System.Threading.Tasks;
using System.Linq;

namespace AllJoynVoice
{
    enum CodewordSpeechRecognizerState
    {
        Idle,
        Listening,
    };

    /// <summary>
    /// Recognizes speech continuously. Whenever a string is matched, the WordRecognized
    /// event is raised. The Codeword must precede the sought phrase to be matched.
    /// </summary>
    class CodewordSpeechRecognizer
    {
        public delegate void WordRecognizedHandler(string tag);
        public delegate void StateChangedHandler(CodewordSpeechRecognizerState tag);

        public event WordRecognizedHandler WordRecognized;
        public event StateChangedHandler StateChanged;

        private CodewordSpeechRecognizerState _state;
        public CodewordSpeechRecognizerState State
        {
            get { return _state; }

            private set {
                _state = value;
                if (StateChanged != null)
                    StateChanged(_state);
            }
        }

        private SpeechRecognizer SpeechRecognizer;

        public CodewordSpeechRecognizer(string Codeword, IReadOnlyList<AllJoynAction> Actions)
        {
            var en_us = new Windows.Globalization.Language("en-US");

            SpeechRecognizer = new SpeechRecognizer(en_us);
            Logger.LogInfo("Recognizing in " + SpeechRecognizer.CurrentLanguage.NativeName);

            // Prepend the codeword to all commands.
            foreach (AllJoynAction action in Actions)
            {
                SpeechRecognizer.Constraints.Add(
                    new SpeechRecognitionListConstraint(
                        action.Commands.Select(
                            x => Codeword + ", " + x
                        ),
                        action.Id
                    )
                );
            }

            SpeechRecognizer.ContinuousRecognitionSession.AutoStopSilenceTimeout = new TimeSpan(1, 0, 0);
            Logger.LogInfo("Timeout: " + SpeechRecognizer.ContinuousRecognitionSession.AutoStopSilenceTimeout);

            SpeechRecognizer.ContinuousRecognitionSession.ResultGenerated += ContinuousRecognitionSession_ResultGenerated;
            SpeechRecognizer.ContinuousRecognitionSession.Completed += ContinuousRecognitionSession_Completed;
        }

        public async Task StartAsync()
        {
            if (State != CodewordSpeechRecognizerState.Idle)
            {
                throw new InvalidOperationException();
            }

            Logger.LogInfo("Starting speech recognizer.");
            State = CodewordSpeechRecognizerState.Listening;
            await SpeechRecognizer.ContinuousRecognitionSession.StartAsync();
        }

        public async Task StopAsync()
        {
            if (State != CodewordSpeechRecognizerState.Listening)
            {
                throw new InvalidOperationException();
            }

            Logger.LogInfo("Stopping speech recognizer.");
            State = CodewordSpeechRecognizerState.Idle;
            await SpeechRecognizer.ContinuousRecognitionSession.StopAsync();
        }

        public async Task CompileConstraintsAsync()
        {
            SpeechRecognitionResultStatus status = (await SpeechRecognizer.CompileConstraintsAsync()).Status;

            if (status != SpeechRecognitionResultStatus.Success)
            {
                throw Logger.LogException(this.GetType().Name, new InvalidOperationException("Couldn't compile the speech recognition constraints."));
            }
        }

        private async void ContinuousRecognitionSession_Completed(SpeechContinuousRecognitionSession sender, SpeechContinuousRecognitionCompletedEventArgs args)
        {
            bool shouldRestart = State == CodewordSpeechRecognizerState.Listening;

            State = CodewordSpeechRecognizerState.Idle;

            if (shouldRestart)
            {
                Logger.LogInfo("Timed out...");
                await StartAsync();
            }
        }

        private void ContinuousRecognitionSession_ResultGenerated(SpeechContinuousRecognitionSession sender, SpeechContinuousRecognitionResultGeneratedEventArgs args)
        {
            if (State == CodewordSpeechRecognizerState.Listening && args.Result.Constraint != null && WordRecognized != null)
            {
                Logger.LogInfo(string.Format("Recognized tag: \"{0}\"", args.Result.Constraint.Tag));
                WordRecognized(args.Result.Constraint.Tag);
            }
        }
    }
}