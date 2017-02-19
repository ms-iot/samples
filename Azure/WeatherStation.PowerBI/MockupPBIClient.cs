using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace WeatherStation
{
    class MockupPBIClient : IPBIClient
    {
        public Task<AuthData> AcquireDeviceCodeAsync()
        {
            var dummyAuthData = new AuthData();
            return Task<AuthData>.FromResult(dummyAuthData);
        }

        public Task ClearRowsAsync(string datasetName, string tableName)
        {
            return Task.CompletedTask;
        }

        public Task CompleteAuthentication()
        {
            return Task.CompletedTask;
        }

        public Task ConnectAsync()
        {
            return Task.CompletedTask;
        }

        public Task CreateDatasetAsync(string datasetName)
        {
            return Task.CompletedTask;
        }

        public Task<PBIDatasets> GetAllDatasetsAsync()
        {
            var datasets = new PBIDatasets();
            datasets.Datasets = new Dataset[] { new Dataset { Name = "SampleDataset" } };
            return Task.FromResult(datasets);
        }

        public Task<IEnumerable<Dataset>> GetDatasetsWithNameAsync(string name)
        {
            var datasets = new Dataset[] { new Dataset { Name = name } };
            IEnumerable<Dataset> ie = datasets;
            return Task.FromResult(ie);
        }

        public Task<string> PushDataToTableAsync<T>(Dataset dataset, string tableName, T data)
        {
            return Task.FromResult("");
        }
    }
}
