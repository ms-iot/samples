// Copyright (c) Microsoft. All rights reserved.

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
                return new KeyValuePair<string, string>(lang.NativeName, lang.LanguageTag);
            }).ToDictionary(keyitem => keyitem.Key, valueItem => valueItem.Value);

            LanguageDisplayNames = displayNameToLanguageMap.Keys.ToList();
        }

        public bool UpdateLanguage(string displayName)
        {
            var currentLang = ApplicationLanguages.PrimaryLanguageOverride;
            var newLang = GetLanguageTagFromDisplayName(displayName);
            if (currentLang != newLang)
            {
                ApplicationLanguages.PrimaryLanguageOverride = newLang;
                return true;
            }
            return false;
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

            return lang.NativeName;
        }

    }
}
