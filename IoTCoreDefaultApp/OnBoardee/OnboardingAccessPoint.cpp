// Copyright (c) Microsoft. All rights reserved.


#include "pch.h"
#include "OnboardingAccessPoint.h"

using namespace Platform;
using namespace Windows::Devices::WiFiDirect;
using namespace Windows::Security::Credentials;
using namespace Windows::Foundation;

namespace OnBoardee
{
    OnboardingAccessPoint::OnboardingAccessPoint(String^ SSID, String^ password) 
        : m_active(false)
    {
        // Begin advertising for legacy clients
        m_publisher = ref new WiFiDirectAdvertisementPublisher();

        // Note: If this flag is not set, the legacy parameters are ignored
        m_publisher->Advertisement->IsAutonomousGroupOwnerEnabled = true;

        // Setup Advertisement to use a custom SSID and WPA2 passphrase
        m_publisher->Advertisement->LegacySettings->IsEnabled = true;
        m_publisher->Advertisement->LegacySettings->Ssid = SSID;
        
        auto passphrase = ref new PasswordCredential();
        passphrase->Password = password;
        m_publisher->Advertisement->LegacySettings->Passphrase = passphrase;

        m_publisher->StatusChanged += ref new TypedEventHandler<WiFiDirectAdvertisementPublisher^, WiFiDirectAdvertisementPublisherStatusChangedEventArgs^>(this, &OnboardingAccessPoint::OnAdvertisementStatusChanged);
    }

    void OnboardingAccessPoint::Start()
    {
        if (!m_active)
        {
            m_publisher->Start();
        }
    }

    void OnboardingAccessPoint::Stop()
    {
        if (m_active)
        {
            m_publisher->Stop();
        }
    }

    OnboardingAccessPoint::~OnboardingAccessPoint()
    {
        this->Stop();
        m_publisher = nullptr;
    }

    void OnboardingAccessPoint::OnAdvertisementStatusChanged(WiFiDirectAdvertisementPublisher^ sender, WiFiDirectAdvertisementPublisherStatusChangedEventArgs^ args)
    {
        if (args->Status == WiFiDirectAdvertisementPublisherStatus::Started)
        {
            m_active = true;
        }
        else if (args->Status == WiFiDirectAdvertisementPublisherStatus::Stopped)
        {
            m_active = false;
        }
        else if (args->Status == WiFiDirectAdvertisementPublisherStatus::Aborted)
        {
            m_active = false;
            if (args->Error == WiFiDirectError::RadioNotAvailable)
            {
                /* The radio was turned off. */
            }
            else if (args->Error == WiFiDirectError::ResourceInUse)
            {
                /* The stack couldn't accept any additional IEs.  Need to turn off any other applications which could be advertising */
            }
        }
    }
}