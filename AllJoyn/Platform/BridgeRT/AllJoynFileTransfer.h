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
#include <string>

#include <alljoyn_c/BusAttachment.h>
#include <alljoyn_c/InterfaceDescription.h>
#include <alljoyn_c/BusObject.h>
#include <alljoyn_c/Message.h>

const size_t MAX_GUID_STRING_LEN = 39;   // 38 chars + terminator null

namespace BridgeRT
{
    class ConfigManager;

	class AllJoynFileTransfer
	{
	public:

		AllJoynFileTransfer();
		virtual ~AllJoynFileTransfer();

		QStatus Initialize(_In_ alljoyn_busattachment* bus, _In_z_ const char *objectPath, _In_ ConfigManager *configManager);
		void Destroy();
		inline alljoyn_busobject GetBusObject()
		{
			return m_allJoynBusObject;
		}
        inline alljoyn_interfacedescription GetInterface()
        {
            return m_interfaceDescription;
        }

        ///*******************************************************************************************************************************
        ///
        /// This method is when after writing a configuration file to a temporary file in the application storage folder.
        /// The "appRelativeFileName" specified here is the name of the temporary file that was saved to disk.
        /// The file is relative to "ApplicationData::Current->LocalFolder"
        ///
        ///*******************************************************************************************************************************
        virtual HRESULT PostFileWriteAction(_In_ std::wstring &appRelativeFileName, _Out_ HANDLE *finalEvent) = 0;

        ///*******************************************************************************************************************************
        ///
        /// This method is called when a request is made to read a configuration file associated with this app.  The caller must
        /// return the location of an app storage file that contains the requested configuration data.
        /// The "appRelativeFileName", returned here, must be relative to "ApplicationData::Current->LocalFolder"
        ///
        ///*******************************************************************************************************************************
        virtual HRESULT PreFileReadAction(_Out_ std::wstring &appRelativeFileName) = 0;


        virtual HRESULT PostFileReadAction(void) = 0;

		void EndTransfer();
		static AllJoynFileTransfer *GetInstance(_In_ alljoyn_busobject busObject);

		static void AJ_CALL StartChunkWrite(_In_ alljoyn_busobject busObject, _In_ const alljoyn_interfacedescription_member* member, _In_ alljoyn_message msg);
		static void AJ_CALL WriteNextChunk(_In_ alljoyn_busobject busObject, _In_ const alljoyn_interfacedescription_member* member, _In_ alljoyn_message msg);

		static void AJ_CALL StartChunkRead(_In_ alljoyn_busobject busObject, _In_ const alljoyn_interfacedescription_member* member, _In_ alljoyn_message msg);
		static void AJ_CALL ReadNextChunk(_In_ alljoyn_busobject busObject, _In_ const alljoyn_interfacedescription_member* member, _In_ alljoyn_message msg);

	private:
		alljoyn_busobject m_allJoynBusObject;
		alljoyn_busattachment *m_bus;
		alljoyn_interfacedescription m_interfaceDescription;

		volatile LONG m_lockCount;
		bool m_registeredOnAllJoyn;
		bool m_endOfTransfer;
		uint32_t m_fileSize;
		uint32_t m_nbOfBytesTransfered;

		// ULONG used to identified file transfer
		//
		// Note that this not by any means a secured identification (AuthN)
		// security should and must be handle at AllJoyn level when AllJoyn will be ready for that
		static ULONG s_transferId;

		std::wstring m_tempFileName;
        std::wstring m_tempAppRelativeFileName;

        HRESULT CreateTempFile();
		void DeleteTempFile();
	};
}
