//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

using System;
using System.Text;
using System.Text.RegularExpressions;
using System.Collections.Generic;
using Microsoft.VisualStudio.TemplateWizard;
using EnvDTE;

namespace CustomWizard
{
    public class ChildWizard : RootWizard
    {
        private const string ERROR_GUID_STRING = "{ }";

        public override void RunStarted(
                            object automationObject,
                            Dictionary<string, string> replacementsDictionary,
                            WizardRunKind runKind,
                            object[] customParams)
        {
            // DSB Application Guid
            Guid appGuid = getAppGuid();

            //Guid in String Format
            string guidStr = getAppGuidString(appGuid);

            if (guidStr == null)
            {
                guidStr = ERROR_GUID_STRING;
            }

            // Root project name - solution name
            replacementsDictionary.Add(
                                "$saferootprojectname$",
                                RootWizard.GlobalDictionary["$saferootprojectname$"]);

            // DSB Application Guid String
            replacementsDictionary.Add(
                                "$appguid$",
                                guidStr);

            // Username
            String originalUsername = RootWizard.GlobalDictionary["$safeusername$"];
            String latestUsername	= removeSpecialCharacters(originalUsername);
            replacementsDictionary.Add(
                                "$safeusername$",
                                latestUsername);
        }

        // Create a guid for the DSB Application
        private Guid getAppGuid()
        {
            return Guid.NewGuid();
        }

        // Get the Application Guid String that is
        // going to be used to initialize the guid byte array.
        private string getAppGuidString(Guid appGuid)
        {
            if (appGuid == Guid.Empty)
            {
                return null;
            }
            else
            {
                // X stands for GUID format <=> {0x00000000,0x0000,0x0000,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}}
                return appGuid.ToString("X");
            }
        }

        // Remove the special characters from a string.
        // This function is used to process the template parameter $username$
        private string removeSpecialCharacters(string str)
        {
            return Regex.Replace(str, "[^a-zA-Z0-9]+", "", RegexOptions.Compiled);
        }
    }
}