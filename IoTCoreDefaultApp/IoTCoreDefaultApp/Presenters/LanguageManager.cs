// Copyright (c) Microsoft. All rights reserved.

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using Windows.ApplicationModel.Resources.Core;
using Windows.Globalization;
using Windows.System;
using Windows.System.UserProfile;

namespace IoTCoreDefaultApp
{
    public class LanguageManager : INotifyPropertyChanged
    {
        private Dictionary<string, string> displayNameToLanguageMap;
        public IReadOnlyList<string> LanguageDisplayNames
        {
            get;
            set;
        }

        private LanguageManager()
        {
            displayNameToLanguageMap = ApplicationLanguages.ManifestLanguages.Select(tag =>
            {
                var lang = new Language(tag);
                return new KeyValuePair<string, string>(lang.NativeName, lang.LanguageTag);
            }).ToDictionary(keyitem => keyitem.Key, valueItem => valueItem.Value);

            LanguageDisplayNames = displayNameToLanguageMap.Keys.ToList();
        }


        private static LanguageManager _LanguageManager = null;
        public static LanguageManager GetInstance()
        {
            if (_LanguageManager == null)
            {
                _LanguageManager = new LanguageManager();
            }
            return _LanguageManager;
        }

        public bool UpdateLanguage(string displayName)
        {
            var currentLang = ApplicationLanguages.PrimaryLanguageOverride;
            var newLang = GetLanguageTagFromDisplayName(displayName);
            if (currentLang != newLang)
            {
                ApplicationLanguages.PrimaryLanguageOverride = newLang;

                // Refresh the resources in new language
                ResourceContext.GetForCurrentView().Reset();

                // Where seems to be some delay between when this is reset and when
                // we can start re-evaluating the resources.  Without a pause, sometimes
                // the first resource remains the previous language.
                new System.Threading.ManualResetEvent(false).WaitOne(100);

                OnPropertyChanged("Item[]");
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

        public string this[string key]
        {
            get
            {
                var resourceLoader = Windows.ApplicationModel.Resources.ResourceLoader.GetForCurrentView();
                var localized = resourceLoader.GetString(key);
                return localized;
            }
        }

        public void OnPropertyChanged(string property)
        {
            var eventInst = PropertyChanged;
            if (eventInst != null)
            {
                eventInst.Invoke(this, new PropertyChangedEventArgs(property));
            }
        }

        public event PropertyChangedEventHandler PropertyChanged;

    }
}
