using System;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace WeatherStation
{
    struct AuthData
    {
        public string VerificationUrl;
        public string UserCode;
    }

    interface IPBIClient
    {
        Task<AuthData> AcquireDeviceCodeAsync();
        Task CompleteAuthentication();
        Task ClearRowsAsync(string datasetName, string tableName);
        Task ConnectAsync();
        Task CreateDatasetAsync(string datasetName);
        Task<PBIDatasets> GetAllDatasetsAsync();
        Task<IEnumerable<Dataset>> GetDatasetsWithNameAsync(string name);
        Task<string> PushDataToTableAsync<T>(Dataset dataset, string tableName, T data);
    }
}