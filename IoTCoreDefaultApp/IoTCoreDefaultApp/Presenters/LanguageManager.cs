/*
    Copyright(c) Microsoft Open Technologies, Inc. All rights reserved.

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
*/

using System;
using System.Collections.Generic;
using System.Linq;
using Windows.Globalization;
using Windows.System;
using Windows.System.UserProfile;

namespace IoTCoreDefaultApp
{
    public class LanguageManager
    {
        private Dictionary<string, string> displayNameToLanguageMap;
        public IReadOnlyList<string> LanguageDisplayNames
        {
            get;
            set;
        }

        public LanguageManager()
        {
            displayNameToLanguageMap = ApplicationLanguages.ManifestLanguages.Select(tag =>
            {
                var lang = new Language(tag);
                return new KeyValuePair<string, string>(lang.DisplayName, lang.LanguageTag);
            }).ToDictionary(keyitem => keyitem.Key, valueItem => valueItem.Value);

            LanguageDisplayNames = displayNameToLanguageMap.Keys.ToList();
        }

        public void UpdateLanguage(string displayName)
        {
            ApplicationLanguages.PrimaryLanguageOverride = GetLanguageTagFromDisplayName(displayName);
        }

        private string GetLanguageTagFromDisplayName(string displayName)
        {
            string newLang;
            displayNameToLanguageMap.TryGetValue(displayName, out newLang);

            if (newLang == null)
            {
                throw new ArgumentException("Failed to get language tag for "+ displayName);
            }

            return newLang;
        }

        public static string GetCurrentLanguageDisplayName()
        {
            var langTag = ApplicationLanguages.PrimaryLanguageOverride;
            if (String.IsNullOrEmpty(langTag))
            {
                langTag = GlobalizationPreferences.Languages[0];
            }
            var lang = new Language(langTag);

            return lang.DisplayName;
        }

        public static IReadOnlyList<string> GetSupportedTimeZones()
        {
            return TimeZoneSettings.SupportedTimeZoneDisplayNames;
        }

        public static string GetCurrentTimeZone()
        {
            return TimeZoneSettings.CurrentTimeZoneDisplayName;
        }

        public static void ChangeTimeZone(string timeZone)
        {
            if (!TimeZoneSettings.CanChangeTimeZone)
            {
                return;
            }

            if (!TimeZoneSettings.SupportedTimeZoneDisplayNames.Contains(timeZone))
            {
                throw new ArgumentException("Failed to change timezone to " + timeZone);
            }

            TimeZoneSettings.ChangeTimeZoneByDisplayName(timeZone);

            // "Workaround" to flush TimeZoneInfo cache. Yes, this really works.
            TimeZoneInfo.ConvertTime(DateTime.MinValue, TimeZoneInfo.Local); 
        }
    }
}
