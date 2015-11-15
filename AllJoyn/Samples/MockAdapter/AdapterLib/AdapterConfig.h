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
#include <ppltasks.h>
#include <string>

using namespace concurrency;
using namespace Windows::Data::Xml::Dom;
using namespace Windows::Storage;
using namespace Windows::ApplicationModel;
using namespace Platform;

class AdapterConfig
{
public:
    AdapterConfig();
    virtual ~AdapterConfig();

    //******************************************************************************************************
    //
    //  If the configuration file exists, then load it to the memory. Otherwise, create a configuration file
    //	with the default settings.
    //
    //  returns: S_OK on success,
    //           HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) if the file is missing.
    //           Other HRESULT on fail
    //
    //******************************************************************************************************
    HRESULT Init();

    //*****************************************************************************************************
    //
    //	Writes the given XML configuration into the adapter configuration file.
    //
    //	xmlString: A complete XML configuration
    //
    //	returns: S_OK on success, other HRESULT on fail
    //
    //*****************************************************************************************************
    HRESULT SetConfig(_In_ String^ xmlString);

    //*****************************************************************************************************
    //
    //	Reads the adapter configurations into an XML string.
    //
    //	xmlString:	Returned adapter configuration in string format.
    //
    //	returns: S_OK on success, other HRESULT on fail
    //
    //*****************************************************************************************************
    HRESULT GetConfig(_Out_ String^* xmlString);

private:
    //******************************************************************************************************
    //
    //  Loads adapter configuration from the xml file.
    //
    //  returns: S_OK on success, other HRESULT on fail
    //
    //******************************************************************************************************
    HRESULT fromFile();

    //******************************************************************************************************
    //
    //  Loads this configuration object from the specified String.
    //
    //  xmlString:  A complete XML configuration.
    //
    //  returns: S_OK on success, other HRESULT on fail
    //
    //******************************************************************************************************
    HRESULT fromString(_In_ String^ xmlString);

    //******************************************************************************************************
    //
    //  Saves the current in memory XML file to disk
    //
    //  returns: S_OK on success, other HRESULT on fail
    //
    //******************************************************************************************************
    HRESULT toFile();

    //******************************************************************************************************
    //
    //  Converts the in memory XML configuration to a string
    //
    //  xmlString:  The returned string-ized version of the current XML document
    //
    //  returns: S_OK on success, other HRESULT on fail
    //
    //******************************************************************************************************
    HRESULT toString(_Out_ String^* xmlString);

    //******************************************************************************************************
    //
    //	Loads default adapter configuration xml.
    //
    //	returns: S_OK on success, other HRESULT on fail
    //
    //******************************************************************************************************
    HRESULT initXml();

    //******************************************************************************************************
    //
    //	Checks if the configuration file exists.
    //
    //******************************************************************************************************
    bool isConfigFilePresent();

    //
    //  The current XML Document
    //
    XmlDocument^ m_pXmlDocument;

};