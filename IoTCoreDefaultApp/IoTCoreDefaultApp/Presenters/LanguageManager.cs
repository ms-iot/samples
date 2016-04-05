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
        private readonly string[] InputLanguages = {
            "af",      "ar-SA",    "as",         "az-Cyrl",    "az-Latn",
            "ba-Cyrl", "be",       "bg",         "bn-IN",      "bo-Tibt",
            "bs-Cyrl", "chr-Cher", "cs",         "cy",         "da",
            "de-CH",   "de-DE",    "dv",         "el",         "en-CA",
            "en-GB",   "en-IE",    "en-IN",      "en-US",      "es-ES",
            "es-MX",   "et",       "fa",         "fi",         "fo",
            "fr-BE",   "fr-CA",    "fr-CH",      "fr-FR",      "gn",
            "gu",      "ha-Latn",  "haw-Latn",   "he",         "hi",
            "hr-HR",   "hsb",      "hu",         "hy",         "ig-Latn",
            "is",      "it-IT",    "iu-Latn",    "ja",         "ka",
            "kk",      "kl",       "km",         "kn",         "ko",
            "ku-Arab", "ky-Cyrl",  "lb",         "lo",         "lt",
            "lv",      "mi-Latn",  "mk",         "ml",         "mn-Cyrl",
            "mn-Mong", "mr",       "mt",         "my",         "nb",
            "ne-NP",   "nl-BE",    "nl-NL",      "nso",        "or",
            "pa",      "pl",       "ps",         "pt-BR",      "pt-PT",
            "ro-RO",   "sah-Cyrl", "se-Latn-NO", "se-Latn-SE", "si",
            "sk",      "sl",       "sq",         "sv-SE",      "syr-Syrc",
            "ta-IN",   "te",       "tg-Cyrl",    "th",         "tk-Latn",
            "tn-ZA",   "tr",       "tt-Cyrl",    "tzm-Latn",   "tzm-Tfng",
            "ug-Arab", "uk",       "ur-PK",      "uz-Cyrl",    "vi",
            "wo-Latn", "yo-Latn"
            };

        private Dictionary<string, string> displayNameToLanguageMap;
        private Dictionary<string, string> displayNameToInputLanguageMap;
        public IReadOnlyList<string> LanguageDisplayNames
        {
            get;
            set;
        }

        public IReadOnlyList<string> InputLanguageDisplayNames
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

            displayNameToInputLanguageMap = InputLanguages.Select(tag =>
            {
                var lang = new Language(tag);
                return new KeyValuePair<string, string>(lang.NativeName, lang.LanguageTag);
            }).ToDictionary(keyitem => keyitem.Key, valueItem => valueItem.Value);

            InputLanguageDisplayNames = displayNameToInputLanguageMap.Keys.ToList();
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
                // Do this twice because in Release mode, once isn't enough
                // to change the current CultureInfo (changing the WaitOne delay
                // doesn't help).
                for (int i = 0; i < 2; i++)
                {
                    ApplicationLanguages.PrimaryLanguageOverride = newLang;

                    // Refresh the resources in new language
                    ResourceContext.GetForCurrentView().Reset();
                    ResourceContext.GetForViewIndependentUse().Reset();

                    // Where seems to be some delay between when this is reset and when
                    // we can start re-evaluating the resources.  Without a pause, sometimes
                    // the first resource remains the previous language.
                    new System.Threading.ManualResetEvent(false).WaitOne(100);
                }

                OnPropertyChanged("Item[]");
                return true;
            }
            return false;
        }

        public bool UpdateInputLanguage(string displayName)
        {
            var currentLang = Windows.Globalization.Language.CurrentInputMethodLanguageTag;
            var newLang = GetInputLanguageTagFromDisplayName(displayName);
            if (currentLang != newLang)
            {
                if (!Language.TrySetInputMethodLanguageTag(newLang))
                {
                    return false;
                }

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

        private string GetInputLanguageTagFromDisplayName(string displayName)
        {
            string newLang;
            displayNameToInputLanguageMap.TryGetValue(displayName, out newLang);

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

        public static string GetCurrentInputLanguageDisplayName()
        {
            var langTag = Windows.Globalization.Language.CurrentInputMethodLanguageTag;
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
