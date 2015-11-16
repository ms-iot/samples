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

#include "pch.h"
#include "BridgeConfig.h"

#pragma warning(disable:4127)

LPCWSTR BRIDGE_CONFIG_PATH = L"/BridgeConfig";
LPCWSTR OBJECTS_NODE_PATH = L"/BridgeConfig/AdapterDevices";

LPCWSTR ALLJOYN_OBJECT_PATH_PREFIX = L"/BridgeConfig/AdapterDevices/Device[@Id=\"";
LPCWSTR ALLJOYN_OBJECT_PATH_SUFFIX = L"\"]";
LPCWSTR OBJECT_NODE = L"Device";
LPCWSTR VISIBLE_ATTR = L"Visible";
LPCWSTR OBJECT_ID_ATTR = L"Id";
LPCWSTR DESC_NODE = L"Desc";
LPCWSTR ROOT_NODE = L"./";
LPCWSTR TEXT_NODE = L"/text()";

LPCWSTR SETTINGS_NODE_PATH = L"/BridgeConfig/Settings";
LPCWSTR SETTINGS_NODE_NAME = L"Settings";
LPCWSTR SETTINGS_BRIDGE_KEYX_PATH = L"/BridgeConfig/Settings/Bridge/KEYX";
LPCWSTR SETTINGS_DEVICE_DEFAULT_VIS_PATH = L"/BridgeConfig/Settings/Device/DefaultVisibility";
LPCWSTR SETTINGS_DEVICE_KEYX_PATH = L"/BridgeConfig/Settings/Device/KEYX";
LPCWSTR SETTINGS_DEVICE_USERNAME_PATH = L"/BridgeConfig/Settings/Device/USERNAME";
LPCWSTR SETTINGS_DEVICE_PASSWORD_PATH = L"/BridgeConfig/Settings/Device/PASSWORD";
LPCWSTR SETTINGS_DEVICE_ECDHE_ECDSA_PRIVATEKEY_PATH = L"/BridgeConfig/Settings/Device/ECDHEECDSAPRIVATEKEY";
LPCWSTR SETTINGS_DEVICE_ECDHE_ECDSA_CERTCHAIN_PATH = L"/BridgeConfig/Settings/Device/ECDHEECDSACERTCHAIN";

const WCHAR DEFAULT_BRIDGE_CFG_XML[] =
L"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
L"<BridgeConfig>\n"
L"  <Settings>\n"
L"    <Bridge>\n"
L"       <KEYX></KEYX>\n"
L"    </Bridge>\n"
L"    <Device>\n"
L"       <DefaultVisibility>true</DefaultVisibility>\n"
L"       <KEYX></KEYX>\n"
L"       <USERNAME></USERNAME>\n"
L"       <PASSWORD></PASSWORD>\n"
L"       <ECDHEECDSAPRIVATEKEY></ECDHEECDSAPRIVATEKEY>\n"
L"       <ECDHEECDSACERTCHAIN></ECDHEECDSACERTCHAIN>\n"
L"    </Device>\n"
L"  </Settings>\n"
L"  <AdapterDevices>\n"
L"  </AdapterDevices>\n"
L"</BridgeConfig>\n";

BridgeConfig::BridgeConfig()
{
    m_pXmlDocument = ref new XmlDocument();
}

BridgeConfig::~BridgeConfig()
{
}

_Use_decl_annotations_
HRESULT
BridgeConfig::Init(String^ pFileName, bool failIfNotPresent)
{
    HRESULT hr = S_OK;

    // If a filename was specified, try to initialize the configuration document from the specified name
    if (pFileName != nullptr)
    {
        //  Does the specified file exist?
        if (!isFilePresent(ApplicationData::Current->LocalFolder, pFileName))
        {
            //Not present. see if it is present in appxpackage
            if(isFilePresent(Package::Current->InstalledLocation, pFileName))
            {
                //copy the file
                auto srcFolder = Package::Current->InstalledLocation;
                auto destFolder = ApplicationData::Current->LocalFolder;

                try
                {
                    create_task(srcFolder->GetFileAsync(pFileName)).then([destFolder](StorageFile^ file)
                    {
                        return file->CopyAsync(destFolder);
                    }).wait();
                }
                catch (Exception^)
                {
                    //ignore
                }
            }
        }

        if (!isFilePresent(ApplicationData::Current->LocalFolder, pFileName))
        {
            // The file doesn't exist, so return a failure if caller wanted to fail, or initialize a default
            if (failIfNotPresent)
            {
                hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
                goto CleanUp;
            }

            CHK_HR(initXml());
            CHK_HR(ToFile(pFileName));
        }
        // Otherwise the file is loaded
        else
        {
            // Try to read file
            CHK_HR(FromFile(pFileName));
        }
    }

CleanUp:
    return hr;
}

_Use_decl_annotations_
HRESULT
BridgeConfig::FromFile(String^ pFileName)
{
    HRESULT hr = S_OK;
    task<StorageFile^> getConfigFileTask;
    StorageFolder^ appFolder = ApplicationData::Current->LocalFolder;

    // Make sure that a filename is (or was) specified for this configuration file
    if ((pFileName == nullptr))
    {
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    if (m_pXmlDocument == nullptr)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_STATE);
        goto CleanUp;
    }

    // otherwise, the input is valid so load the XML
    // Update the file name and convert to a variant
    m_fileName = pFileName;

    // Load the XML file
    try
    {
        getConfigFileTask = create_task(appFolder->GetFileAsync(m_fileName));
    }
    catch (Platform::Exception^ ex)
    {
        hr = ex->HResult;
        goto CleanUp;
    }

    getConfigFileTask
        .then([this](StorageFile^ sFile)
    {
        return  XmlDocument::LoadFromFileAsync(sFile);
    })
        .then([this](XmlDocument^ doc)
    {
        m_pXmlDocument = doc;
    })
        .then([&](task<void> checkExceptionTask)
    {
        try
        {
            checkExceptionTask.get();
        }
        catch (Platform::Exception^ ex)
        {
            hr = ex->HResult;
        }
    })
        .wait();

CleanUp:
    return hr;
}

_Use_decl_annotations_
HRESULT
BridgeConfig::FromString(String^ xmlString)
{
    HRESULT hr = S_OK;

    if (m_pXmlDocument == nullptr)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_STATE);
        goto CleanUp;
    }

    m_pXmlDocument->LoadXml(xmlString);

CleanUp:
    return hr;
}

_Use_decl_annotations_
HRESULT
BridgeConfig::ToFile(String^ pFileName)
{
    HRESULT hr = S_OK;
    task<StorageFile^> createConfigFileTask;
    StorageFolder^ appFolder = ApplicationData::Current->LocalFolder;

    // Make sure that a filename is (or was) specified for this configuration file
    if (((pFileName == nullptr) && (m_fileName->Length() == 0)))
    {
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    // If filename is specified then change it before saving
    if (pFileName != nullptr)
    {
        m_fileName = pFileName;
    }

    // Save/Update the configuration file
    try
    {
        createConfigFileTask = create_task(appFolder->CreateFileAsync(
            m_fileName,
            CreationCollisionOption::ReplaceExisting));
    }
    catch (Platform::Exception^ ex)
    {
        hr = ex->HResult;
        goto CleanUp;
    }

    createConfigFileTask
        .then([this](StorageFile^ sFile)
    {
        return m_pXmlDocument->SaveToFileAsync(sFile);
    })
        .then([&](task<void> checkExceptionTask)
    {
        try
        {
            checkExceptionTask.get();
        }
        catch (Platform::Exception^ ex)
        {
            hr = ex->HResult;
        }
    })
        .wait();

CleanUp:
    return hr;
}

_Use_decl_annotations_
HRESULT
BridgeConfig::ToString(String^* xmlString)
{
    HRESULT hr = S_OK;

    if (m_pXmlDocument == nullptr)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_STATE);
        goto CleanUp;
    }

    *(xmlString) = m_pXmlDocument->GetXml();

CleanUp:
    return hr;
}

HRESULT
BridgeConfig::initXml()
{
    HRESULT hr = S_OK;
    StringReference target(DEFAULT_BRIDGE_CFG_XML);
    m_pXmlDocument->LoadXml(target.GetString());

    return hr;
}

_Use_decl_annotations_
HRESULT
BridgeConfig::FindObject(String^ id, DsbObjectConfig& object)
{
    HRESULT hr = S_OK;
    String^ objectPathIdAttr;
    StringReference objectPathPrefix(ALLJOYN_OBJECT_PATH_PREFIX);
    StringReference objectPathSuffix(ALLJOYN_OBJECT_PATH_SUFFIX);
    StringReference visibleAttr(VISIBLE_ATTR);
    StringReference descNode(DESC_NODE);
    String^ visibleVal;
    IXmlNode^ pNode;
    XmlElement^ pElement;

    // Verify that Init has been called
    if (m_pXmlDocument == nullptr)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_STATE);
        goto CleanUp;
    }

    // Try to locate the specified bus object in the document
    objectPathIdAttr += objectPathPrefix;
    objectPathIdAttr += id;
    objectPathIdAttr += objectPathSuffix;
    pNode = m_pXmlDocument->SelectSingleNode(objectPathIdAttr);
    if (pNode == nullptr)
    {
        hr = S_FALSE;
        goto CleanUp;
    }

    // Object was found, so try to read the Visibility attribute
    pElement = (XmlElement^)pNode;
    visibleVal = pElement->GetAttribute(visibleAttr.GetString());
    if (visibleVal == nullptr)
    {
        hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
        goto CleanUp;
    }

    // Convert the variant to bool
    CHK_HR(convertXsBooleanToBool(visibleVal, object.bVisible));

    try
    {
        object.id = id;
    }
    catch (...)
    {
        hr = E_OUTOFMEMORY;
        goto CleanUp;
    }

    // The following nodes are optional.  S_FALSE may be returned, which is ok.
    CHK_HR(readTextNode(descNode, pElement, true, &(object.description)));

    // Force result to S_OK (in the event that the preceding optional nodes were S_FALSE)
    hr = S_OK;

CleanUp:
    return hr;
}

_Use_decl_annotations_
HRESULT
BridgeConfig::AddObject(const DsbObjectConfig& object)
{
    HRESULT hr = S_OK;
    StringReference objectsRoot(OBJECTS_NODE_PATH);
    StringReference visibleAttr(VISIBLE_ATTR);
    StringReference objNodeName(OBJECT_NODE);
    StringReference objIdAttr(OBJECT_ID_ATTR);
    StringReference descNode(DESC_NODE);

    String^ objIdVal = object.id;
    Boolean objVisibleVal = object.bVisible ? 1 : 0;

    IXmlNode^ pOutNode = nullptr;
    XmlElement^ pAllJoynObjectElem = nullptr;
    XmlElement^ pRootAllJoynObjListElem = nullptr;

    // Verify that Init has been called
    if (m_pXmlDocument == nullptr)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_STATE);
        goto CleanUp;
    }

    pRootAllJoynObjListElem = (XmlElement^)m_pXmlDocument->SelectSingleNode(objectsRoot);
    if (pRootAllJoynObjListElem == nullptr)
    {
        CHK_HR(initXml());
        pRootAllJoynObjListElem = (XmlElement^)m_pXmlDocument->SelectSingleNode(objectsRoot);
        if (pRootAllJoynObjListElem == nullptr)
        {
            hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
            goto CleanUp;
        }
    }

    // Create the AllJoyn Object Node and configure its attributes
    pAllJoynObjectElem = m_pXmlDocument->CreateElement(objNodeName.GetString());
    if (pAllJoynObjectElem == nullptr)
    {
        hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
        goto CleanUp;
    }

    pAllJoynObjectElem->SetAttribute(objIdAttr.GetString(), objIdVal);
    pAllJoynObjectElem->SetAttribute(visibleAttr.GetString(), objVisibleVal.ToString());

    // Create the AllJoyn Object Node and configure its attributes
    if (object.description->Length() > 0)
    {
        CHK_HR(addTextNode(descNode.GetString(), object.description, pAllJoynObjectElem));
    }

    if (pRootAllJoynObjListElem->AppendChild(pAllJoynObjectElem) == nullptr)
    {
        hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
    }

CleanUp:
    return hr;
}

_Use_decl_annotations_
HRESULT
BridgeConfig::RemoveObject(String^ id)
{
    HRESULT hr = S_OK;
    String^ objectPathName;
    StringReference objectsRoot(OBJECTS_NODE_PATH);
    StringReference objectPathPrefix(ALLJOYN_OBJECT_PATH_PREFIX);
    StringReference objectPathSuffix(ALLJOYN_OBJECT_PATH_SUFFIX);
    StringReference visibleAttr(VISIBLE_ATTR);

    IXmlNode^ pNode;
    XmlElement^ pElement;
    XmlElement^ pDestObjListElem = nullptr;

    // Verify that Init has been called
    if (m_pXmlDocument == nullptr)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_STATE);
        goto CleanUp;
    }

    pDestObjListElem = (XmlElement^)m_pXmlDocument->SelectSingleNode(objectsRoot);
    if (pDestObjListElem == nullptr)
    {
        hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
        goto CleanUp;
    }

    // Create an XPATH query to find the specifed bus-oject in this document
    objectPathName += objectPathPrefix.GetString();
    objectPathName += id;
    objectPathName += objectPathSuffix.GetString();

    // Try to locate the specified bus object in the document
    pNode = m_pXmlDocument->SelectSingleNode(objectPathName);
    if (pNode == nullptr)
    {
        hr = S_FALSE;
        goto CleanUp;
    }

    // Remove the child
    if (pDestObjListElem->RemoveChild(pNode) == nullptr)
    {
        hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
    }

CleanUp:
    return hr;
}

_Use_decl_annotations_
HRESULT
BridgeConfig::getAttributeValue(
IXmlNode^ pNode,
String^ attributeName,
String^* result)
{
    HRESULT hr = S_OK;
    XmlElement^ pElem = nullptr;

    // Determine the current bus-object's ID from the source element
    pElem = (XmlElement^)pNode;
    *(result) = pElem->GetAttribute(attributeName);
    if (result == nullptr)
    {
        hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
    }

    return hr;
}

_Use_decl_annotations_
HRESULT
BridgeConfig::MergeObjectNode(
IXmlNode^ pCurrSrcNode,
XmlElement^ pDestDocElem)
{
    HRESULT hr = S_OK;
    String^  allJoynObjectId;
    String^  objectPathId;
    StringReference objectPathPrefix(ALLJOYN_OBJECT_PATH_PREFIX);
    StringReference objectPathSuffix(ALLJOYN_OBJECT_PATH_SUFFIX);
    StringReference objectIdAttr(OBJECT_ID_ATTR);
    XmlElement^ pCurrDestNode = nullptr;

    if (pCurrSrcNode->NodeType != NodeType::ElementNode)
    {
        hr = S_FALSE;
        goto CleanUp;
    }

    // Determine the current bus-object's ID from the source element
    CHK_HR(getAttributeValue(pCurrSrcNode, objectIdAttr.GetString(), &(allJoynObjectId)));

    // Try to locate the specified bus-object in the destination document (this document)
    objectPathId += objectPathPrefix.GetString();
    objectPathId += allJoynObjectId;
    objectPathId += objectPathSuffix.GetString();
    pCurrDestNode = (XmlElement^)(m_pXmlDocument->SelectSingleNode(objectPathId));

    // If the node exists in the destination document
    if (pCurrDestNode != nullptr)
    {
        XmlElement^ pCurrSrcElem = nullptr;
        IXmlNode^ pNewOutNode = nullptr;
        IXmlNode^ pClonedReplacementNode = nullptr;

        // Then copy the entire source node's sub-tree and replace the
        // the entire destination node (the source completely trumps destination)
        pCurrSrcElem = (XmlElement^)pCurrSrcNode;

        pClonedReplacementNode = m_pXmlDocument->ImportNode(pCurrSrcElem, true);
        if (pClonedReplacementNode == nullptr)
        {
            hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
            goto CleanUp;
        }

        if (pDestDocElem->ReplaceChild(pClonedReplacementNode, pCurrDestNode) == nullptr)
        {
            hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
            goto CleanUp;
        }
    }

CleanUp:
    return hr;
}

_Use_decl_annotations_
HRESULT
BridgeConfig::MergeSettingsNode(XmlDocument^ pSrcDoc)
{
    HRESULT hr = S_OK;
    String^  allJoynObjectId;
    String^  objectPathId;
    StringReference settingsRoot(SETTINGS_NODE_PATH);
    IXmlNode^ pSettingsNode = pSrcDoc->SelectSingleNode(settingsRoot);

    if (pSettingsNode != nullptr)
    {
        // Then copy the entire source node's sub-tree and replace the
        // the entire destination node (the source completely trumps destination)
        XmlElement^ pCurrSrcElem = (XmlElement^)pSettingsNode;

        // Make a copy of the source settings in the target settings context (import from one file to the local copy)
        IXmlElement^ pClonedReplacementNode = (IXmlElement^)(m_pXmlDocument->ImportNode(pCurrSrcElem, true));
        if (pClonedReplacementNode == nullptr)
        {
            hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
            goto CleanUp;
        }

        // Get root of target document's settings (which should always exist)
        XmlElement^ pDestSettingsElem = (XmlElement^)(m_pXmlDocument->SelectSingleNode(settingsRoot));

        // If for some reason the settings are missing in the target document (even though they should always exist)
        // simply add them to the bridge config node
        if (pDestSettingsElem == nullptr)
        {
            StringReference bridgeConfigNode(BRIDGE_CONFIG_PATH);
            XmlElement^ pDestSettingsParentElem = (XmlElement^)(m_pXmlDocument->SelectSingleNode(bridgeConfigNode));
            if (nullptr == pDestSettingsParentElem->AppendChild(pClonedReplacementNode))
            {
                hr = E_UNEXPECTED;
            }
        }
        // otherwise, the settings do exist in the target file so just replace them completely.
        else
        {
            XmlElement^ pDestSettingsParentElem = (XmlElement^)(pDestSettingsElem->ParentNode);
            if (pDestSettingsParentElem->ReplaceChild(pClonedReplacementNode, pDestSettingsElem) == nullptr)
            {
                hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
                goto CleanUp;
            }
        }
    }

CleanUp:
    return hr;
}


_Use_decl_annotations_
HRESULT
BridgeConfig::MergeFrom(BridgeConfig& src)
{
    HRESULT hr = S_OK;
    HRESULT hrMergeOp = S_OK;
    long itemIdx = 0;
    long numSuccesses = 0;
    long numObjects = 0;
    String^ failedSrcElemId;
    StringReference objectsRoot(OBJECTS_NODE_PATH);
    StringReference objIdAttr(OBJECT_ID_ATTR);

    XmlElement^ pSrcDocElem = nullptr;
    XmlElement^ pDestDocElem = nullptr;
    IXmlNode^ pCurrSrcNode = nullptr;
    XmlNodeList^ pSrcObjects = nullptr;

    // Verify that Init has been called on both objects
    auto srcDoc = src.m_pXmlDocument;
    if ((srcDoc == nullptr) || (m_pXmlDocument == nullptr))
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_STATE);
        goto CleanUp;
    }

    // Try to merge the Settings node first (if no settings exist in the source document, this is a no-op)
    hr = MergeSettingsNode(srcDoc);
    if (FAILED(hr))
    {
        goto CleanUp;
    }

    // Get root of source document's object list.  If it doesn't exist, then there is nothing to do
    // so exit immediately
    pSrcDocElem = (XmlElement^)srcDoc->SelectSingleNode(objectsRoot);
    if (pSrcDocElem == nullptr)
    {
        hr = S_OK;
        goto CleanUp;
    }

    // Get root of target document's object list
    pDestDocElem = (XmlElement^)m_pXmlDocument->SelectSingleNode(objectsRoot);
    if (pDestDocElem == nullptr)
    {
        hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
        goto CleanUp;
    }

    pSrcObjects = pSrcDocElem->ChildNodes;
    if (pSrcObjects == nullptr)
    {
        hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
        goto CleanUp;
    }

    pCurrSrcNode = pSrcObjects->Item(itemIdx);
    if (pCurrSrcNode == nullptr)
    {
        hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
        goto CleanUp;
    }

    numObjects = pSrcObjects->Length;

    // For each object in the source list, see if it should replace an object in the target's list
    while (pCurrSrcNode != nullptr)
    {
        // Try to merge the current source node into the current in-memory document
        HRESULT hrTemp = MergeObjectNode(pCurrSrcNode, pDestDocElem);

        // Track first intermediate failure, but keep trying to merge
        if (FAILED(hrTemp) && (hrMergeOp == S_OK))
        {
            hrMergeOp = hrTemp;
            getAttributeValue(pCurrSrcNode, objIdAttr, &failedSrcElemId);
        }
        else if (hrTemp == S_OK)
        {
            ++numSuccesses;
        }

        // We're done with the node in the source object, so advance to the next one
        ++itemIdx;
        pCurrSrcNode = pSrcObjects->Item(itemIdx);
    }

    // If there was a merge-op failure, then update hr.  Will be S_OK if no intermediate merge failure
    hr = hrMergeOp;

CleanUp:
    return hr;
}

_Use_decl_annotations_
HRESULT
BridgeConfig::addTextNode(
String^ name,
String^ text,
XmlElement^ parentNode)
{
    HRESULT hr = S_OK;
    XmlElement^ textNode = m_pXmlDocument->CreateElement(name);
    textNode->InnerText = text;

    if (parentNode->AppendChild(textNode) == nullptr)
    {
        hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
    }

    return hr;
}

_Use_decl_annotations_
HRESULT
BridgeConfig::convertXsBooleanToBool(String^ varValue, bool &bValue)
{
    HRESULT hr = S_OK;
    const WCHAR NUM_TRUE[] = L"1";
    const WCHAR NUM_FALSE[] = L"0";
    const WCHAR WORD_TRUE[] = L"true";
    const WCHAR WORD_FALSE[] = L"false";

    if (wcscmp(NUM_TRUE, varValue->Data()) == 0 || wcscmp(WORD_TRUE, varValue->Data()) == 0)
    {
        bValue = true;
    }
    else if (wcscmp(WORD_FALSE, varValue->Data()) == 0 || wcscmp(WORD_TRUE, varValue->Data()) == 0)
    {
        bValue = false;
    }
    else
    {
        hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
    }

    return hr;
}

_Use_decl_annotations_
HRESULT
BridgeConfig::readTextNode(
String^ nodeName,
XmlElement^ parentNode,
bool bIsOptional,
String^* text)
{
    HRESULT hr = S_OK;
    IXmlNode^ outNode;
    StringReference rootNode(ROOT_NODE);
    StringReference textNode(TEXT_NODE);

    try
    {
        // Clear string value
        *(text) = nullptr;

        // Create XPATH Query.
        // Try to find specified node relative to the current node
        String^ queryString;
        queryString += rootNode.GetString();
        queryString += nodeName;
        queryString += textNode.GetString();

        outNode = parentNode->SelectSingleNode(queryString);
        // If node was not found, then decide whether to return success or failure
        if (outNode == nullptr)
        {
            if (!bIsOptional)
            {
                hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
                goto CleanUp;
            }
        }
        // If node was found, extract its text value and return to caller
        else
        {
            *(text) = dynamic_cast<String^>(outNode->NodeValue);
        }
    }
    // Handle potential exceptions from wstring
    catch (...)
    {
        hr = E_OUTOFMEMORY;
    }

CleanUp:
    return hr;
}

_Use_decl_annotations_
bool
BridgeConfig::isFilePresent(StorageFolder^ appFolder, String^ pFileName)
{
    IStorageItem^ sItem = nullptr;

    task<IStorageItem^> tryGetFileTask = create_task(appFolder->TryGetItemAsync(pFileName));
    tryGetFileTask.then([&](IStorageItem^ item)
    {
        sItem = item;
    }).wait();

    return sItem != nullptr;
}

String^
BridgeConfig::BridgeKeyX()
{
    return GetCredentialValue(SETTINGS_BRIDGE_KEYX_PATH);
}

String^
BridgeConfig::DeviceKeyX()
{
    return GetCredentialValue(SETTINGS_DEVICE_KEYX_PATH);
}

String^
BridgeConfig::DeviceUsername()
{
    return GetCredentialValue(SETTINGS_DEVICE_USERNAME_PATH);
}

String^
BridgeConfig::DevicePassword()
{
	return GetCredentialValue(SETTINGS_DEVICE_PASSWORD_PATH);
}

String^
BridgeConfig::DeviceEcdheEcdsaPrivateKey()
{
    return GetCredentialValue(SETTINGS_DEVICE_ECDHE_ECDSA_PRIVATEKEY_PATH);
}

String^
BridgeConfig::DeviceEcdheEcdsaCertChain()
{
    return GetCredentialValue(SETTINGS_DEVICE_ECDHE_ECDSA_CERTCHAIN_PATH);
}

String^
BridgeConfig::GetCredentialValue(LPCWSTR xmlPath)
{
    String^ result(L"");
    // Create XPATH Query.
    // Try to find specified node relative to the current node
    StringReference queryString(xmlPath);
    auto outNode = m_pXmlDocument->SelectSingleNode(queryString);

    // If node found, then get the value out of the node and return it
    if (outNode != nullptr)
    {
        result = dynamic_cast<String^>(outNode->InnerText);
    }

    return result;
}

bool BridgeConfig::DefaultVisibility()
{
    bool bDefaultVisibility = false;

    // Create XPATH Query.
    // Try to find specified node relative to the current node
    StringReference queryString(SETTINGS_DEVICE_DEFAULT_VIS_PATH);
    auto outNode = m_pXmlDocument->SelectSingleNode(queryString);

    // If node found, then get the value out of the node and return it
    if (outNode != nullptr)
    {
        HRESULT hr = convertXsBooleanToBool(dynamic_cast<String^>(outNode->InnerText), bDefaultVisibility);
        if (FAILED(hr))
        {
            bDefaultVisibility = false;
        }
    }

    return bDefaultVisibility;
}
