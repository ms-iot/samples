// Copyright (c) Microsoft. All rights reserved.

using System;
using System.Collections.ObjectModel;

namespace CompanionAppClient
{
    public interface IAccessPointHelper
    {
        event Action<string> AccessPointsEnumeratedEvent;
        event Action<string> AccessPointConnectedEvent;
        event Action<string> ClientNetworkConnectedEvent;
        event Action<string> ClientNetworksEnumeratedEvent;

        void FindAccessPoints(ObservableCollection<AccessPoint> availableAccessPoints);
        void ConnectToAccessPoint(AccessPoint accessPoint);
        void RequestClientNetworks(ObservableCollection<Network> availableNetworks);
        void ConnectToClientNetwork(string networkSsid, string password);
        void DisconnectFromClientNetwork(string networkSsid);
        void Disconnect();
    }
}
