using DeviceProviders;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AllJoynVoice
{
    static class DeviceProvidersExtensions
    {
        /// <summary>
        /// Caches the result of AllObjects.
        /// </summary>
        private static IDictionary<string, IEnumerable<IBusObject>> busObjectCache = new Dictionary<string, IEnumerable<IBusObject>>();

        /// <summary>
        /// Lists all of an IService's IBusObjects, including the ones not in the About announcement.
        /// </summary>
        /// <param name="service">The service whose objects are to be listed.</param>
        public static IEnumerable<IBusObject> AllObjects(this IService service)
        {
            lock (busObjectCache)
            {
                if (busObjectCache.ContainsKey(service.Name))
                    return busObjectCache[service.Name];

                IList<IBusObject> objects = new List<IBusObject>();
                ICollection<string> seenPaths = new HashSet<string>();

                try
                {
                    foreach (IBusObject x in service.Objects)
                        DFS(x, objects, seenPaths);
                }
                catch (Exception ex)
                {
                    Logger.LogException("AllObjects: " + service.Name, ex);
                    // DEMO TEMP FIX: throw;
                }

                busObjectCache[service.Name] = objects;

                return objects;
            }
        }

        private static void DFS(IBusObject busObject, ICollection<IBusObject> allObjects, ICollection<string> seenPaths) {
            if (seenPaths.Contains(busObject.Path)) return;

            allObjects.Add(busObject);
            seenPaths.Add(busObject.Path);

            try
            {
                foreach (IBusObject child in busObject.ChildObjects)
                    DFS(child, allObjects, seenPaths);
            }
            catch (Exception ex)
            {
                Logger.LogException("AllObjects (DFS): " + busObject.Path, ex);
                // DEMO TEMP FIX: throw;
            }
        }
    }
}
