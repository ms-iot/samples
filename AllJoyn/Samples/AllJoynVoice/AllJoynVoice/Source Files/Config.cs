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
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using Windows.Data.Json;
using Windows.Storage;
using DeviceProviders;

namespace AllJoynVoice
{
    class Config
    {
        const string configFilename = @"config.json";
        const string allJoynJSFilename = @"alljoyn.js";

        private static readonly IReadOnlyList<string> configDirs = new List<string>()
        {
            @"ms-appdata:///local/",
            @"ms-appx:///",
        };

        public static string Codeword { get; private set; }

        public static IReadOnlyList<AllJoynAction> Actions { get; private set; }

        public static AllJoynNotifier Notifier { get; private set; }

        public delegate void ServiceJoinedHandler(IService service);
        public static event ServiceJoinedHandler ServiceJoined;

        private static IProvider Provider;

        /// <summary>
        /// Loads and parses the configuration file.
        /// </summary>
        /// <returns>A Task that must be awaited before trying to access any other class members.</returns>
        public async static Task Load()
        {
            // Set up AllJoynProvider
            {
                Config.Provider = new AllJoynProvider();

                Config.Provider.ServiceJoined += (s, e) => {
                    Logger.LogInfo("Service joined: " + e.Service.AboutData.AppName + ", " + e.Service.AboutData.DeviceId);

                    if (ServiceJoined != null)
                        ServiceJoined(e.Service);
                };

                Config.Notifier = new AllJoynNotifier();
            }

            // Parse configuration file. Throws if something went wrong.
            {
                string configFileContents = null;

                try
                {
                    StorageFile configFile = await findFile(configFilename);
                    configFileContents = await FileIO.ReadTextAsync(configFile);
                }
                catch (Exception)
                {
                    throw Logger.LogException("Config", new FileNotFoundException("Couldn't find the configuration file."));
                }

                parseConfig(configFileContents);
            }

            // Start AllJoynProvider after subscribing to everything.
            {
                Logger.LogInfo("Starting AllJoynProvider...");

                Config.Provider.Start();
            }
        }

        /// <summary>
        /// Tries looking for the specified filename in the configDirs.
        /// </summary>
        /// <param name="filename">The file to find</param>
        /// <returns>The contents or the file or null.</returns>
        private static async Task<StorageFile> findFile(string filename)
        {
            foreach (string dir in configDirs)
            {
                Uri uri = new Uri(dir + filename);

                try
                {
                    return await StorageFile.GetFileFromApplicationUriAsync(uri);
                }
                catch (FileNotFoundException)
                {
                    Logger.LogError("Couldn't open file: " + uri);
                }
            }

            return null;
        }

        /// <summary>
        /// Takes the contents of the JSON config file and parses them.
        /// </summary>
        /// <param name="content">The contents of the config file as a string.</param>
        private static void parseConfig(string content)
        {
            var root = JsonObject.Parse(content);

            Config.Codeword = root.GetNamedString("codeword");

            Config.Actions = root.GetNamedArray("actions")
                                 .Select(x =>
                                 {
                                     JsonObject xObj = x.GetObject();

                                     string id = xObj.GetNamedString("actionID");
                                     List<string> constraints = xObj.GetNamedArray("spokenCommands").Select(y => y.GetString()).ToList();
                                     JsonArray actionConfig = xObj.GetNamedArray("interactions");

                                     return new AllJoynAction(id, constraints, actionConfig);
                                 }).ToList();
        }
    }
}