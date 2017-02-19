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
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Reflection;
using System.Windows.Input;
using Windows.UI.Xaml.Markup;

namespace MyLivingRoom.ViewModels
{
    public class InvocationAction
    {
        public string DeviceId { get; set; }
        public string PropertyName { get; set; }
        public object PropertyValue { get; set; }
    }

    public class InvocationActionCollection : Collection<InvocationAction>
    {
    }

    [ContentProperty(Name = "Actions")]
    public class ActionInvocationTopicDefinition : TopicDefinition, IProtocolProcessor
    {
        public InvocationActionCollection Actions { get; } = new InvocationActionCollection();

        public bool ProcessProtocolUri(Uri uri, IList<string> remainingSegments)
        {
            this.SelectTopic();
            return true;
        }

        public override void SelectTopic()
        {
            var deviceDefinitions = App.Current
                .TopicGroupDefinitions
                .SelectMany(g => g.TopicDefinitions)
                .Where(t => t.TopicViewModel != null ? (t.TopicViewModel is BaseDeviceViewModel) : false)
                .ToDictionary(t => t.Id);

            foreach (var action in this.Actions)
            {
                var topicDefinition = default(TopicDefinition);
                if (deviceDefinitions.TryGetValue(action.DeviceId, out topicDefinition) && topicDefinition != null)
                {
                    var topicViewModel = topicDefinition.TopicViewModel;
                    var propertyInfo = topicViewModel?.GetType().GetProperty(action.PropertyName);
                    if (propertyInfo != null)
                    {
                        if (typeof(ICommand).IsAssignableFrom(propertyInfo.PropertyType))
                        {
                            var command = propertyInfo.GetValue(topicViewModel) as ICommand;
                            if (command != null && command.CanExecute(action.PropertyValue))
                            {
                                command.Execute(action.PropertyValue);
                            }
                        }
                        else if (propertyInfo.CanWrite)
                        {
                            propertyInfo.SetValue(topicViewModel, action.PropertyValue);
                        }
                    }
                }
            }
        }
    }
}
