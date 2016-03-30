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

using MyLivingRoom.Cortana;
using MyLivingRoom.Events;
using MyLivingRoom.Extensions;
using MyLivingRoom.ViewModels;
using MyLivingRoom.Views;
using Prism.Events;
using Prism.Windows;
using Prism.Windows.Navigation;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Windows.ApplicationModel;
using Windows.ApplicationModel.Activation;
using Windows.Media.SpeechRecognition;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Data;

namespace MyLivingRoom
{
    sealed partial class App : PrismApplication
    {
        public App()
        {
            this.InitializeComponent();
        }

        public static new App Current
        {
            get { return (App)Application.Current; }
        }

        public EventAggregator EventAggregator { get; } = new EventAggregator();

        public new INavigationService NavigationService
        {
            get { return base.NavigationService; }
        }

        public TopicGroupDefinitionCollection TopicGroupDefinitions
        {
            get { return (TopicGroupDefinitionCollection)((CollectionViewSource)this.Resources["TopicGroupDefinitions"]).Source; }
        }

        public object GetViewModelFromTopicId(string topicId)
        {
            return this.TopicGroupDefinitions
                .SelectMany(tgd => tgd.TopicDefinitions)
                .Where(tvm => tvm.Id.Equals(topicId, StringComparison.Ordinal))
                .FirstOrDefault()?.TopicViewModel;
        }

        protected override Frame OnCreateRootFrame()
        {
            Frame frame = null;
            if (_shell == null)
            {
                _shell = new ShellView(out frame);
            }

            return frame;
        }

        protected override UIElement CreateShell(Frame rootFrame)
        {
            _shell.DataContext = new ShellViewModel();
            return _shell;
        }
        private ShellView _shell;

        protected override Task OnLaunchApplicationAsync(LaunchActivatedEventArgs args)
        {
            var deviceDisplayNames = this.TopicGroupDefinitions
                .SelectMany(g => g.TopicDefinitions)
                .Where(t => t.TopicViewModel is BaseDeviceViewModel)
                .Select(v => v.DisplayName);

            VoiceCommandService.InstallVoiceCommandService(deviceDisplayNames);

            this.NavigationService.ClearHistory();
            this.NavigationService.Navigate(typeof(HomePageView), null);
            return Task.CompletedTask;
        }

        protected override Task OnActivateApplicationAsync(IActivatedEventArgs args)
        {
            if (args.Kind == ActivationKind.Protocol)
            {
                var protocolArgs = args as ProtocolActivatedEventArgs;
                if (protocolArgs != null)
                {
                    var unused = _shell.Dispatcher.RunAsync(CoreDispatcherPriority.Normal,
                        () => this.OnActivateApplicationForProtocol(protocolArgs));
                }
            }
            else if (args.Kind == ActivationKind.VoiceCommand)
            {
                // The arguments can represent many different activation types. Cast it so we can get the
                // parameters we care about out.
                var voiceArgs = args as VoiceCommandActivatedEventArgs;
                if (voiceArgs != null)
                {
                    var unused = _shell.Dispatcher.RunAsync(CoreDispatcherPriority.Normal,
                        () => this.OnActivateApplicationForVoiceCommand(voiceArgs));
                }
            }

            return base.OnActivateApplicationAsync(args);
        }

        protected override Type GetPageType(string pageToken)
        {
            return Type.GetType(pageToken, true);
        }

        private void OnActivateApplicationForProtocol(ProtocolActivatedEventArgs args)
        {
            try
            {
                var uri = args.Uri;
                this.ProcessProtocolUri(args.Uri);
            }
            catch (UriFormatException formatException)
            {
                this.EventAggregator.GetEvent<InvalidProtocolEvent>().Publish(new InvalidProtocolEventArgs(formatException));
            }
        }

        private void OnActivateApplicationForVoiceCommand(VoiceCommandActivatedEventArgs args)
        {
            SpeechRecognitionResult speechRecognitionResult = args.Result;

            // Get the name of the voice command and the text spoken. See VoiceCommandService.xml for
            // the <Command> tags this can be filled with.
            string commandName = speechRecognitionResult.RulePath[0];
            string textSpoken = speechRecognitionResult.Text;

            // The commandMode is either "voice" or "text", and it indictes how the voice command
            // was entered by the user. Apps should respect "text" mode by providing feedback in silent form.
            string commandMode = VoiceCommandService.SemanticInterpretation("commandMode", speechRecognitionResult);

            switch (commandName)
            {
                case "showDevice":
                    // Access the value of the {device} phrase in the voice command
                    var device = VoiceCommandService.SemanticInterpretation("device", speechRecognitionResult);

                    var deviceTopic = this.TopicGroupDefinitions
                        .SelectMany(g => g.TopicDefinitions)
                        .Where(t => t.DisplayName.Equals(device, StringComparison.OrdinalIgnoreCase))
                        .FirstOrDefault();

                    if (deviceTopic != null)
                    {
                        deviceTopic.SelectTopic();
                        break;
                    }
                    App.Current.NavigationService.Navigate(typeof(HomePageView), null);
                    break;

                case "showRoom":
                default:
                    App.Current.NavigationService.Navigate(typeof(HomePageView), null);
                    break;
            }
        }

        private void ProcessProtocolUri(Uri uri)
        {
            if (uri.Scheme.Equals(ProtocolParts.SchemeName, StringComparison.OrdinalIgnoreCase))
            {
                // The path segments drill down the object hiearchy
                var remainingSegments = new List<string>(uri.Segments.Select(s => Uri.UnescapeDataString(s.Trim('/', '\\'))));

                // Allow for both mylivingroom:topicGroupId and mylivingroom://topicGroupId
                if (!string.IsNullOrEmpty(uri.Host))
                {
                    if (remainingSegments.Count > 0 && string.IsNullOrEmpty(remainingSegments[0]))
                    {
                        remainingSegments[0] = uri.Host;
                    }
                    else
                    {
                        remainingSegments.Insert(0, uri.Host);
                    }
                }
                else if (remainingSegments.Count == 0)
                {
                    // If no path segments specified, then activating the application was the successful URI resolution
                    return;
                }

                // The first segment is the topic group id
                var currentSegment = remainingSegments[0];
                var topicDefinitionGroup = App.Current.TopicGroupDefinitions
                    .FirstOrDefault(g => string.Equals(g.Id, currentSegment, StringComparison.OrdinalIgnoreCase));

                if (topicDefinitionGroup == null)
                {
                    this.EventAggregator.GetEvent<InvalidProtocolEvent>()
                        .Publish(new InvalidProtocolEventArgs(uri, remainingSegments));
                    return;
                }

                // The next segment is the topic id
                remainingSegments.RemoveAt(0);
                currentSegment = remainingSegments.Count > 0 ? remainingSegments[0] : null;
                var topicDefinition = topicDefinitionGroup.TopicDefinitions
                    .Where(t => t is IProtocolProcessor || t.TopicViewModel is IProtocolProcessor)
                    .FirstOrDefault(t => string.Equals(t.Id, currentSegment, StringComparison.OrdinalIgnoreCase));

                var topicViewModel = default(IProtocolProcessor);
                if (topicDefinition != null)
                {
                    if (topicDefinition is IProtocolProcessor)
                    {
                        topicViewModel = (IProtocolProcessor)topicDefinition;
                    }
                    else if (topicDefinition != null)
                    {
                        topicViewModel = (IProtocolProcessor)topicDefinition.TopicViewModel;
                    }
                }

                if (topicViewModel == null || !topicViewModel.ProcessProtocolUri(uri, remainingSegments))
                {
                    this.EventAggregator.GetEvent<InvalidProtocolEvent>()
                        .Publish(new InvalidProtocolEventArgs(uri, remainingSegments));
                    return;
                }
            }
        }
    }
}

