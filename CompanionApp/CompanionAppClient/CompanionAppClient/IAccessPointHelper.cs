// Copyright (c) Microsoft. All rights reserved.

using System;
using System.Collections.ObjectModel;
using System.Threading.Tasks;

namespace CompanionAppClient
{
    public interface IAccessPointHelper
    {
        event Action<string> AccessPointsEnumeratedEvent;
        event Action<string> AccessPointConnectedEvent;
        event Action<string> ClientNetworkConnectedEvent;
        event Action<string> ClientNetworksEnumeratedEvent;

        Task FindAccessPoints(ObservableCollection<AccessPoint> availableAccessPoints);
        Task ConnectToAccessPoint(AccessPoint accessPoint);
        Task RequestClientNetworks(ObservableCollection<Network> availableNetworks);
        Task ConnectToClientNetwork(string networkSsid, string password);
        Task DisconnectFromClientNetwork(string networkSsid);
        Task Disconnect();
    }
}
