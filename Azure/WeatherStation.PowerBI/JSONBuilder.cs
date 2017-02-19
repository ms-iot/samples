// Copyright Microsoft 2015

using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using Newtonsoft.Json;
using Newtonsoft.Json.Serialization;

namespace WeatherStation
{
    public static class JSONBuilder
    {
        public static string ToJson(object obj)
        {
            StringBuilder jsonBuilder = new StringBuilder();

            jsonBuilder.Append("{\"rows\":");
            var json_value = JsonConvert.SerializeObject(obj, new JsonSerializerSettings());
            jsonBuilder.Append(json_value);
            jsonBuilder.Append("}");

            return jsonBuilder.ToString();
        }

        public static string ToJsonSchema(object obj, string name)
        {
            StringBuilder jsonSchemaBuilder = new StringBuilder();
            string typeName = string.Empty;

            jsonSchemaBuilder.Append(string.Format("{0}\"name\": \"{1}\",\"tables\": [", "{", name));
            jsonSchemaBuilder.Append(String.Format("{0}\"name\": \"{1}\", ", "{", obj.GetType().Name));
            jsonSchemaBuilder.Append("\"columns\": [");

            PropertyInfo[] properties = obj.GetType().GetProperties();

            foreach (PropertyInfo p in properties)
            {
                string sPropertyTypeName = p.PropertyType.Name;
                if (sPropertyTypeName.StartsWith("Nullable") && p.PropertyType.GenericTypeArguments != null && p.PropertyType.GenericTypeArguments.Length == 1)
                    sPropertyTypeName = p.PropertyType.GenericTypeArguments[0].Name;
                switch (sPropertyTypeName)
                {
                    case "Int32":
                    case "Int64":
                        typeName = "Int64";
                        break;
                    case "Double":
                        typeName = "Double";
                        break;
                    case "Boolean":
                        typeName = "bool";
                        break;
                    case "DateTime":
                        typeName = "DateTime";
                        break;
                    case "DateTimeOffset":
                        typeName = "DateTimeOffset";
                        break;
                    case "String":
                        typeName = "string";
                        break;
                    default:
                        typeName = null;
                        break;
                }

                if (typeName == null)
                    throw new Exception("type not supported");

                jsonSchemaBuilder.Append(string.Format("{0} \"name\": \"{1}\", \"dataType\": \"{2}\"{3},", "{", p.Name, typeName, "}"));
            }

            jsonSchemaBuilder.Remove(jsonSchemaBuilder.ToString().Length - 1, 1);
            jsonSchemaBuilder.Append("]}]}");

            return jsonSchemaBuilder.ToString();
        }
    }
}
