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

#include "pch.h"
#include <ppltasks.h>

using namespace concurrency;
using namespace Windows::Data::Xml::Dom;
using namespace Windows::Storage;
using namespace Windows::ApplicationModel;
using namespace Platform;

struct DsbObjectConfig
{
    String^ id;
    bool bVisible;
    String^ description;

    bool operator==(const DsbObjectConfig& rhs)
    {
        if (&rhs == this)
        {
            return true;
        }

        return ((id == rhs.id) &&
            (bVisible == rhs.bVisible) &&
            (description == rhs.description));
    }

    bool operator!=(const DsbObjectConfig& rhs)
    {
        if (&rhs == this)
        {
            return false;
        }

        return ((id != rhs.id) ||
            (bVisible != rhs.bVisible) ||
            (description != rhs.description));
    }
};


class BridgeConfig
{
public:
    BridgeConfig();
    virtual ~BridgeConfig();

    //******************************************************************************************************
    //
    //  Initialize this object
    //
    //  pFileName:			Relative path and filename to initialize from, or null.
    //
    //  faileIfNotPresent:  If a file name is specified and the file is not detected, fail, instead
    //                      of calling the default initialization method.  Ignored if filename is not
    //                      specified
    //
    //  If the file specified by pFileName is not found, this routine will try to create a default
    //  file with the specified name at the specified path.  If the file specified by pFileName is found,
    //  the file will be schema validated and loaded.  In either case, the filename is cached within
    //  this object and does not need to be specified in the ToFile function when the configuration is updated.
    //
    //  If the file specified by pFileName is null, a later call to FromFile or FromString is required.
    //
    //  returns: S_OK on success,
    //           HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) if the file is missing.
    //           Other HRESULT on fail
    //
    //******************************************************************************************************
    HRESULT Init(_In_ String^ pFileName, bool failIfNotPresent = false);

    //******************************************************************************************************
    //
    //  Loads this configuration object from the specified XML file using the
    //
    //  pFileName:   Fully qualified path and filename
    //
    //  returns: S_OK on success, other HRESULT on fail
    //
    //******************************************************************************************************
    HRESULT FromFile(_In_ String^ pFileName);

    //******************************************************************************************************
    //
    //  Loads this configuration object from the specified String.
    //
    //  xmlString:  A complete XML configuration.
    //
    //  returns: S_OK on success, other HRESULT on fail
    //
    //******************************************************************************************************
    HRESULT FromString(_In_ String^ xmlString);

    //******************************************************************************************************
    //
    //  Saves the current in memory XML file to disk
    //
    //  pFileName:  The file name to save this configuration object to.  Can be null if filename was
    //              specified by earlier call to Init.
    //
    //  returns: S_OK on success, other HRESULT on fail
    //
    //******************************************************************************************************
    HRESULT ToFile(_In_ String^ pFileName = nullptr);

    //******************************************************************************************************
    //
    //  Converts the in memory XML configuration to a string
    //
    //  xmlString:  The returned string-ized version of the current XML document
    //
    //  returns: S_OK on success, other HRESULT on fail
    //
    //******************************************************************************************************
    HRESULT ToString(_Out_ String^* xmlString);

    //******************************************************************************************************
    //
    //	Find the object with the given id.
    //
    //	id: ID of the device object
    //
    //	object: corresponding device object returned from the xml
    //
    //	returns: S_OK on success, other HRESULT on fail
    //
    //******************************************************************************************************
    HRESULT FindObject(_In_ String^ id, _Out_ DsbObjectConfig& object);

    //******************************************************************************************************
    //
    //	Add object into the xml configurations.
    //
    //	object: the device object to be added
    //
    //	returns: S_OK on success, other HRESULT on fail
    //
    //******************************************************************************************************
    HRESULT AddObject(_In_ const DsbObjectConfig& object);

    //******************************************************************************************************
    //
    //	Remove the object with the given id.
    //
    //	id: ID of the device object
    //
    //	returns: S_OK on success, other HRESULT on fail
    //
    //******************************************************************************************************
    HRESULT RemoveObject(_In_ String^ id);


    //******************************************************************************************************
    //
    //	Merge an input xml document into the current in-memory xml document
    //
    //	src: source xml document
    //
    //	returns: S_OK on success, other HRESULT on fail
    //
    //******************************************************************************************************
    HRESULT MergeFrom(_In_ BridgeConfig& src);


    //******************************************************************************************************
    //
    //	Returns App Local relative file name managed by this Bridge Configuration XML File
    //
    //	returns: null if not initialized, full Path and File Name of Configuration file managed by this
    //           instance
    //
    //******************************************************************************************************
    String^ FileName()
    {
        return m_fileName;
    }

	//******************************************************************************************************
	//
	//	Returns the Security KeyX for configuration/bridge (if configured)
	//
	//	returns: empty string if not configured, a KeyX value if present
	//
	//******************************************************************************************************
    String^ BridgeKeyX();

    //******************************************************************************************************
    //
    //	Returns the Security KeyX for device (if configured)
    //
    //	returns: empty string if not configured, a KeyX value if present
    //
    //******************************************************************************************************
    String^ DeviceKeyX();

    //******************************************************************************************************
	//
	//	Returns the Security Username for device (if configured)
	//
	//	returns: empty string if not configured, a username value if present
	//
	//******************************************************************************************************
	String^ DeviceUsername();

	//******************************************************************************************************
	//
	//	Returns the Security Password for device (if configured)
	//
	//	returns: empty string if not configured, a password value if present
	//
	//******************************************************************************************************
	String^ DevicePassword();

    //******************************************************************************************************
    //
    //	Returns the Security ECDHE ECDSA private key for device (if configured)
    //
    //	returns: empty string if not configured, a ECDHE ECDSA private key value if present
    //
    //******************************************************************************************************
    String^ DeviceEcdheEcdsaPrivateKey();

    //******************************************************************************************************
    //
    //	Returns the Security ECDHE ECDSA CERT chain for device (if configured)
    //
    //	returns: empty string if not configured, a ECDHE ECDSA CERT chain value if present
    //
    //******************************************************************************************************
    String^ DeviceEcdheEcdsaCertChain();

    //******************************************************************************************************
    //
    //	Returns the default visibility to apply for objects found by the adapter
    //
    //	returns: true if new objects found by the adapter should appear on the alljoyn bus be default,
    //			 false if not set or if configured as false.
    //
    //******************************************************************************************************
    bool DefaultVisibility();


private:
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
    //  Helper method that Adds a Node to the XML document with the specified text value under the
    //  specified parent node.
    //
    //  nodeName:    Name of node to write
    //  text:        Text value to set for node
    //  parentNode:  pointer to parent node to add this node to
    //
    //  returns: S_OK on success, other HRESULT on fail
    //
    //******************************************************************************************************
    HRESULT addTextNode(_In_ String^ nodeName, _In_ String^ text, _In_ XmlElement^ parentNode);

    //******************************************************************************************************
    //
    //  Helper method for handling boolean XML data vailes
    //
    //  varValue:   Variant value read from the XML file.
    //  bool:       The boolean value read from the file.
    //
    //  returns: S_OK on success, other HRESULT on fail
    //
    //******************************************************************************************************
    HRESULT convertXsBooleanToBool(_In_ String^ varValue, _Out_ bool &bValue);

    //******************************************************************************************************
    //
    //  Reads the text value of the specified node (if found)
    //
    //  nodeName:       Name of node to write
    //  parentNode:     pointer to parent node to add this node to
    //  bIsOptional:    true if the specified node is optional (will return an empty string and not fail)
    //                  false if the specified node is required and missing (will return an error);
    //  text:           The text value of the requested node
    //
    //  returns: S_OK on success, other HRESULT on fail
    //
    //******************************************************************************************************
    HRESULT readTextNode(
        _In_ String^ nodeName,
        _In_ XmlElement^ parentNode,
        _In_ bool bIsOptional,
        _Out_ String^* text);

    //******************************************************************************************************
    //
    //	Get the value for the specified attribute from the specified node
    //
    //	pNode: xml node that contains the attribute
    //
    //	attributeName: attribute to get its value
    //
    //	result:	returned attribute value
    //
    //	returns: S_OK on success, other HRESULT on fail
    //
    //******************************************************************************************************
    HRESULT getAttributeValue(
        _In_ IXmlNode^ pNode,
        _In_ String^ attributeName,
        _Out_ String^* result);

    //******************************************************************************************************
    //
    //	Checks if the specified file exists.
    //
    //******************************************************************************************************
    bool isFilePresent(_In_ StorageFolder^ appFolder, _In_ String^ pFileName);


    //******************************************************************************************************
    //
    //	Merge source node into the current in-memory xml document
    //
    //	pCurrSrcNode: source object node to be merged into the xml document
    //
    //	pDestDocElem: destination objects node list to merge the specified node to.
    //
    //	returns: S_OK on success, other HRESULT on fail
    //
    //******************************************************************************************************
    HRESULT MergeObjectNode(IXmlNode^ pCurrSrcNode, XmlElement^ pDestDocElem);


    //******************************************************************************************************
    //
    //	Internal method that merges settings from source document to the locally persisted copy
    //
    //  pSrcDoc - Source document to merge settings from
    //
    //******************************************************************************************************
    HRESULT MergeSettingsNode(_In_ XmlDocument^ pSrcDoc);

    //******************************************************************************************************
    //
    //	Returns value of a specific credential item, e.g.: PIN, USERNAME, PASSWORD... (if configured)
    //
    //	returns: empty string if not configured, a pin value if present
    //
    //******************************************************************************************************
    String^ GetCredentialValue(LPCWSTR xmlPath);

    //
    //  The current XML Document
    //
    XmlDocument^ m_pXmlDocument;

    //
    //  Relative Path and File Name
    //
    String^ m_fileName;
};