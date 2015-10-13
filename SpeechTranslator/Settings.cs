using System;
using System.Collections.Generic;
using System.IO;

namespace SpeechTranslator
{
    public class Settings : Dictionary<string, string>
    {
        private IEnumerable<string> lines;

        public void Load(string path)
        {
            lines = File.ReadLines(path);
        }

        public void ReadBlock(string key)
        {
            string blockToLoad = null;
            bool loadingBlock = false;

            foreach (var line in lines)
            {
                var trimmed = line.Trim();
                var index = trimmed.IndexOf(key + "=");
                if ( index >-1)
                {
                    blockToLoad = line.Substring(index + 1);
                }

                if (blockToLoad == null)
                {
                    continue;
                }

                index = trimmed.IndexOf("[" + blockToLoad + "]");
                if (index > -1)
                {
                    loadingBlock = true;
                    continue;
                }

                if (!loadingBlock || trimmed.StartsWith( "//" ) )
                {
                    continue;
                }

                if (trimmed.StartsWith("["))
                {
                    break;
                }

                index = trimmed.IndexOf("=");
                this.Add(trimmed.Substring(0,index), trimmed.Substring(index+1));
            }
        } 
    }
}