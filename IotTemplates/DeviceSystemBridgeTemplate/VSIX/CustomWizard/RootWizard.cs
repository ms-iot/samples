﻿//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//

using System;
using System.Collections.Generic;
using Microsoft.VisualStudio.TemplateWizard;
using EnvDTE;
using System.IO;

namespace CustomWizard
{
    public class RootWizard : IWizard
    {
        private static readonly int NUMBER_OF_PROJECTS = 5;

        // this value come from VSLangProj.PrjKind.prjKindCSharpProject
        private const string prjKindCSharpProject = "{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}";

        private const string _rspFileName = "\\CscOptions.rsp";
        private string[] _cscOptions = { "/checksumalgorithm:SHA256" };
        private static string _destinationDir;

        // Indices are found by iterating through the EnvDTE.DTE.Solution.Projects
        private static readonly int ADAPTERLIB_PROJECT_INDEX = 2;
        private static readonly int BRIDGERT_PROJECT_INDEX = 4;

        //Use to communicate $saferootprojectname$ to BridgeRTWizard
        public static Dictionary<String, String> GlobalDictionary = new Dictionary<String, String>();

        // This method is called before opening any item that
        // has the OpenInEditor attribute.
        public void BeforeOpeningFile(ProjectItem projectItem)
        {
        }

        public void ProjectFinishedGenerating(Project project)
        {
            // if necessary C# compiler options that will be used by all CS projects 
            if (null != project &&
                project.Kind == prjKindCSharpProject)
            {
                string fileName = _destinationDir + _rspFileName;
                if (!File.Exists(fileName))
                {
                    File.WriteAllLines(fileName, _cscOptions);
                }
            }
        }

        // This method is only called for item templates,
        // not for project templates.
        public void ProjectItemFinishedGenerating(ProjectItem projectItem)
        {
        }

        // This method is called after the project is created.
        public void RunFinished()
        {
            EnvDTE.DTE dte = (EnvDTE.DTE)System.Runtime.InteropServices.Marshal.GetActiveObject("VisualStudio.DTE");

            int projectCount = dte.Solution.Projects.Count;

            if (projectCount == NUMBER_OF_PROJECTS)
            {
                Project adapterLibProject   = dte.Solution.Item(ADAPTERLIB_PROJECT_INDEX);
                Project bridgeRTProject     = dte.Solution.Item(BRIDGERT_PROJECT_INDEX);

                BuildDependency bd = dte.Solution.SolutionBuild.BuildDependencies.Item(adapterLibProject.UniqueName);
                if (bd != null)
                {
                    bd.AddProject(bridgeRTProject.UniqueName);
                }
            }
        }

        public virtual void RunStarted(
                            object automationObject,
                            Dictionary<string, string> replacementsDictionary,
                            WizardRunKind runKind,
                            object[] customParams)
        {
            // Place root $safeprojectname$ to Global Dictionary
            GlobalDictionary["$saferootprojectname$"] = replacementsDictionary["$safeprojectname$"];

            // Place $username$ to Global Dictionary
            GlobalDictionary["$safeusername$"] = replacementsDictionary["$username$"];

            // save away destination directory
            _destinationDir = replacementsDictionary["$destinationdirectory$"];
        }

        // This method is only called for item templates,
        // not for project templates.
        public bool ShouldAddProjectItem(string filePath)
        {
            return true;
        }
    }
}
