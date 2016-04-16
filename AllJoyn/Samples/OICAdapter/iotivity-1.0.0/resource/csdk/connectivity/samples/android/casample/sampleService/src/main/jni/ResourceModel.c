#include <jni.h>
#include <android/log.h>
#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cainterface.h"
#include "cacommon.h"
#include "caadapterutils.h"
#include "oic_string.h"

#include "org_iotivity_ca_service_RMInterface.h"

#define  LOG_TAG   "JNI_INTERFACE_SAMPLE"
#define  LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define  LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Iotivity Device Identity.
const unsigned char IDENTITY[] = ("1111111111111111");

// PSK between this device and peer device.
const unsigned char RS_CLIENT_PSK[] = ("AAAAAAAAAAAAAAAA");

#define PORT_LENGTH 5
#define SECURE_DEFAULT_PORT 5684
#define RESOURCE_URI_LENGTH 14
#define OPTION_INFO_LENGTH 1024
#define NETWORK_INFO_LENGTH 1024
#define BIG_PAYLOAD_LENGTH 1024
#define RECEIVED_FILE_NAME_PREFIX_LENGTH 7
#define RECEIVED_FILE_NAME_LENGTH 14

typedef struct
{
    char ipAddress[CA_IPADDR_SIZE];
    uint16_t port;
} addressSet_t;

static void request_handler(const CAEndpoint_t* object, const CARequestInfo_t* requestInfo);
static void response_handler(const CAEndpoint_t* object, const CAResponseInfo_t* responseInfo);
static void error_handler(const CAEndpoint_t *object, const CAErrorInfo_t* errorInfo);
static void get_resource_uri(const char *URI, char *resourceURI, int32_t length);
static uint32_t get_secure_information(CAPayload_t payLoad);
static CAResult_t get_network_type(uint32_t selectedNetwork);
static void callback(char *subject, char *receivedData);
static CAResult_t get_remote_address(const char *address);
static void parsing_coap_uri(const char* uri, addressSet_t* address, CATransportFlags_t *flags);
static void delete_global_references(JNIEnv *env, jobject obj);
static int get_address_set(const char *pAddress, addressSet_t* outAddress);
bool read_file(const char* name, char** bytes, size_t* length);
uint32_t gettodaydate();
void saveFile(const char *payload, size_t payloadSize);

uint16_t g_localSecurePort = SECURE_DEFAULT_PORT;
CATransportAdapter_t g_selectedNwType = CA_ADAPTER_IP;
static CAToken_t g_lastRequestToken = NULL;
static uint8_t g_lastRequestTokenLength = 0;

static const char COAP_PREFIX[] =  "coap://";
static const char COAPS_PREFIX[] = "coaps://";
static const uint16_t COAP_PREFIX_LEN = sizeof(COAP_PREFIX) - 1;
static const uint16_t COAPS_PREFIX_LEN = sizeof(COAPS_PREFIX) - 1;

static const char RECEIVED_FILE_PATH[] = "/storage/emulated/0/Download/%d%s.txt";

static const char SECURE_INFO_DATA[]
                                   = "{\"oc\":[{\"href\":\"%s\",\"prop\":{\"rt\":[\"core.led\"],"
                                     "\"if\":[\"oic.if.baseline\"],\"obs\":1,\"sec\":1,\"port\":"
                                     "%d}}]}";
static const char NORMAL_INFO_DATA[]
                                   = "{\"oc\":[{\"href\":\"%s\",\"prop\":{\"rt\":[\"core.led\"],"
                                     "\"if\":[\"oic.if.baseline\"],\"obs\":1}}]}";

static jobject g_responseListenerObject = NULL;
static JavaVM *g_jvm;

static CAEndpoint_t *g_clientEndpoint = NULL;
static char *g_resourceUri = NULL;
static CAToken_t g_clientToken = NULL;
static uint8_t g_clientTokenLength = 0;

static uint16_t g_clientMsgId = 0;
static char *g_remoteAddress = NULL;

// init
JNIEXPORT void JNICALL
Java_org_iotivity_ca_service_RMInterface_setNativeResponseListener(JNIEnv *env, jobject obj,
                                                                   jobject listener)
{
    LOGI("setNativeResponseListener");
    if (!env || !obj || !listener)
    {
        LOGI("Invalid input parameter");
        return;
    }

    g_responseListenerObject = (*env)->NewGlobalRef(env, listener);
}

#ifdef __WITH_DTLS__
// Internal API. Invoked by OC stack to retrieve credentials from this module
int32_t CAGetDtlsPskCredentials( CADtlsPskCredType_t type,
              const unsigned char *desc, size_t desc_len,
              unsigned char *result, size_t result_length)
{
    LOGI("CAGetDtlsPskCredentials IN");

    int32_t ret = -1;

    if (NULL == result)
    {
        return ret;
    }

    switch (type)
    {
        case CA_DTLS_PSK_HINT:
        case CA_DTLS_PSK_IDENTITY:

            if (result_length < sizeof(IDENTITY))
            {
                LOGE("ERROR : Wrong value for result for storing IDENTITY");
                return ret;
            }

            memcpy(result, IDENTITY, sizeof(IDENTITY));
            ret = sizeof(IDENTITY);
            break;

        case CA_DTLS_PSK_KEY:

            if ((desc_len == sizeof(IDENTITY)) &&
                memcmp(desc, IDENTITY, sizeof(IDENTITY)) == 0)
            {
                if (result_length < sizeof(RS_CLIENT_PSK))
                {
                    LOGE("ERROR : Wrong value for result for storing RS_CLIENT_PSK");
                    return ret;
                }

                memcpy(result, RS_CLIENT_PSK, sizeof(RS_CLIENT_PSK));
                ret = sizeof(RS_CLIENT_PSK);
            }
            break;

        default:

            LOGE("Wrong value passed for PSK_CRED_TYPE.");
            ret = -1;
    }

    LOGI("CAGetDtlsPskCredentials OUT\n");
    return ret;
}

#endif

JNIEXPORT jint JNI_OnLoad(JavaVM *jvm, void *reserved)
{
    LOGI("JNI_OnLoad");
    (void)reserved;

    JNIEnv* env;
    if (JNI_OK != (*jvm)->GetEnv(jvm, (void**) &env, JNI_VERSION_1_6))
    {
        return -1;
    }
    g_jvm = jvm; /* cache the JavaVM pointer */

    CANativeJNISetJavaVM(g_jvm);

    return JNI_VERSION_1_6;
}

void JNI_OnUnload(JavaVM *jvm, void *reserved)
{
    LOGI("JNI_OnUnload");
    (void)reserved;

    JNIEnv* env;
    if (JNI_OK != (*jvm)->GetEnv(jvm, (void**) &env, JNI_VERSION_1_6))
    {
        return;
    }
    g_jvm = 0;
    return;
}

JNIEXPORT void JNICALL
Java_org_iotivity_ca_service_RMInterface_RMInitialize(JNIEnv *env, jobject obj, jobject context)
{
    LOGI("RMInitialize");
    if (!env || !obj || !context)
    {
        LOGI("Invalid input parameter");
        return;
    }

    //Currently set context for Android Platform
    CANativeJNISetContext(env, context);

    CAResult_t res = CAInitialize();

    if (CA_STATUS_OK != res)
    {
        LOGE("Could not Initialize");
    }

#ifdef __WITH_DTLS__
    res = CARegisterDTLSCredentialsHandler(CAGetDtlsPskCredentials);
    if(CA_STATUS_OK != res)
    {
        LOGE("Set credential handler fail");
        return;
    }
#endif
}

JNIEXPORT void JNICALL
Java_org_iotivity_ca_service_RMInterface_RMTerminate(JNIEnv *env, jobject obj)
{
    LOGI("RMTerminate");
    if (!env || !obj)
    {
        LOGI("Invalid input parameter");
        return;
    }

    CADestroyToken(g_lastRequestToken);
    CATerminate();
    delete_global_references(env, obj);
}

JNIEXPORT void JNICALL
Java_org_iotivity_ca_service_RMInterface_RMStartListeningServer(JNIEnv *env, jobject obj)
{
    LOGI("RMStartListeningServer");
    if (!env || !obj)
    {
        LOGI("Invalid input parameter");
        return;
    }

    if (CA_STATUS_OK != CAStartListeningServer())
    {
        LOGE("Could not start Listening server");
    }
}

JNIEXPORT void JNICALL
Java_org_iotivity_ca_service_RMInterface_RMStartDiscoveryServer(JNIEnv *env, jobject obj)
{
    LOGI("RMStartDiscoveryServer");
    if (!env || !obj)
    {
        LOGI("Invalid input parameter");
        return;
    }

    if (CA_STATUS_OK != CAStartDiscoveryServer())
    {
        LOGE("Could not start discovery server");
    }
}

JNIEXPORT void JNICALL
Java_org_iotivity_ca_service_RMInterface_RMRegisterHandler(JNIEnv *env, jobject obj)
{
    LOGI("RMRegisterHandler");
    if(!env || !obj)
    {
        LOGI("Invalid input parameter");
        return;
    }

    CARegisterHandler(request_handler, response_handler, error_handler);
}

JNIEXPORT void JNICALL
Java_org_iotivity_ca_service_RMInterface_RMSendRequest(JNIEnv *env, jobject obj, jstring uri,
                                                       jstring payload, jint selectedNetwork,
                                                       jint isSecured, jint msgType,
                                                       jboolean isBigData)
{
    LOGI("selectedNetwork - %d", selectedNetwork);
    if (!env || !obj)
    {
        LOGI("Invalid input parameter");
        return;
    }

    if (!uri)
    {
        LOGE("Invalid input parameter : uri");
        return;
    }

    CAResult_t res = get_network_type(selectedNetwork);
    if (CA_STATUS_OK != res)
    {
        return;
    }

    const char* strUri = (*env)->GetStringUTFChars(env, uri, NULL);
    LOGI("RMSendRequest - %s", strUri);

    CATransportFlags_t flags;
    addressSet_t address = {{0}, 0};
    parsing_coap_uri(strUri, &address, &flags);

    //create remote endpoint
    CAEndpoint_t* endpoint = NULL;
    res = CACreateEndpoint(flags, g_selectedNwType, (const char*)(address.ipAddress),
                           address.port, &endpoint);
    if (CA_STATUS_OK != res)
    {
        LOGE("Could not create remote end point");
        (*env)->ReleaseStringUTFChars(env, uri, strUri);
        return;
    }

    CAMessageType_t messageType = msgType;

    // create token
    CAToken_t token = NULL;
    uint8_t tokenLength = CA_MAX_TOKEN_LEN;

    res = CAGenerateToken(&token, tokenLength);
    if ((CA_STATUS_OK != res) || (!token))
    {
        LOGE("token generate error!!");
        // destroy remote endpoint
        CADestroyEndpoint(endpoint);
        (*env)->ReleaseStringUTFChars(env, uri, strUri);
        return;
    }

    char resourceURI[RESOURCE_URI_LENGTH + 1] = { 0 };

    get_resource_uri((const CAURI_t) strUri, resourceURI, RESOURCE_URI_LENGTH);
    (*env)->ReleaseStringUTFChars(env, uri, strUri);

    CAInfo_t requestData = { 0 };
    requestData.token = token;
    requestData.tokenLength = tokenLength;

    size_t payloadLength = 0;
    if (isBigData)
    {
        const char* path = NULL;
        if (payload)
        {
            path = (*env)->GetStringUTFChars(env, payload, NULL);
            if(path)
            {
                char* bigData = NULL;
                bool result = read_file(path, &bigData, &payloadLength);
                if (!result)
                {
                    LOGE("read has failed");
                    (*env)->ReleaseStringUTFChars(env, payload, path);
                    CADestroyToken(token);
                    CADestroyEndpoint(endpoint);
                    return;
                }
                (*env)->ReleaseStringUTFChars(env, payload, path);

                requestData.payload = (CAPayload_t) malloc(payloadLength);
                if (NULL == requestData.payload)
                {
                    LOGE("Memory allocation failed!");
                    // destroy token
                    CADestroyToken(token);
                    // destroy remote endpoint
                    CADestroyEndpoint(endpoint);
                    return;
                }
                memcpy(requestData.payload, bigData, payloadLength);
                requestData.payloadSize = payloadLength;
            }
        }
    }
    else
    {
        if (isSecured)
        {
            payloadLength = sizeof(SECURE_INFO_DATA) + strlen(resourceURI);
            requestData.payload = (CAPayload_t) malloc(payloadLength);
            if (NULL == requestData.payload)
            {
                LOGE("Memory allocation failed!");
                // destroy token
                CADestroyToken(token);
                // destroy remote endpoint
                CADestroyEndpoint(endpoint);
                return;
            }
            snprintf((char *) requestData.payload, payloadLength, SECURE_INFO_DATA,
                     resourceURI, g_localSecurePort);
            requestData.payloadSize = payloadLength;
        }
        else
        {
            payloadLength = sizeof(NORMAL_INFO_DATA) + strlen(resourceURI);
            requestData.payload = (CAPayload_t) malloc(payloadLength);
            if (NULL == requestData.payload)
            {
                LOGE("Memory allocation failed!");
                // destroy token
                CADestroyToken(token);
                // destroy remote endpoint
                CADestroyEndpoint(endpoint);
                return;
            }
            snprintf((char *) requestData.payload, payloadLength, NORMAL_INFO_DATA, resourceURI);
            requestData.payloadSize = payloadLength;
        }
    }

    requestData.type = messageType;
    requestData.resourceUri = (CAURI_t) malloc(sizeof(resourceURI));
    if (NULL == requestData.resourceUri)
    {
        LOGE("Memory allocation failed!");
        // destroy token
        CADestroyToken(token);
        // destroy remote endpoint
        CADestroyEndpoint(endpoint);
        free(requestData.payload);
        return;
    }
    memcpy(requestData.resourceUri, resourceURI, sizeof(resourceURI));

    CARequestInfo_t requestInfo = { 0 };
    requestInfo.method = CA_GET;
    requestInfo.isMulticast = false;
    requestInfo.info = requestData;

    // send request
    if (CA_STATUS_OK != CASendRequest(endpoint, &requestInfo))
    {
        LOGE("Could not send request");
    }

    // destroy token
    CADestroyToken(token);

    // destroy remote endpoint
    CADestroyEndpoint(endpoint);

    free(requestData.payload);
    free(requestData.resourceUri);
}

JNIEXPORT void JNICALL
Java_org_iotivity_ca_service_RMInterface_RMSendReqestToAll(JNIEnv *env, jobject obj, jstring uri,
                                                           jint selectedNetwork)
{
    LOGI("selectedNetwork - %d", selectedNetwork);
    if (!env || !obj)
    {
        LOGI("Invalid input parameter");
        return;
    }

    CAResult_t res = get_network_type(selectedNetwork);
    if (CA_STATUS_OK != res)
    {
        return;
    }

    // create remote endpoint
    CAEndpoint_t *endpoint = NULL;
    res = CACreateEndpoint(CA_IPV4, g_selectedNwType, NULL, 0, &endpoint);

    if (CA_STATUS_OK != res)
    {
        LOGE("create remote endpoint error, error code: %d", res);
        return;
    }

    // create token
    CAToken_t token = NULL;
    uint8_t tokenLength = CA_MAX_TOKEN_LEN;

    res = CAGenerateToken(&token, tokenLength);
    if ((CA_STATUS_OK != res) || (!token))
    {
        LOGE("token generate error!!");
        // destroy remote endpoint
        CADestroyEndpoint(endpoint);
        return;
    }

    LOGI("generated token %s", token);

    CAInfo_t requestData = { 0 };
    requestData.token = token;
    requestData.tokenLength = tokenLength;
    requestData.payload = (CAPayload_t) "TempJsonPayload";
    requestData.payloadSize = strlen((const char *) requestData.payload);
    requestData.type = CA_MSG_NONCONFIRM;

    const char* strUri = (*env)->GetStringUTFChars(env, uri, NULL);
    LOGI("resourceUri - %s", strUri);
    requestData.resourceUri = (CAURI_t)strUri;

    uint8_t optionNum = 2;
    CAHeaderOption_t *headerOpt = (CAHeaderOption_t*) calloc(1,
                                                             sizeof(CAHeaderOption_t) * optionNum);
    if (NULL == headerOpt)
    {
        LOGE("Memory allocation failed");
        // destroy remote endpoint
        CADestroyEndpoint(endpoint);
        return;
    }

    char* FirstOptionData = "Hello";
    headerOpt[0].optionID = 3000;
    memcpy(headerOpt[0].optionData, FirstOptionData, strlen(FirstOptionData));
    headerOpt[0].optionLength = (uint16_t) strlen(FirstOptionData);

    char* SecondOptionData2 = "World";
    headerOpt[1].optionID = 3001;
    memcpy(headerOpt[1].optionData, SecondOptionData2, strlen(SecondOptionData2));
    headerOpt[1].optionLength = (uint16_t) strlen(SecondOptionData2);

    requestData.numOptions = optionNum;
    requestData.options = headerOpt;

    CARequestInfo_t requestInfo = { 0 };
    requestInfo.method = CA_GET;
    requestInfo.isMulticast = true;
    requestInfo.info = requestData;

    // send request to all
    res = CASendRequest(endpoint, &requestInfo);
    if (CA_STATUS_OK != res)
    {
        LOGE("Could not send request to all");
        //destroy token
        CADestroyToken(token);
    }
    else
    {
        CADestroyToken(g_lastRequestToken);
        g_lastRequestToken = token;
        g_lastRequestTokenLength = tokenLength;
    }

    //ReleaseStringUTFChars for strUri
    (*env)->ReleaseStringUTFChars(env, uri, strUri);

    free(headerOpt);

    // destroy remote endpoint
    CADestroyEndpoint(endpoint);
}

JNIEXPORT void JNICALL
Java_org_iotivity_ca_service_RMInterface_RMSendResponse(JNIEnv *env, jobject obj,
                                                        jint selectedNetwork,
                                                        jint isSecured, jint msgType,
                                                        jint responseValue)
{
    LOGI("RMSendResponse");
    if (!env || !obj)
    {
        LOGI("Invalid input parameter");
        return;
    }

    LOGI("selectedNetwork - %d", selectedNetwork);

    CAResult_t res = get_network_type(selectedNetwork);
    if (CA_STATUS_OK != res)
    {
        LOGE("Not supported network type");
        return;
    }

    if (NULL == g_clientEndpoint)
    {
        LOGE("No Request received");
        return;
    }

    CAMessageType_t messageType = msgType;

    CAInfo_t responseData = { 0 };
    responseData.type = messageType;
    responseData.messageId = g_clientMsgId;
    responseData.resourceUri = (CAURI_t)g_resourceUri;

    CAResponseInfo_t responseInfo = { 0 };

    if (msgType != CA_MSG_RESET)
    {
        responseData.token = g_clientToken;
        responseData.tokenLength = g_clientTokenLength;
        responseInfo.result = responseValue;

        if (1 == isSecured)
        {
            uint32_t length = strlen(SECURE_INFO_DATA) + strlen(g_resourceUri) + 1;
            responseData.payload = (CAPayload_t) malloc(length);
            sprintf((char *) responseData.payload, SECURE_INFO_DATA, g_resourceUri,
                    g_localSecurePort);
            responseData.payloadSize = length;
        }
        else
        {
            uint32_t length = strlen(NORMAL_INFO_DATA) + strlen(g_resourceUri) + 1;
            responseData.payload = (CAPayload_t) malloc(length);
            sprintf((char *) responseData.payload, NORMAL_INFO_DATA, g_resourceUri);
            responseData.payloadSize = length;
        }
    }
    //msgType is RESET
    else
    {
        responseInfo.result = CA_EMPTY;
    }

    responseInfo.info = responseData;

    // send response
    res = CASendResponse(g_clientEndpoint, &responseInfo);
    if (CA_STATUS_OK != res)
    {
        LOGE("Could not send response");
    }

    // destroy token
    CADestroyToken(g_clientToken);
    g_clientToken = NULL;
    g_clientTokenLength = 0;

    // destroy remote endpoint
    CADestroyEndpoint(g_clientEndpoint);
    g_clientEndpoint = NULL;
}

JNIEXPORT void JNICALL
Java_org_iotivity_ca_service_RMInterface_RMSendNotification(JNIEnv *env, jobject obj, jstring uri,
                                                            jstring payload, jint selectedNetwork,
                                                            jint isSecured, jint msgType)
{
    LOGI("selectedNetwork - %d", selectedNetwork);
    if (!env || !obj)
    {
        LOGI("Invalid input parameter");
        return;
    }

    if (!payload)
    {
        LOGE("payload is NULL");
    }

    if (!uri)
    {
        LOGE("Invalid input parameter : uri");
        return;
    }

    CAResult_t res = get_network_type(selectedNetwork);
    if (CA_STATUS_OK != res)
    {
        LOGE("Not supported network type");
        return;
    }

    const char* strUri = (*env)->GetStringUTFChars(env, uri, NULL);
    LOGI("RMSendNotification - %s", strUri);

    CATransportFlags_t flags;
    addressSet_t address = {{0}, 0};
    parsing_coap_uri(strUri, &address, &flags);

    //create remote endpoint
    CAEndpoint_t* endpoint = NULL;
    if (CA_STATUS_OK != CACreateEndpoint(flags, g_selectedNwType,
                                         (const char*)address.ipAddress,
                                         address.port, &endpoint))
    {
        //ReleaseStringUTFChars for strUri
        (*env)->ReleaseStringUTFChars(env, uri, strUri);
        LOGE("Could not create remote end point");
        return;
    }

    char resourceURI[RESOURCE_URI_LENGTH + 1] = { 0 };
    get_resource_uri(strUri, resourceURI, RESOURCE_URI_LENGTH);

    //ReleaseStringUTFChars for strUri
    (*env)->ReleaseStringUTFChars(env, uri, strUri);

    CAMessageType_t messageType = msgType;

    // create token
    CAToken_t token = NULL;
    uint8_t tokenLength = CA_MAX_TOKEN_LEN;

    res = CAGenerateToken(&token, tokenLength);
    if ((CA_STATUS_OK != res) || (!token))
    {
        LOGE("token generate error!");
        CADestroyEndpoint(endpoint);
        return;
    }

    CAInfo_t requestData = { 0 };
    requestData.token = token;
    requestData.tokenLength = tokenLength;
    requestData.resourceUri = (CAURI_t) malloc(sizeof(resourceURI));
    if (NULL == requestData.resourceUri)
    {
        LOGE("Memory allocation failed!");
        // destroy token
        CADestroyToken(token);
        // destroy remote endpoint
        CADestroyEndpoint(endpoint);
        return;
    }
    memcpy(requestData.resourceUri, resourceURI, sizeof(resourceURI));

    if (1 == isSecured)
    {
        uint32_t length = sizeof(SECURE_INFO_DATA) + strlen(resourceURI);
        requestData.payload = (CAPayload_t) malloc(length);
        if (NULL == requestData.payload)
        {
            LOGE("Memory allocation failed!");
            // destroy token
            CADestroyToken(token);
            // destroy remote endpoint
            CADestroyEndpoint(endpoint);

            free(requestData.resourceUri);
            return;
        }
        snprintf((char *) requestData.payload, length, SECURE_INFO_DATA, resourceURI, g_localSecurePort);
        requestData.payloadSize = length;
    }
    else
    {
        uint32_t length = sizeof(NORMAL_INFO_DATA) + strlen(resourceURI);
        requestData.payload = (CAPayload_t) malloc(length);
        if (NULL == requestData.payload)
        {
            LOGE("Memory allocation failed!");
            // destroy token
            CADestroyToken(token);
            // destroy remote endpoint
            CADestroyEndpoint(endpoint);

            free(requestData.resourceUri);
            return;
        }
        snprintf((char *) requestData.payload, length, NORMAL_INFO_DATA, resourceURI);
        requestData.payloadSize = length;
    }

    requestData.type = messageType;

    CARequestInfo_t requestInfo = { 0 };
    requestInfo.method = CA_GET;
    requestInfo.info = requestData;

    // send notification
    if (CA_STATUS_OK != CASendRequest(endpoint, &requestInfo))
    {
        LOGE("Could not send notification");
    }

    LOGI("Send Notification");

    // destroy token
    CADestroyToken(token);

    // destroy remote endpoint
    CADestroyEndpoint(endpoint);

    free(requestData.payload);
    free(requestData.resourceUri);
}

JNIEXPORT void JNICALL
Java_org_iotivity_ca_service_RMInterface_RMSelectNetwork(JNIEnv *env, jobject obj,
                                                         jint networkType)
{
    LOGI("RMSelectNetwork Type : %d", networkType);
    if (!env || !obj)
    {
        LOGI("Invalid input parameter");
        return;
    }

    if (CA_STATUS_OK != CASelectNetwork(networkType))
    {
        LOGE("Could not select network");
    }
}

JNIEXPORT void JNICALL
Java_org_iotivity_ca_service_RMInterface_RMUnSelectNetwork(JNIEnv *env, jobject obj,
                                                           jint networkType)
{
    LOGI("RMUnSelectNetwork Type : %d", networkType);
    if (!env || !obj)
    {
        LOGI("Invalid input parameter");
        return;
    }

    if (CA_STATUS_OK != CAUnSelectNetwork(networkType))
    {
        LOGE("Could not unselect network");
    }
}

JNIEXPORT void JNICALL
Java_org_iotivity_ca_service_RMInterface_RMGetNetworkInfomation(JNIEnv *env, jobject obj)
{
    LOGI("RMGetNetworkInfomation");
    if (!env || !obj)
    {
        LOGI("Invalid input parameter");
        return;
    }

    CAEndpoint_t *tempInfo = NULL;
    uint32_t tempSize = 0;

    CAResult_t res = CAGetNetworkInformation(&tempInfo, &tempSize);
    if (CA_STATUS_OK != res)
    {
        LOGE("Could not start get network information");
        free(tempInfo);
        return;
    }

    LOGI("################## Network Information #######################");
    callback("######## Network Information", "#######");
    LOGI("Network info total size is %d", tempSize);

    uint32_t index;
    for (index = 0; index < tempSize; index++)
    {
        res = get_remote_address(tempInfo[index].addr);
        if (CA_STATUS_OK != res)
        {
            free(tempInfo);
            return;
        }
        if (NULL != g_responseListenerObject)
        {
            char networkInfo[NETWORK_INFO_LENGTH];
            LOGI("Type: %d", tempInfo[index].adapter);
            sprintf(networkInfo, "%d",tempInfo[index].adapter);
            callback("Type :", networkInfo);
            if (CA_ADAPTER_IP == tempInfo[index].adapter)
            {
                LOGI("Port: %d", tempInfo[index].port);
                sprintf(networkInfo, "%d",tempInfo[index].port);
                callback("Port: ", networkInfo);
            }
            LOGI("Secured: %d", (tempInfo[index].flags & CA_SECURE));
            LOGI("Address: %s", g_remoteAddress);
            callback("Address: ", g_remoteAddress);
            free(g_remoteAddress);
        }
        if (tempInfo[index].flags & CA_SECURE)
        {
            g_localSecurePort = tempInfo[index].port;
        }
    }

    // free
    free(tempInfo);

    LOGI("##############################################################");
}

JNIEXPORT void JNICALL
Java_org_iotivity_ca_service_RMInterface_RMHandleRequestResponse(JNIEnv *env, jobject obj)
{
    LOGI("RMHandleRequestResponse");
    if(!env || !obj)
    {
        LOGI("Invalid input parameter");
        return;
    }

    if (CA_STATUS_OK != CAHandleRequestResponse())
    {
        LOGE("Could not handle request and response");
    }
}

void request_handler(const CAEndpoint_t* object, const CARequestInfo_t* requestInfo)
{

    if (!object)
    {
        LOGE("Remote endpoint is NULL!");
        return;
    }

    if (!requestInfo)
    {
        LOGE("Request info is NULL!");
        return;
    }

    if ((NULL != g_lastRequestToken) && (NULL != requestInfo->info.token) &&
            (strncmp(g_lastRequestToken, requestInfo->info.token,
                     requestInfo->info.tokenLength) == 0))
    {
        LOGI("token is same. received request of it's own. skip.. ");
        return;
    }

    CAResult_t res = get_remote_address(object->addr);
    if (CA_STATUS_OK != res)
    {
        return;
    }

    LOGI("##########received request from remote device #############");
    LOGI("Remote Address: %s", g_remoteAddress);
    LOGI("Remote Port: %d", object->port);
    LOGI("Uri: %s", requestInfo->info.resourceUri);
    LOGI("Data: %s", requestInfo->info.payload);
    LOGI("Token: %s", requestInfo->info.token);
    LOGI("Code: %d", requestInfo->method);
    LOGI("MessageType: %d", requestInfo->info.type);

    if (NULL != g_responseListenerObject)
    {
        char *cloneUri = NULL;
        uint32_t len = 0;

        if (NULL != requestInfo->info.resourceUri)
        {
            len = strlen(requestInfo->info.resourceUri);
            cloneUri = (char *)malloc(sizeof(char) * (len + 1));

            if (NULL == cloneUri)
            {
                LOGE("cloneUri Out of memory");
                free(g_remoteAddress);
                return;
            }

            memcpy(cloneUri, requestInfo->info.resourceUri, len + 1);
            callback("Uri: ", cloneUri);
        }

        len = strlen(g_remoteAddress);
        char *cloneRemoteAddress = (char *) malloc(sizeof(char) * (len + 1));

        if (NULL == cloneRemoteAddress)
        {
            LOGE("cloneRemoteAddress Out of memory");
            free(g_remoteAddress);
            free(cloneUri);
            return;
        }

        memcpy(cloneRemoteAddress, g_remoteAddress, len + 1);

        callback("Remote Address: ", cloneRemoteAddress);
        free(cloneRemoteAddress);
        free(g_remoteAddress);

        char portInfo[PORT_LENGTH] = { 0, };
        sprintf(portInfo, "%d", object->port);
        callback("Remote Port: ", portInfo);

        //clone g_clientEndpoint
        g_clientEndpoint = (CAEndpoint_t *) malloc(sizeof(CAEndpoint_t));
        if (NULL == g_clientEndpoint)
        {
            LOGE("g_clientEndpoint Out of memory");
            free(cloneUri);
            return;
        }
        memcpy(g_clientEndpoint, object, sizeof(CAEndpoint_t));

        if (NULL != cloneUri)
        {
            len = strlen(cloneUri);
            g_resourceUri = (char *) malloc(sizeof(char) * (len + 1));
            if (NULL == g_resourceUri)
            {
                LOGE("g_clientEndpoint->resourceUri Out of memory");
                free(g_clientEndpoint);
                free(cloneUri);
                return;
            }
            memcpy(g_resourceUri, cloneUri, len + 1);
            free(cloneUri);
        }
        //clone g_clientToken
        len = requestInfo->info.tokenLength;

        g_clientToken = (char *) malloc(sizeof(char) * len);
        if (NULL == g_clientToken)
        {
            LOGE("g_clientToken Out of memory");
            free(g_clientEndpoint);
            return;
        }

        if (NULL != requestInfo->info.token)
        {
            memcpy(g_clientToken, requestInfo->info.token, len);
            g_clientTokenLength = len;

        }

        //clone g_clientMsgId
        g_clientMsgId = requestInfo->info.messageId;

        if (NULL != requestInfo->info.payload && requestInfo->info.payloadSize > 0)
        {
            len = requestInfo->info.payloadSize;
            char *clonePayload = (char *) malloc(len + 1);
            if (NULL == clonePayload)
            {
                LOGE("clonePayload Out of memory");
                free(g_clientEndpoint);
                return;
            }

            memcpy(clonePayload, requestInfo->info.payload, len);
            clonePayload[len] = '\0';

            if (len > BIG_PAYLOAD_LENGTH)
            {
                saveFile(clonePayload, len);
            }
            else
            {
                callback("Data: ", clonePayload);
            }
            free(clonePayload);
        }
    }

    if (requestInfo->info.options)
    {
        uint32_t len = requestInfo->info.numOptions;
        uint32_t i;

        LOGI("Option count: %d", requestInfo->info.numOptions);

        for (i = 0; i < len; i++)
        {
            LOGI("Option %d", i + 1);
            LOGI("ID : %d", requestInfo->info.options[i].optionID);
            LOGI("Data[%d]: %s", requestInfo->info.options[i].optionLength,
                 requestInfo->info.options[i].optionData);

            if (NULL != g_responseListenerObject)
            {
                char optionInfo[OPTION_INFO_LENGTH] = { 0, };
                sprintf(optionInfo, "Num[%d] - ID : %d, Option Length : %d", i + 1,
                        requestInfo->info.options[i].optionID,
                        requestInfo->info.options[i].optionLength);

                callback("Option info: ", optionInfo);

                size_t optionDataLen = strlen(requestInfo->info.options[i].optionData);
                char *cloneOptionData = (char *) malloc(sizeof(char) * (optionDataLen + 1));
                if (NULL == cloneOptionData)
                {
                    LOGE("cloneOptionData Out of memory");
                    free(g_clientEndpoint);
                    return;
                }

                memcpy(cloneOptionData, requestInfo->info.options[i].optionData,
                       optionDataLen + 1);

                callback("Option Data: ", cloneOptionData);
                free(cloneOptionData);
            }
        }
    }
    LOGI("############################################################");

    //Check if this has secure communication information
    if (requestInfo->info.payload && CA_ADAPTER_IP == object->adapter)
    {
        uint32_t securePort = get_secure_information(requestInfo->info.payload);
        if (0 < securePort) //Set the remote endpoint secure details and send response
        {
            LOGI("This is secure resource...");

            CAEndpoint_t *endpoint = NULL;
            if (CA_STATUS_OK != CACreateEndpoint(CA_SECURE,
                        object->adapter, object->addr, securePort, &endpoint))
            {
                LOGE("Failed to create duplicate of remote endpoint!");
                return;
            }
            object = endpoint;
        }
    }
}

void response_handler(const CAEndpoint_t* object, const CAResponseInfo_t* responseInfo)
{
    if (!object || !responseInfo)
    {
        LOGE("Invalid input parameter");
        return;
    }

    CAResult_t res = get_remote_address(object->addr);
    if (CA_STATUS_OK != res)
    {
        return;
    }

    LOGI("##########Received response from remote device #############");
    LOGI("Uri: %s", responseInfo->info.resourceUri);
    LOGI("Remote Address: %s", g_remoteAddress);
    LOGI("Remote Port: %d", object->port);
    LOGI("response result: %d", responseInfo->result);
    LOGI("Data: %s", responseInfo->info.payload);
    LOGI("Token: %s", responseInfo->info.token);
    LOGI("MessageType: %d", responseInfo->info.type);

    if (NULL != g_responseListenerObject)
    {
        uint32_t len = 0;

        if (NULL != responseInfo->info.resourceUri)
        {
            len = strlen(responseInfo->info.resourceUri);
            char *cloneUri = (char *) malloc(sizeof(char) * (len + 1));

            if (NULL == cloneUri)
            {
                LOGE("cloneUri Out of memory");
                free(g_remoteAddress);
                return;
            }

            memcpy(cloneUri, responseInfo->info.resourceUri, len + 1);

            callback("Uri: ", cloneUri);
            free(cloneUri);
        }

        len = strlen(g_remoteAddress);
        char *cloneRemoteAddress = (char *) malloc(sizeof(char) * (len + 1));

        if (NULL == cloneRemoteAddress)
        {
            LOGE("cloneRemoteAddress Out of memory");
            free(g_remoteAddress);
            return;
        }

        memcpy(cloneRemoteAddress, g_remoteAddress, len + 1);

        callback("Remote Address: ", cloneRemoteAddress);
        free(cloneRemoteAddress);
        free(g_remoteAddress);

        char portInfo[PORT_LENGTH] = { 0, };
        sprintf(portInfo, "%d", object->port);
        callback("Remote Port: ", portInfo);

        if (NULL != responseInfo->info.payload && responseInfo->info.payloadSize)
        {
            len = responseInfo->info.payloadSize;
            char *clonePayload = (char *) malloc(len + 1);
            if (NULL == clonePayload)
            {
                LOGE("clonePayload Out of memory");
                return;
            }

            memcpy(clonePayload, responseInfo->info.payload, len);
            clonePayload[len] = '\0';

            if (len > BIG_PAYLOAD_LENGTH)
            {
                saveFile(clonePayload, len);
            }
            else
            {
                callback("Data: ", clonePayload);
            }
            free(clonePayload);
        }
    }

    if (responseInfo->info.options)
    {
        uint32_t len = responseInfo->info.numOptions;
        uint32_t i;
        for (i = 0; i < len; i++)
        {
            LOGI("Option %d", i + 1);
            LOGI("ID : %d", responseInfo->info.options[i].optionID);
            LOGI("Data[%d]: %s", responseInfo->info.options[i].optionLength,
                 responseInfo->info.options[i].optionData);

            if (NULL != g_responseListenerObject)
            {
                char optionInfo[OPTION_INFO_LENGTH] = { 0, };
                sprintf(optionInfo, "Num[%d] - ID : %d, Option Length : %d", i + 1,
                        responseInfo->info.options[i].optionID,
                        responseInfo->info.options[i].optionLength);

                callback("Option info: ", optionInfo);

                size_t optionDataLen = strlen(responseInfo->info.options[i].optionData);
                char *cloneOptionData = (char *) malloc(sizeof(char) * (optionDataLen + 1));
                if (NULL == cloneOptionData)
                {
                    LOGE("cloneOptionData Out of memory");
                    return;
                }
                memcpy(cloneOptionData, responseInfo->info.options[i].optionData,
                       optionDataLen + 1);
                callback("Option Data: ", cloneOptionData);
                free(cloneOptionData);
            }
        }
    }
    LOGI("############################################################");

    //Check if this has secure communication information
    if (responseInfo->info.payload && CA_ADAPTER_IP == object->adapter)
    {
        uint32_t securePort = get_secure_information(responseInfo->info.payload);
        if (0 < securePort) //Set the remote endpoint secure details and send response
        {
            LOGI("This is secure resource...");
        }
    }
}

void error_handler(const CAEndpoint_t *rep, const CAErrorInfo_t* errorInfo)
{
    LOGI("+++++++++++++++++++++++++++++++++++ErrorInfo+++++++++++++++++++++++++++++++++++");

    if (rep)
    {
        LOGI("Error Handler, Adapter Type : %d", rep->adapter);
        LOGI("Error Handler, Adapter Type : %s", rep->addr);
    }

    if (errorInfo)
    {
        const CAInfo_t *info = &errorInfo->info;
        LOGI("Error Handler, ErrorInfo :");
        LOGI("Error Handler result    : %d", errorInfo->result);
        LOGI("Error Handler token     : %s", info->token);
        LOGI("Error Handler messageId : %d", (uint16_t) info->messageId);
        LOGI("Error Handler resourceUri : %s", info->resourceUri);
        LOGI("Error Handler type      : %d", info->type);
        LOGI("Error Handler payload   : %s", info->payload);

        if(CA_ADAPTER_NOT_ENABLED == errorInfo->result)
        {
            LOGE("CA_ADAPTER_NOT_ENABLED, enable the adapter");
        }
        else if(CA_SEND_FAILED == errorInfo->result)
        {
            LOGE("CA_SEND_FAILED, unable to send the message, check parameters");
        }
        else if(CA_MEMORY_ALLOC_FAILED == errorInfo->result)
        {
            LOGE("CA_MEMORY_ALLOC_FAILED, insufficient memory");
        }
        else if(CA_SOCKET_OPERATION_FAILED == errorInfo->result)
        {
            LOGE("CA_SOCKET_OPERATION_FAILED, socket operation failed");
        }
        else if(CA_STATUS_FAILED == errorInfo->result)
        {
            LOGE("CA_STATUS_FAILED, message could not be delivered, internal error");
        }
    }
    LOGI("++++++++++++++++++++++++++++++++End of ErrorInfo++++++++++++++++++++++++++++++++");

    return;
}

void get_resource_uri(const char *URI, char *resourceURI, int32_t length)
{
    const char *startPos = URI;
    const char *temp = strstr(URI, "://");
    if (NULL != temp)
    {
        startPos = strchr(temp + 3, '/');
        if (!startPos)
        {
            LOGE("Resource URI is missing");
            return;
        }
    }

    const char *endPos = strchr(startPos, '?');
    if (!endPos)
    {
        endPos = URI + strlen(URI);
    }
    --endPos;

    if (endPos - startPos <= length)
    {
        memcpy(resourceURI, startPos + 1, endPos - startPos);
    }

    LOGI("URI: %s, ResourceURI: %s", URI, resourceURI);
}

uint32_t get_secure_information(CAPayload_t payLoad)
{
    LOGI("entering get_secure_information");

    if (!payLoad)
    {
        LOGE("Payload is NULL");
        return -1;
    }

    const char *subString = NULL;
    if (NULL == (subString = strstr((const char *) payLoad, "\"sec\":1")))
    {
        LOGE("This is not secure resource");
        return -1;
    }

    if (NULL == (subString = strstr((const char *) payLoad, "\"port\":")))
    {
        LOGE("This secure resource does not have port information");
        return -1;
    }

    const char *startPos = strstr(subString, ":");
    if (!startPos)
    {
        LOGE("Parsing failed !");
        return -1;
    }

    const char *endPos = strstr(startPos, "}");
    if (!endPos)
    {
        LOGE("Parsing failed !");
        return -1;
    }

    char portStr[6] = { 0 };
    memcpy(portStr, startPos + 1, (endPos - 1) - startPos);

    LOGI("secured port is: %s", portStr);
    return atoi(portStr);
}

CAResult_t get_network_type(uint32_t selectedNetwork)
{

    uint32_t number = selectedNetwork;

    if (!(number & 0xf))
    {
        return CA_NOT_SUPPORTED;
    }
    if (number & CA_ADAPTER_IP)
    {
        g_selectedNwType = CA_ADAPTER_IP;
        return CA_STATUS_OK;
    }
    if (number & CA_ADAPTER_RFCOMM_BTEDR)
    {
        g_selectedNwType = CA_ADAPTER_RFCOMM_BTEDR;
        return CA_STATUS_OK;
    }
    if (number & CA_ADAPTER_GATT_BTLE)
    {
        g_selectedNwType = CA_ADAPTER_GATT_BTLE;
        return CA_STATUS_OK;
    }

    return CA_NOT_SUPPORTED;
}

void callback(char *subject, char *receivedData)
{
    bool isAttached = false;
    JNIEnv* env;

    if (!g_responseListenerObject)
    {
        LOGE("g_responseListenerObject is NULL, cannot have callback");
        return;
    }

    jint res = (*g_jvm)->GetEnv(g_jvm, (void**) &env, JNI_VERSION_1_6);
    if (JNI_OK != res)
    {
        LOGI("Could not get JNIEnv pointer");
        res = (*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL);

        if (JNI_OK != res)
        {
            LOGE("AttachCurrentThread has failed");
            return;
        }
        isAttached = true;
    }

    jclass cls = (*env)->GetObjectClass(env, g_responseListenerObject);
    if (!cls)
    {
        LOGE("could not get class");
        goto detach_thread;
    }

    jmethodID mid = (*env)->GetMethodID(env, cls, "OnResponseReceived",
                                        "(Ljava/lang/String;Ljava/lang/String;)V");
    if (!mid)
    {
        LOGE("could not get Method ID");
        goto detach_thread;
    }

    jstring jsubject = (*env)->NewStringUTF(env, (char*) subject);
    if (!jsubject)
    {
        LOGE("NewStringUTF failed");
        goto detach_thread;
    }

    jstring jreceivedData = (*env)->NewStringUTF(env, (char*) receivedData);
    if (!jreceivedData)
    {
        LOGE("NewStringUTF failed");
        goto detach_thread;
    }

    (*env)->CallVoidMethod(env, g_responseListenerObject, mid, jsubject, jreceivedData);

detach_thread :
    if (isAttached)
    {
        (*g_jvm)->DetachCurrentThread(g_jvm);
        LOGI("DetachCurrentThread");
    }
}

CAResult_t get_remote_address(const char *address)
{
    uint32_t len = strlen(address);

    g_remoteAddress = (char *)malloc(sizeof (char) * (len + 1));
    if (NULL == g_remoteAddress)
    {
        LOGE("g_remoteAddress Out of memory");
        return CA_MEMORY_ALLOC_FAILED;
    }

    memcpy(g_remoteAddress, address, len + 1);

    return CA_STATUS_OK;
}


void parsing_coap_uri(const char* uri, addressSet_t* address, CATransportFlags_t *flags)
{
    if (NULL == uri || NULL == address)
    {
        LOGE("parameter is null");
        return;
    }

    // parse uri
    // #1. check prefix
    uint8_t startIndex = 0;
    if (strncmp(COAPS_PREFIX, uri, COAPS_PREFIX_LEN) == 0)
    {
        LOGI("uri has '%s' prefix", COAPS_PREFIX);
        startIndex = COAPS_PREFIX_LEN;
        *flags = CA_SECURE;
    }
    else if (strncmp(COAP_PREFIX, uri, COAP_PREFIX_LEN) == 0)
    {
        LOGI("uri has '%s' prefix", COAP_PREFIX);
        startIndex = COAP_PREFIX_LEN;
        *flags = CA_IPV4;
    }

    // #2. copy uri for parse
    size_t len = strlen(uri) - startIndex;

    if (len <= 0)
    {
        LOGE("uri length is 0!");
        return;
    }

    char *cloneUri = (char *) calloc(len + 1, sizeof(char));
    if (NULL == cloneUri)
    {
        LOGE("Out of memory");
        return;
    }

    OICStrcpy(cloneUri, len+1, &uri[startIndex]);

    char *pstr = NULL;
    //filter out the resource uri
    char *pUrl = strtok_r(cloneUri, "/", &pstr);

    if (pUrl)
    {
        LOGI("pAddress : %s", pUrl);
        int res = get_address_set(pUrl, address);
        if (res == -1)
        {
            LOGE("address parse error");

            return;
        }
    }
    else
    {
        LOGE("strtok_r error, could not get the address");
    }

    return;
}

int get_address_set(const char *pAddress, addressSet_t* outAddress)
{
    if (NULL == pAddress || NULL == outAddress)
    {
        LOGE("parameter is null");
        return -1;
    }

    size_t len = strlen(pAddress);
    int isIp = 0;
    size_t ipLen = 0;

    for (size_t i = 0; i < len; i++)
    {
        if (pAddress[i] == '.')
        {
            isIp = 1;
        }

        // found port number start index
        if (isIp && pAddress[i] == ':')
        {
            ipLen = i;
            break;
        }
    }

    if (isIp)
    {
        if(ipLen && (ipLen <  sizeof(outAddress->ipAddress)))
        {
            strncpy(outAddress->ipAddress, pAddress, ipLen);
            outAddress->ipAddress[ipLen] = '\0';
        }
        else if (!ipLen && (len <  sizeof(outAddress->ipAddress)))
        {
            strncpy(outAddress->ipAddress, pAddress, len);
            outAddress->ipAddress[len] = '\0';
        }
        else
        {
            LOGE("IP Address too long: %d", ipLen==0 ? len : ipLen);
            return -1;
        }

        if (ipLen > 0)
        {
            outAddress->port = atoi(pAddress + ipLen + 1);
        }
    }
    else
    {
        strncpy(outAddress->ipAddress, pAddress, len);
        outAddress->ipAddress[len] = '\0';
    }

    return isIp;
}

void delete_global_references(JNIEnv *env, jobject obj)
{
    LOGI("delete_global_references");
    if (!env || !obj )
    {
        LOGI("Invalid input parameter");
        return;
    }

    (*env)->DeleteGlobalRef(env, g_responseListenerObject);
}


bool read_file(const char* name, char** bytes, size_t* length)
{
    if (NULL == name)
    {
        LOGE("parameter is null");
        return false;
    }

    FILE* file;
    char* buffer;
    size_t fileLen;

    // Open file
    file = fopen(name, "rt");
    if (!file)
    {
        fprintf(stderr, "Unable to open file %s", name);
        return false;
    }

    // Get file length
    fseek(file, 0, SEEK_END);
    fileLen = ftell(file);
    fseek(file, 0, SEEK_SET);

    LOGI("file size: %d", fileLen);

    // Allocate memory
    buffer = calloc(1, sizeof(char) * fileLen + 1);
    if (!buffer)
    {
        fprintf(stderr, "Memory error!");
        fclose(file);
        return false;
    }

    // Read file contents into buffer
    size_t ret = fread(buffer, fileLen, 1, file);
    if (ret != 1)
    {
        printf("Failed to read data from file, %s\n", name);
        fclose(file);
        free(buffer);
        return false;
    }

    fclose(file);

    LOGI("file bytes: %s", buffer);

    *bytes = buffer;
    *length = fileLen;

    return true;
}

void saveFile(const char *payload, size_t payloadSize)
{
    // 1. get day
    uint32_t day = gettodaydate();

    // 2. get time
    time_t current_time;
    struct tm * time_info;
    char timeString[RECEIVED_FILE_NAME_PREFIX_LENGTH];

    time(&current_time);
    time_info = localtime(&current_time);

    strftime(timeString, sizeof(timeString), "%H%M%S", time_info);

    uint32_t path_length = strlen(RECEIVED_FILE_PATH) + RECEIVED_FILE_NAME_LENGTH + 1;
    char* path = calloc(1, sizeof(char) * path_length);
    if (path != NULL)
    {
        sprintf(path, RECEIVED_FILE_PATH, day, timeString);
        LOGI("received file path: %s", path);

        FILE *fp = fopen(path, "wt");
        fwrite(payload, 1, payloadSize, fp);
        fclose(fp);

        callback("File Path: ", path);
    }
    else
    {
        LOGE("path Out of memory");
    }
}

uint32_t gettodaydate()
{
    uint32_t ldate;
    time_t clock;
    struct tm *date;

    clock = time(0);
    date = localtime(&clock);
    ldate = date->tm_year * 100000;
    ldate += (date->tm_mon + 1) * 1000;
    ldate += date->tm_mday * 10;
    ldate += date->tm_wday;
    ldate += 190000000;
    ldate /= 10;

    return(ldate);
}
