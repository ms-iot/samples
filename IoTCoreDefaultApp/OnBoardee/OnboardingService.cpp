// Copyright (c) Microsoft. All rights reserved.


#include "pch.h"
#include "OnboardingService.h"

using namespace concurrency;
using namespace OnBoardee;
using namespace org::alljoyn;
using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Devices::AllJoyn;
using namespace Windows::Devices::WiFi;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Foundation;
using namespace Windows::Networking::Connectivity;
using namespace Windows::Security::Credentials;
using namespace Windows::ApplicationModel::Background;
using namespace Windows::System::Threading;

OnboardingService::OnboardingService()
    : m_busAttachment(nullptr)
    , m_producer(nullptr)
    , m_wifiAdapter(nullptr)
    , m_personalSSID(nullptr)
    , m_personalPassphrase(nullptr)
    , m_state(OnBoardingState::NotConfigured)
{
}

OnboardingService::~OnboardingService()
{
}

void OnboardingService::Initialize()
{
    m_config.Init("OnboardingConfig.xml");

    m_wifiAdapter = this->GetWiFiAdapter();

    auto profiles = NetworkInformation::GetConnectionProfiles();
    for (auto profile : profiles)
    {
        if (profile->IsWlanConnectionProfile)
        {
            AutoLock lock(&m_stateLock, true);
            m_state = OnBoardingState::ConfiguredValidated;
            break;
        }
    }

    m_accessPoint = ref new OnboardingAccessPoint(m_config.SSID(), m_config.Password());
    if (m_state != OnBoardingState::ConfiguredValidated)
    {
        m_accessPoint->Start();
    }

    m_busAttachment = ref new AllJoynBusAttachment();
    m_busAttachment->AboutData->DefaultDescription = m_config.DefaultDescription();
    m_busAttachment->AboutData->DefaultManufacturer = m_config.DefaultManufacturer();

    m_producer = ref new OnboardingProducer(m_busAttachment);
    m_producer->Service = this;
    m_producer->Start();
}

void OnboardingService::Shutdown()
{
    if (nullptr != m_producer)
    {
        m_producer->Stop();
        m_producer = nullptr;
    }

    if (nullptr != m_accessPoint)
    {
        m_accessPoint->Stop();
        m_accessPoint = nullptr;
    }
}

WiFiAdapter^ OnboardingService::GetWiFiAdapter()
{
    return create_task(WiFiAdapter::RequestAccessAsync()).then([this](WiFiAccessStatus access) -> IAsyncOperation<DeviceInformationCollection^>^
    {
        if (access != WiFiAccessStatus::Allowed)
        {
            return create_async([]() -> DeviceInformationCollection^ { return nullptr; });
        }
        return DeviceInformation::FindAllAsync(WiFiAdapter::GetDeviceSelector());

    }).then([this] (DeviceInformationCollection^ adapters) -> IAsyncOperation<WiFiAdapter^>^
    {
        if (adapters == nullptr || adapters->Size == 0)
        {
            return create_async([]() -> WiFiAdapter^ { return nullptr; });
        }
        return WiFiAdapter::FromIdAsync(adapters->GetAt(0)->Id);

    }).get();
}

void OnboardingService::ConnectToNetwork(WiFiAvailableNetwork^ network)
{
    IAsyncOperation<WiFiConnectionResult^>^ connectOperation;
    if (network->SecuritySettings->NetworkAuthenticationType == NetworkAuthenticationType::Open80211)
    {
        connectOperation = m_wifiAdapter->ConnectAsync(network, WiFiReconnectionKind::Automatic);
    }
    else
    {
        auto credential = ref new Windows::Security::Credentials::PasswordCredential();
        credential->Password = m_personalPassphrase;
        connectOperation = m_wifiAdapter->ConnectAsync(network, WiFiReconnectionKind::Automatic, credential);
    }

    // lock scope
    {
        AutoLock lock(&m_stateLock, true);
        m_state = OnBoardingState::ConfiguredValidating;
    }
    

    create_task(connectOperation).then([this](WiFiConnectionResult^ result)
    {
        AutoLock lock(&m_stateLock, true);

        if (result->ConnectionStatus == WiFiConnectionStatus::Success)
        {
            m_error = OnBoardingError::Validated;
            m_errorMessage = nullptr;
            m_state = OnBoardingState::ConfiguredValidated;
        }
        else
        {
            m_state = OnBoardingState::ConfiguredError;

            switch (result->ConnectionStatus)
            {
            case WiFiConnectionStatus::AccessRevoked:
                {
                    m_error = OnBoardingError::Unauthorized;
                    m_errorMessage = "AccessRevoked";
                }
                break;
            case WiFiConnectionStatus::InvalidCredential:
                {
                    m_error = OnBoardingError::Unauthorized;
                    m_errorMessage = "InvalidCredential";
                }
                break;
            case WiFiConnectionStatus::NetworkNotAvailable:
                {
                    m_error = OnBoardingError::ErrorMessage;
                    m_errorMessage = "NetworkNotAvailable";
                }
                break;
            case WiFiConnectionStatus::Timeout:
                {
                    m_error = OnBoardingError::ErrorMessage;
                    m_errorMessage = "Timeout";
                }
                break;
            case WiFiConnectionStatus::UnspecifiedFailure:
                {
                    m_error = OnBoardingError::ErrorMessage;
                    m_errorMessage = "UnspecifiedFailure";
                }
                break;
            case WiFiConnectionStatus::UnsupportedAuthenticationProtocol:
                {
                    m_error = OnBoardingError::UnsupportedProtocol;
                    m_errorMessage = "UnsupportedAuthenticationProtocol";
                }
                break;
            default:
                break;
            }
        }
    });
}

Windows::Foundation::IAsyncOperation<OnboardingConfigureWifiResult^>^ OnboardingService::ConfigureWifiAsync(AllJoynMessageInfo ^ info, Platform::String ^ interfaceMemberSSID, Platform::String ^ interfaceMemberPassphrase, int16 interfaceMemberAuthType)
{
    return create_async([this, interfaceMemberSSID, interfaceMemberPassphrase, interfaceMemberAuthType]() -> OnboardingConfigureWifiResult^ {
        
        // make sure a valid value is provided for auth type
        if (interfaceMemberAuthType < WPA2_AUTO || interfaceMemberAuthType > WPS)
        {
            return OnboardingConfigureWifiResult::CreateFailureResult(Windows::Devices::AllJoyn::AllJoynStatus::InvalidArgument3);
        }

        // May want to switch this to concurrent mode if possible:
        // "Concurrent step used to validate the personal AP connection. In this case, the Onboarder application must wait for the 
        // ConnectionResult signal to arrive over the AllJoyn session established over the SoftAP link."

        this->m_personalSSID = interfaceMemberSSID;
        this->m_personalPassphrase = interfaceMemberPassphrase;
        this->m_authType = static_cast<AuthType>(interfaceMemberAuthType);

        AutoLock lock(&m_stateLock, true);
        m_state = OnBoardingState::ConfiguredNotValidated;

        // Status "1" indicates the SoftAP will remain available until the Connect method is invoked
        return OnboardingConfigureWifiResult::CreateSuccessResult(1);
    });
}

Windows::Foundation::IAsyncOperation<OnboardingConnectResult^>^ OnboardingService::ConnectAsync(AllJoynMessageInfo ^ info)
{
    return create_async([this]() -> OnboardingConnectResult^ {

        // find the network with the specified SSID
        for (auto network : m_wifiAdapter->NetworkReport->AvailableNetworks)
        {
            if (network->Ssid == m_personalSSID)
            {
                if (m_accessPoint != nullptr)
                {
                    this->m_accessPoint->Stop();
                }
                this->ConnectToNetwork(network);
            }
        }
        return OnboardingConnectResult::CreateSuccessResult();
    });
}

Windows::Foundation::IAsyncOperation<OnboardingOffboardResult^>^ OnboardingService::OffboardAsync(AllJoynMessageInfo ^ info)
{
    return create_async([this]() -> OnboardingOffboardResult^ {
        
        AutoLock lock(&m_stateLock, true);

        if (m_state == OnBoardingState::ConfiguredValidated || m_state == OnBoardingState::ConfiguredValidating)
        {
            m_wifiAdapter->Disconnect();
        }       
        m_state = OnBoardingState::NotConfigured;

        m_personalSSID = nullptr;
        m_personalPassphrase = nullptr;
        m_authType = AuthType::Any;

        if (m_accessPoint != nullptr)
        {
            m_accessPoint->Start();
        }        

        return OnboardingOffboardResult::CreateSuccessResult();
    });
}

Windows::Foundation::IAsyncOperation<OnboardingGetScanInfoResult^>^ OnboardingService::GetScanInfoAsync(AllJoynMessageInfo ^ info)
{       
    return create_async([this]() ->OnboardingGetScanInfoResult ^
    {
        create_task(m_wifiAdapter->ScanAsync()).wait();

        auto ajAvailableNetworks = ref new Vector<OnboardingScanListItem^>();
        for (auto network : m_wifiAdapter->NetworkReport->AvailableNetworks)
        {
            auto ajNetwork = ref new OnboardingScanListItem();
            ajNetwork->Value1 = network->Ssid;
            
            switch (network->SecuritySettings->NetworkAuthenticationType)
            {
            case NetworkAuthenticationType::Open80211:
                {
                    ajNetwork->Value2 = AuthType::Open;
                }
                break;
            case NetworkAuthenticationType::Wpa:
                {
                    ajNetwork->Value2 = AuthType::WPA_AUTO;
                }
                break;
            default:
                {
                    ajNetwork->Value2 = AuthType::Any;
                }
            }
            ajAvailableNetworks->Append(ajNetwork);
        }

        return OnboardingGetScanInfoResult::CreateSuccessResult(0, ajAvailableNetworks);
    });
}

Windows::Foundation::IAsyncOperation<OnboardingGetVersionResult^>^ OnboardingService::GetVersionAsync(AllJoynMessageInfo ^ info)
{
    return create_async([this]() -> OnboardingGetVersionResult^ {
            OnboardingGetVersionResult^ result = ref new OnboardingGetVersionResult();

            return result;
    });
}

Windows::Foundation::IAsyncOperation<OnboardingGetStateResult^>^ OnboardingService::GetStateAsync(AllJoynMessageInfo ^ info)
{
    return create_async([this]() -> OnboardingGetStateResult^ {
        AutoLock(&m_stateLock, true);
        return OnboardingGetStateResult::CreateSuccessResult(static_cast<short>(m_state));
    });
}

Windows::Foundation::IAsyncOperation<OnboardingGetLastErrorResult^>^ OnboardingService::GetLastErrorAsync(AllJoynMessageInfo ^ info)
{
    return create_async([this]() -> OnboardingGetLastErrorResult^ {
        AutoLock(&m_stateLock, true);

        auto lastError = ref new OnboardingLastError();
        lastError->Value1 = static_cast<short>(m_error);
        lastError->Value2 = m_errorMessage;

        return OnboardingGetLastErrorResult::CreateSuccessResult(lastError);
    });
}