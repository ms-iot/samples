//
// Copyright (c) 2015, Microsoft Corporation
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
// SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
// IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//

#pragma once
#include <alljoyn_c/BusAttachment.h>
#include <alljoyn_c/BusObject.h>
#include <alljoyn_c/AboutData.h>
#include <alljoyn_c/AboutObj.h>

#include "Misc.h"

namespace BridgeRT
{
    class BridgeAuthHandler
    {
    public:
        BridgeAuthHandler();
        ~BridgeAuthHandler();

		inline QStatus InitializeWithKeyXAuthentication(_In_ alljoyn_busattachment bus, _In_ Platform::String^ keyX)
		{
			return InitializeWithAllAuthenticationMethods(bus, keyX, nullptr, nullptr, nullptr, nullptr);
		}
        inline QStatus InitializeWithCredentialAuthentication(_In_ alljoyn_busattachment bus, _In_ Platform::String^ username, _In_ Platform::String^ password)
        {
            return InitializeWithAllAuthenticationMethods(bus, nullptr, username, password, nullptr, nullptr);
        }
        inline QStatus InitializeWithEcdheEcdsaAuthentication(_In_ alljoyn_busattachment bus, _In_ Platform::String^ ecdheEcdsaPrivateKey, _In_ Platform::String^ ecdheEcdsaCertChain)
        {
            return InitializeWithAllAuthenticationMethods(bus, nullptr, nullptr, nullptr, ecdheEcdsaPrivateKey, ecdheEcdsaCertChain);
        }
        void ResetAccess(_In_ std::string remoteName);

        QStatus InitializeWithAllAuthenticationMethods(_In_ alljoyn_busattachment bus,
            _In_ Platform::String^ keyx,
            _In_ Platform::String^ username, _In_ Platform::String^ password,
            _In_ Platform::String^ ecdheEcdsaPrivateKey, _In_ Platform::String^ ecdheEcdsaCertChain);

        void ShutDown();

    private:
        alljoyn_busattachment m_busAttachment;
        alljoyn_authlistener m_authListener;
        std::string m_keyx;
		std::string m_username;
		std::string m_password;
        std::string m_ecdheEcdsaPrivateKey;
        std::string m_ecdheEcdsaCertChain;

		static QStatus AJ_CALL ReqCredentialsAsync(
            void* context,
            alljoyn_authlistener listener,
            const char* authMechanism,
            const char* authPeer,
            uint16_t authCount,
            const char* userId,
            uint16_t credMask,
            void* authContext);

        static void AJ_CALL AuthComplete(
            const void* context,
            const char* authMechanism,
            const char* authPeer,
            QCC_BOOL success);

        static QStatus AJ_CALL VerifyCredentialsAsync(
            const void*context,
            alljoyn_authlistener listener,
            const char* authMechanism,
            const char* authPeer,
            const alljoyn_credentials creds,
            void*authContext);
    };
}
