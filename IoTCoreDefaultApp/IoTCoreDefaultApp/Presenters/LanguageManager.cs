// Copyright (c) Microsoft. All rights reserved.

using IoTCoreDefaultApp.Utils;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.ApplicationModel.Resources.Core;
using Windows.Foundation.Metadata;
using Windows.Globalization;
using Windows.Media.SpeechRecognition;
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

        
        //Chinese Langs are failing (search) with SortedDictionary
        private Dictionary<string, string> displayNameToLanguageMap;
        private SortedDictionary<string, string> displayNameToInputLanguageMap;
        private SortedDictionary<string, string> displayNameToImageLanguageMap;

        /// <summary>
        /// Contains ApplicationManifest Languages and Image Enabled Languages
        /// </summary>
        public IReadOnlyList<string> LanguageDisplayNames
        {
            get;
            set;
        }
        /// <summary>
        /// Contains Input or Keyboard Languages
        /// </summary>
        public IReadOnlyList<string> InputLanguageDisplayNames
        {
            get;
            set;
        }
        /// <summary>
        /// Contains Only Image Enabled Languages
        /// </summary>
        public IReadOnlyList<string> ImageLanguageDisplayNames
        {
            get;
            set;
        }                
       
        private LanguageManager()
        {
            List<string> imageLanguagesList = GetImageSupportedLanguages();
            //Only Image Enable Map
            displayNameToImageLanguageMap = new SortedDictionary<string, string>(
                imageLanguagesList.Select(tag =>
                {
                    var lang = new Language(tag);
                    return new KeyValuePair<string, string>(lang.NativeName, GetLocaleFromLanguageTag(lang.LanguageTag));
                }).ToDictionary(keyitem => keyitem.Key, valueItem => valueItem.Value) 
                );
        
            ImageLanguageDisplayNames = displayNameToImageLanguageMap.Keys.ToList();


            displayNameToLanguageMap = new Dictionary<string, string> (
                ApplicationLanguages.ManifestLanguages.Union(imageLanguagesList).Select(tag =>
                {
                    var lang = new Language(tag);
                    return new KeyValuePair<string, string>(lang.NativeName, GetLocaleFromLanguageTag(lang.LanguageTag));
                }).OrderBy(a => a.Key).ToDictionary(keyitem => keyitem.Key, valueItem => valueItem.Value)
                );

            LanguageDisplayNames = displayNameToLanguageMap.Keys.ToList();

            displayNameToInputLanguageMap = new SortedDictionary<string, string>(
                InputLanguages.Select(tag =>
            {
                var lang = new Language(tag);
                return new KeyValuePair<string, string>(lang.NativeName, GetLocaleFromLanguageTag(lang.LanguageTag));
            }).ToDictionary(keyitem => keyitem.Key, valueItem => valueItem.Value)
            );


            InputLanguageDisplayNames = displayNameToInputLanguageMap.Keys.ToList();
                                    
            //Exception when running in Local Machine
            try
            {
                //Add Image Enabled Languages as Global Preferences List
                if (ApiInformation.IsApiContractPresent("Windows.Foundation.UniversalApiContract", 5))
                {
                    GlobalizationPreferences.TrySetLanguages(displayNameToImageLanguageMap.Values);
                }
            }
            catch(InvalidCastException)
            {
                // This is indicitive of EmbeddedMode not being enabled (i.e.
                // running IotCoreDefaultApp on Desktop or Mobile without 
                // enabling EmbeddedMode) 
                //  https://developer.microsoft.com/en-us/windows/iot/docs/embeddedmode
            }
            
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

        /// <summary>
        /// List of GlobalizationPreferences Languages
        /// </summary>
        /// <returns></returns>
        public Dictionary<string, string> GetSupportedRegions()
        {
            Dictionary<string, string> supportedLangs = GlobalizationPreferences.Languages.Select(tag =>
            {
                var lang = new Language(tag);
                return new KeyValuePair<string, string>(lang.NativeName, lang.LanguageTag);
            }).ToDictionary(keyitem => keyitem.Key, valueItem => valueItem.Value);

            return supportedLangs;
        }

        /// <summary>
        /// Returns the full format of Locale ex: for ru, it returns ru-RU
        /// </summary>
        /// <param name="identifier"></param>
        /// <returns></returns>
        public static string GetLocaleFromLanguageTag(string identifier)
        {
            int result;

            StringBuilder localeName = new StringBuilder(500);
            result = LocaleFunctions.ResolveLocaleName(identifier, localeName, 500);

            return localeName.ToString();
        }

        /// <summary>
        /// Returns all the Image Enabled Languages supported
        /// </summary>
        /// <returns></returns>
        public List<string> GetImageSupportedLanguages()
        {
            //TODO : Replace with Win Store API
            Utils.ImageLanguages.GetMUILanguages();

            return Utils.ImageLanguages.Languages.Values.ToList();

        }


        /// <summary>
        /// Returns the Tuple with 
        ///     Item1: Image has language resources
        ///     Item2: Speech supported
        ///     Item3: Supports Localization through Manifest
        /// </summary>
        /// <param name="languageTag"></param>
        /// <returns></returns>
        public Tuple<bool, bool, bool> GetLanguageTuple(string languageTag)
        {
            List<string> imageList = GetImageSupportedLanguages();
            Tuple<bool, bool, bool> langVerify = new Tuple<bool, bool, bool>(false, false, false);

            langVerify = Tuple.Create<bool, bool, bool>(
                imageList.Contains(languageTag),
                CortanaHelper.IsCortanaSupportedLanguage(languageTag),
                displayNameToLanguageMap.Values.Contains(languageTag));

            return langVerify;
        }

            /// <summary>
            /// Check to return appropriate language depending on selection
            /// Item1: Image Exists, Item2:Speech, Item3:App Localization(manifest), Lang Tag
            /// </summary>
            /// <param name="language"></param>
            /// <returns></returns>
        public Tuple<bool, bool, bool, string> CheckUpdateLanguage(string language)
        {
            string currentLang = ApplicationLanguages.PrimaryLanguageOverride;
            string languageTag = GetLanguageTagFromDisplayName(language);
            Tuple<bool, bool, bool, string> langVerify = new Tuple<bool, bool, bool, string>(false, false, false, languageTag);

            List<string> imageLanguagesList = GetImageSupportedLanguages();
            langVerify = Tuple.Create<bool, bool, bool, string>(
                imageLanguagesList.Contains(languageTag), 
                CortanaHelper.IsCortanaSupportedLanguage(languageTag), 
                displayNameToLanguageMap.Values.Contains(languageTag),  
                languageTag);

            if (currentLang != languageTag && Language.IsWellFormed(languageTag))
            {
                //image does not contain or selected lang does not support Speech
                if (!imageLanguagesList.Contains(languageTag)  ||  !(CortanaHelper.IsCortanaSupportedLanguage(languageTag)))
                {
                    //Look for near Lang removing all sugtags
                    var filteredList = imageLanguagesList.Where ( x=> x.Contains(languageTag.Substring(0, languageTag.IndexOf('-'))));
                    foreach (var item in filteredList)
                    {
                        if (null != item && item.Trim().Length > 0)
                        {
                            //Found matching Language, take preference, continue checking
                            //Change the primary only if primary language not part of imagelist
                            if (!imageLanguagesList.Contains(languageTag)) {
                                languageTag = item;
                            }
                            //Set appropriate languageTag if Language as speech
                            if(CortanaHelper.IsCortanaSupportedLanguage(item) ) {
                                languageTag = item;
                                langVerify = Tuple.Create<bool, bool, bool, string>(
                                    imageLanguagesList.Contains(languageTag), 
                                    CortanaHelper.IsCortanaSupportedLanguage(languageTag),
                                    displayNameToLanguageMap.Values.Contains(languageTag),
                                    languageTag);
                                break;
                            }
                        
                        }
                    }
                }

                langVerify = Tuple.Create<bool, bool, bool, string>(
                        imageLanguagesList.Contains(languageTag), 
                        CortanaHelper.IsCortanaSupportedLanguage(languageTag),
                        displayNameToLanguageMap.Values.Contains(languageTag), 
                        languageTag);

            }

            return langVerify;
        }


        private string GetRegionFromBCP47LanguageTag(string languageTag)
        {
            // https://tools.ietf.org/html/bcp47
            // BCP47 language tag is formed by language tag itself along with region subtag, e.g.: 
            //   en-US -> english US region
            //   fr-CA -> french CA region
            //   ex: some are populated as this: az-Cyrl-AZ
            //   without -
            // Not an extensive implementation, but covering major Region Formats
            string region = "";

            var parts = languageTag.LastIndexOf('-');
            if (parts != -1)
            {
                region = languageTag.Substring(parts + 1);
            }
            return region;
        }


        private void SetLanguageEntites(string languageTag)
        {
            // Use BCP47 Format
            string bcp47Tag = GetRegionFromBCP47LanguageTag(languageTag);
            //Apply the PrimaryLanguage
            ApplicationLanguages.PrimaryLanguageOverride = languageTag;

            if (ApiInformation.IsApiContractPresent("Windows.Foundation.UniversalApiContract", 5))
            {
                // set user language
                if (Windows.Globalization.Language.IsWellFormed(languageTag))
                {
                    try
                    {
                        //Set the Region
                        GlobalizationPreferences.TrySetHomeGeographicRegion(bcp47Tag);

                        //Set the Speech  Language
                        Task.Run(async () =>
                        {
                            Language speechLanguage = new Language(languageTag);
                            await SpeechRecognizer.TrySetSystemSpeechLanguageAsync(speechLanguage);
                        });

                    }
                    catch (InvalidCastException)
                    {
                        // This is indicitive of EmbeddedMode not being enabled (i.e.
                        // running IotCoreDefaultApp on Desktop or Mobile without 
                        // enabling EmbeddedMode) 
                        //  https://developer.microsoft.com/en-us/windows/iot/docs/embeddedmode
                    }
                }

            } //Only for ApiContract > 5

        }
        
        /// <summary>
        /// Updates Application Language, Geographic Region and Speech Language
        /// </summary>
        /// <param name="languageTag"></param>
        /// <param name="automatic">default: true, will fallback to nearest region enabled on image </param>
        /// <returns></returns>
        public string UpdateLanguageByTag(string languageTag)
        {
            var currentLang = ApplicationLanguages.PrimaryLanguageOverride;
            
            if (currentLang != languageTag && Language.IsWellFormed(languageTag))
            {

                SetLanguageEntites(languageTag);

                // Do this twice because in Release mode, once isn't enough
                // to change the current CultureInfo (changing the WaitOne delay
                // doesn't help).
                for (int i = 0; i < 2; i++)
                {
                    // Refresh the resources in new language
                    ResourceContext.GetForCurrentView().Reset();
                    ResourceContext.GetForViewIndependentUse().Reset();

                    // Where seems to be some delay between when this is reset and when
                    // we can start re-evaluating the resources.  Without a pause, sometimes
                    // the first resource remains the previous language.
                    new System.Threading.ManualResetEvent(false).WaitOne(100);
                }

                OnPropertyChanged("Item[]");
                return languageTag;
            }

            return currentLang;
        }


        /// <summary>
        /// Updates Application Language, Geographic Region and Speech Language
        /// </summary>
        /// <param name="displayName"></param>
        /// <param name="automatic">default: true, will fallback to nearest region enabled on image </param>
        /// <returns></returns>
        public string UpdateLanguage(string displayName, bool automatic = false )
        {
            if (!automatic)
            {
                return UpdateLanguageByTag( GetLanguageTagFromDisplayName(displayName) );

            } else
            {
                var getCompatibleLang = CheckUpdateLanguage(displayName);

                return UpdateLanguageByTag(getCompatibleLang.Item4);
            }            
        }

        /// <summary>
        /// Updates the Keyboard Language
        /// </summary>
        /// <param name="displayName"></param>
        /// <returns></returns>
        public bool UpdateInputLanguage(string displayName)
        {
            var currentLang = Language.CurrentInputMethodLanguageTag;
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

        /// <summary>
        /// Returns the Language Tag given DisplayName
        /// </summary>
        /// <param name="displayName"></param>
        /// <returns></returns>
        public string GetLanguageTagFromDisplayName(string displayName)
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

        /// <summary>
        /// Returns the Current Language Tag ( ex: en-US )
        /// </summary>
        /// <returns></returns>
        public static string GetCurrentLanguageTag()
        {
            var langTag = ApplicationLanguages.PrimaryLanguageOverride;
            if (String.IsNullOrEmpty(langTag))
            {
                //For Single Lang Image's (non-en-US)
                List<string> langs = GlobalizationPreferences.Languages.ToList();
                if(langs.Count > 1 && langs.Contains("en-US") )
                {
                    langs.Remove("en-US");
                    if( langs.Count == 1 )
                    {
                        langTag = langs[0];
                    } else
                    {
                        langTag = GlobalizationPreferences.Languages[0];
                    }
                }
                else
                {
                    langTag = GlobalizationPreferences.Languages[0];
                }
            }

            return langTag;
        }

        /// <summary>
        /// Returns the Current Language DisplayName
        /// </summary>
        /// <returns></returns>
        public static string GetCurrentLanguageDisplayName()
        {
            var langTag = GetCurrentLanguageTag();
            var lang = new Language(langTag);

            return lang.NativeName;
        }

        /// <summary>
        /// Returns LanguageDisplayName from Language Tag
        /// </summary>
        /// <param name="langTag"></param>
        /// <returns></returns>
        public static string GetDisplayNameFromLanguageTag(string langTag)
        {
            if (String.IsNullOrEmpty(langTag))
            {
                return string.Empty;
            }
            var lang = new Language(langTag);

            return lang.NativeName;
        }

        /// <summary>
        /// Returns Current Keyboard Language DisplayName
        /// </summary>
        /// <returns></returns>
        public static string GetCurrentInputLanguageDisplayName()
        {
            var langTag = Language.CurrentInputMethodLanguageTag;
            var lang = new Language(langTag);

            return lang.NativeName;
        }

        /// <summary>
        /// Returns Current Region Display Name
        /// </summary>
        /// <returns></returns>
        public static string GetCurrentRegionDisplayName()
        {
            var langTag = GlobalizationPreferences.HomeGeographicRegion;
         
            return langTag;
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
