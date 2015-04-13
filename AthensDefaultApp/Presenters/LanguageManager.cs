using System;
using System.Collections.Generic;
using System.Linq;
using Windows.Globalization;
using Windows.System;
using Windows.System.UserProfile;

namespace AthensDefaultApp
{
    public class LanguageManager
    {
        private Dictionary<string, string> displayNameToLanguageMap;
        private static bool hasChangedTimeZone = false;
        public IReadOnlyList<string> LanguageDisplayNames
        {
            get;
        }

        public LanguageManager()
        {
            displayNameToLanguageMap = ApplicationLanguages.Languages.Select(tag =>
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
                throw new ArgumentException("displayName");
            }

            return newLang;
        }

        public static string GetCurrentLanguageDisplayName()
        {
            var lang = new Language(GlobalizationPreferences.Languages[0]);

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

        public static DateTime GetTimeZoneAdjustedDateTimeNow()
        {
            if (!hasChangedTimeZone)
            {
                return DateTime.Now;
            }

            var timeZoneString = GetCurrentTimeZone();  // In the format (UTC+00:00) <NAME>

            if (timeZoneString.StartsWith("(UTC)")) // Exception being UTC time zones
            {
                return DateTime.UtcNow;
            }

            var utcOffset = timeZoneString.Substring(4, 6); // Pull off +00:00 portion of the display name

            var positive = utcOffset[0] == '+'; // Determine which side of UTC we are at

            var hours = Convert.ToInt32(utcOffset.Substring(1, 2)); // Pull off first part of the time for hours and assume daylight savings right now
            var minutes = Convert.ToInt32(utcOffset.Substring(4)); // Last two characters for minutes

            var offset = new TimeSpan(hours, minutes, 0);

            // subtract or add depending depending on the UTC side
            var finalTime = positive == true ? DateTime.UtcNow.Add(offset) : DateTime.UtcNow.Subtract(offset);

            return finalTime.Add(TimeSpan.FromHours(1)); // Hack for DST
        }

        public static void ChangeTimeZone(string timeZone)
        {
            if (!TimeZoneSettings.CanChangeTimeZone)
            {
                return;
            }

            if (!TimeZoneSettings.SupportedTimeZoneDisplayNames.Contains(timeZone))
            {
                throw new ArgumentException("timeZone");
            }

            TimeZoneSettings.ChangeTimeZoneByDisplayName(timeZone);
            hasChangedTimeZone = true;
        }
    }
}
