using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

namespace IoTCoreDefaultApp.Utils
{

    /// <summary>
    /// Using WinAPI Calls to get the List of Image Enabled Languages. 
    /// TODO: This should be replaced with Store Enabled APIs.
    ///       EnumUILanguages, EnumUILanguagesProc (callback)
    /// </summary>
    internal static class ImageLanguages
    {
        public static Dictionary<int, string> Languages = new Dictionary<int, string>();

        [DllImport("api-ms-win-core-localization-obsolete-l1-3-0.dll", CharSet = CharSet.Unicode)]
        static extern System.Boolean EnumUILanguagesW(
            EnumUILanguagesProc lpUILanguageEnumProc,
            System.UInt32 dwFlags,
            System.IntPtr lParam
            );

        private static bool UILanguageProc(IntPtr lpLang, IntPtr lParam)
        {
            int langID = Convert.ToInt32(Marshal.PtrToStringUni(lpLang), 16);
            StringBuilder data = new StringBuilder(500);
           
            if (!Languages.ContainsKey(langID))
            {
                LocaleFunctions.LCIDToLocaleName(Convert.ToUInt32(langID), data, data.Capacity, 0);
                Languages.Add(langID, data.ToString());
            }

            return true;
        }

        public static void GetMUILanguages()
        {
            EnumUILanguagesW(UILanguageProc, 0, IntPtr.Zero);
        }
    }

    /// <summary>
    /// Callback function for EnumUILanguages
    /// </summary>
    /// <param name="lpUILanguageString"></param>
    /// <param name="lParam"></param>
    /// <returns></returns>
    delegate System.Boolean EnumUILanguagesProc(
            System.IntPtr lpUILanguageString,
            System.IntPtr lParam
            );

    /// <summary>
    /// Enabled with Store Apps API
    /// </summary>
    internal static class LocaleFunctions
    {
        [DllImport("api-ms-win-core-localization-l1-2-1.dll", SetLastError = true, CharSet = CharSet.Unicode)]
        public static extern int ResolveLocaleName(string lpNameToResolve, StringBuilder lpLocaleName, int cchLocaleName);

        [DllImport("api-ms-win-core-localization-l1-2-1.dll", SetLastError = true, CharSet = CharSet.Unicode)]
        public static extern int LCIDToLocaleName(uint Locale, StringBuilder lpName, int cchName, int dwFlags);
    }
        

}
