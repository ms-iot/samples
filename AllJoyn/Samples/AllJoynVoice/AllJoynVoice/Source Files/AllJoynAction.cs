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
using System.Linq;
using System.Threading.Tasks;
using Windows.Data.Json;
using DeviceProviders;

namespace AllJoynVoice
{
    /// <summary>
    /// Corresponds to an entry in the actions array of the configuration file.
    /// </summary>
    class AllJoynAction
    {
        public readonly string Id;
        public readonly IReadOnlyList<string> Commands;

        private readonly IList<Interaction> Interactions;

        public AllJoynAction(string id, List<string> commands, JsonArray actionsConfig)
        {
            Id = id;
            Commands = commands;
            Interactions = actionsConfig.Select(x => new Interaction(x.GetObject())).ToList();
        }

        /// <summary>
        /// Asynchronously executes the configured actions.
        /// </summary>
        public async Task RunAsync()
        {
            await Task.Yield(); // force this to run asynchronously

            Logger.LogInfo("Starting AllJoynAction: " + Id);

            foreach (var interaction in Interactions)
            {
                if (interaction.Status != InteractionStatus.Ready) continue;

                try
                {
                    await interaction.ExecuteAsync();
                }
                catch (Exception) { }; // TODO: Maybe report how many interactions actually worked?
            }
        }

        private enum InteractionStatus
        {
            /// <summary> Waiting for the device's about announcement. If deviceID is a wildcard, waiting for the first device. </summary>
            Discovery,
            /// <summary> There was an error parsing the configuration data. </summary>
            ConfigError,
            /// <summary> The interaction is ready to be called. </summary>
            Ready
        };

        private class Interaction
        {
            public string DeviceId { get; private set; }
            public string Path { get; private set; }
            public string InterfaceName { get; private set; }

            public async Task ExecuteAsync()
            {
                foreach (var action in Actions)
                {
                    await action();
                }
            }

            private IList<Func<Task>> Actions = new List<Func<Task>>();

            public InteractionStatus Status
            {
                get
                {
                    if (Actions.Any())
                    {
                        return InteractionStatus.Ready;
                    }
                    else if (ErrorDescription != null)
                    {
                        return InteractionStatus.ConfigError;
                    }
                    else
                    {
                        return InteractionStatus.Discovery;
                    }
                }
            }

            /// <summary>
            /// If the Status is ConfigError, a string detailing the error.
            /// </summary>
            public string ErrorDescription { get; private set; }

            private JsonObject InteractionConfig;

            public Interaction(JsonObject InteractionConfig)
            {
                this.InteractionConfig = InteractionConfig;

                try
                {
                    DeviceId = InteractionConfig.GetNamedString("deviceID");
                    Path = InteractionConfig.GetNamedString("path");
                    InterfaceName = InteractionConfig.GetNamedString("interface");
                }
                catch (Exception ex)
                {
                    Logger.LogException(this.GetType().Name, ex);
                    failDiscovery("Invalid configuration: " + ex.Message);
                }

                Config.ServiceJoined += ServiceJoined;
            }

            private void ServiceJoined(IService service)
            {
                // Don't continue if the new service isn't the one we're interested in.
                if (DeviceId != "*" && service.AboutData.DeviceId != DeviceId) return;

                // Prepare the interaction.

                IEnumerable<IBusObject> busObjects = new List<IBusObject>();

                if (Path == "*")
                {
                    // Get everything
                    try
                    {
                        busObjects = service.AllObjects().Where(x => x.Interfaces != null && x.Interfaces.Any(y => y.Name == InterfaceName));
                    }
                    catch (Exception ex)
                    {
                        Logger.LogException("Interaction", ex);
                    }
                }
                else
                {
                    try
                    {
                        // DEMO TEMP FIX:
                        // Replace with IService::GetBusObject()
                        busObjects = new List<IBusObject>() {
                            service.AllObjects().First(x => x.Path == Path)
                        };
                    }
                    catch (InvalidOperationException)
                    {
                        failDiscovery("Couldn't find bus object: " + Path);
                        return;
                    }
                }

                foreach (IBusObject busObject in busObjects)
                {
                    IInterface @interface;
                    int delayMSec = 0;

                    try
                    {
                        @interface = busObject.Interfaces.First(x => x.Name == InterfaceName);
                    }
                    catch (InvalidOperationException)
                    {
                        failDiscovery(busObject.Path + ", couldn't find interface: " + InterfaceName);
                        return;
                    }

                    if (InteractionConfig.ContainsKey("delay"))
                    {
                        try
                        {
                            delayMSec = (int)InteractionConfig["delay"].GetNumber();
                        }
                        catch (InvalidCastException)
                        {
                            failDiscovery("Couldn't cast delay value: " + InteractionConfig.GetNamedString("delay"));
                        }
                    }

                    if (InteractionConfig.ContainsKey("propertyName"))
                    {
                        IProperty property = null;
                        object newValue = null;

                        try
                        {
                            property = @interface.Properties.First(x => x.Name == InteractionConfig.GetNamedString("propertyName"));
                        }
                        catch (InvalidOperationException)
                        {
                            failDiscovery("Couldn't find property: " + InteractionConfig.GetNamedString("propertyName"));
                            return;
                        }

                        try
                        {
                            newValue = AllJoynTypes.Convert(property.TypeInfo, InteractionConfig["propertyValue"]);
                        }
                        catch (InvalidCastException)
                        {
                            failDiscovery("Couldn't cast property value for property: " + InteractionConfig.GetNamedString("propertyName"));
                            return;
                        }

                        Actions.Add(
                            async () =>
                            {
                                if (delayMSec > 0)
                                {
                                    await System.Threading.Tasks.Task.Delay(delayMSec);
                                }

                                AllJoynStatus status = await property?.SetValueAsync(newValue);

                                if (status == null || status.IsFailure)
                                {
                                    throw Logger.LogException(service.Name,
                                        new InvalidOperationException(
                                            string.Format("{0} - Couldn't modify property: {1}", status, property.Name)
                                        )
                                    );
                                }
                            }
                        );
                    }
                    else if (InteractionConfig.ContainsKey("methodName"))
                    {
                        IMethod method = null;
                        List<object> methodArgs = null;

                        try
                        {
                            method = @interface.Methods.First(x => x.Name == InteractionConfig.GetNamedString("methodName"));
                        }
                        catch (InvalidOperationException)
                        {
                            failDiscovery("Couldn't find method: " + InteractionConfig.GetNamedString("methodName"));
                            return;
                        }

                        try
                        {
                            methodArgs = Enumerable.Zip(method.InSignature,
                                                        InteractionConfig.GetNamedArray("methodArguments"),
                                                        (t, x) => AllJoynTypes.Convert(t.TypeDefinition, x)
                                                       ).ToList();
                        }
                        catch (InvalidCastException)
                        {
                            failDiscovery("Couldn't cast arguments for method: " + InteractionConfig.GetNamedString("methodName"));
                            return;
                        }

                        Actions.Add(
                            async () =>
                            {
                                if (delayMSec > 0)
                                {
                                    await System.Threading.Tasks.Task.Delay(delayMSec);
                                }

                                InvokeMethodResult status = await method?.InvokeAsync(methodArgs);

                                if (status == null || status.Status.IsFailure)
                                {
                                    throw Logger.LogException(service.Name,
                                        new InvalidOperationException(
                                            string.Format("{0} - Couldn't invoke method: {1}", status, method.Name)
                                        )
                                    );
                                }
                            }
                        );
                    }
                    else
                    {
                        failDiscovery("Interaction does not have property nor method information.");
                        return;
                    }
                }
            }

            private void failDiscovery(string error)
            {
                ErrorDescription = error;
                Logger.LogError(error);
            }
        }
    }
}
