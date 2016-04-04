/* ****************************************************************
 *
 * Copyright 2014 Samsung Electronics All Rights Reserved.
 *
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************/

/**
 * @file cacommon.h
 * This file contains the common data structures between Resource , CA and adapters
 */

#ifndef CA_COMMON_H_
#define CA_COMMON_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>


#ifdef __cplusplus
extern "C"
{
#endif

/**
 * IP address Length
 */
#define CA_IPADDR_SIZE 16

/**
 * Remote Access jabber ID length.
 */
#define CA_RAJABBERID_SIZE 256

/**
 * Mac address length for BT port
 */
#define CA_MACADDR_SIZE 18

/**
 * Max header options data length
 */
#define CA_MAX_HEADER_OPTION_DATA_LENGTH 20

/**
* Max token length
*/
#define CA_MAX_TOKEN_LEN (8)

/**
 * Max URI length
 */
#ifdef ARDUINO
#define CA_MAX_URI_LENGTH 128  /* maximum size of URI for embedded platforms*/
#else
#define CA_MAX_URI_LENGTH 512 /* maximum size of URI for other platforms*/
#endif

/**
 * Max PDU length supported
 */
#ifdef ARDUINO
#define COAP_MAX_PDU_SIZE           320  /* maximum size of a CoAP PDU for embedded platforms*/
#else
#define COAP_MAX_PDU_SIZE           1400 /* maximum size of a CoAP PDU for big platforms*/
#endif

#ifdef WITH_BWT
#define CA_DEFAULT_BLOCK_SIZE       CA_BLOCK_SIZE_1024_BYTE
#endif

/**
 *Maximum length of the remoteEndpoint identity
 */
#define CA_MAX_ENDPOINT_IDENTITY_LEN   (32)

/**
 * option types - the highest option number 63
 */
#define CA_OPTION_IF_MATCH 1
#define CA_OPTION_ETAG 4
#define CA_OPTION_IF_NONE_MATCH 5
#define CA_OPTION_OBSERVE 6
#define CA_OPTION_LOCATION_PATH 8
#define CA_OPTION_URI_PATH 11
#define CA_OPTION_CONTENT_FORMAT 12
#define CA_OPTION_CONTENT_TYPE COAP_OPTION_CONTENT_FORMAT
#define CA_OPTION_MAXAGE 14
#define CA_OPTION_URI_QUERY 15
#define CA_OPTION_ACCEPT 17
#define CA_OPTION_LOCATION_QUERY 20

/**
 * Payload information from resource model
 */
typedef uint8_t *CAPayload_t;

/**
 * URI for the OIC base.CA considers relative URI as the URI.
 */
typedef char *CAURI_t;

/**
 * Token information for mapping the request and responses by resource model
 */
typedef char *CAToken_t;

// The following flags are the same as the equivalent OIC values in
// octypes.h, allowing direct copying with slight fixup.
// The CA layer should used the OC types when build allows that.
#ifdef RA_ADAPTER
#define MAX_ADDR_STR_SIZE_CA (256)
#else
#define MAX_ADDR_STR_SIZE_CA (40)
#endif

typedef enum
{
    CA_DEFAULT_ADAPTER = 0,

    // value zero indicates discovery
    CA_ADAPTER_IP            = (1 << 0),   // IPv4 and IPv6, including 6LoWPAN
    CA_ADAPTER_GATT_BTLE     = (1 << 1),   // GATT over Bluetooth LE
    CA_ADAPTER_RFCOMM_BTEDR  = (1 << 2),   // RFCOMM over Bluetooth EDR

#ifdef RA_ADAPTER
    CA_ADAPTER_REMOTE_ACCESS = (1 << 3),   // Remote Access over XMPP.
#endif

    CA_ADAPTER_TCP = (1 << 4),   // CoAP over TCP
    CA_ALL_ADAPTERS          = 0xffffffff
} CATransportAdapter_t;

typedef enum
{
    CA_DEFAULT_FLAGS = 0,

    // Insecure transport is the default (subject to change)
    CA_SECURE          = (1 << 4),   // secure the transport path
    // IPv4 & IPv6 autoselection is the default
    CA_IPV6            = (1 << 5),   // IP adapter only
    CA_IPV4            = (1 << 6),   // IP adapter only
    // Indication that a message was received by multicast.
    CA_MULTICAST       = (1 << 7),
    // Link-Local multicast is the default multicast scope for IPv6.
    // These correspond in both value and position to the IPv6 address bits.
    CA_SCOPE_INTERFACE = 0x1, // IPv6 Interface-Local scope
    CA_SCOPE_LINK      = 0x2, // IPv6 Link-Local scope (default)
    CA_SCOPE_REALM     = 0x3, // IPv6 Realm-Local scope
    CA_SCOPE_ADMIN     = 0x4, // IPv6 Admin-Local scope
    CA_SCOPE_SITE      = 0x5, // IPv6 Site-Local scope
    CA_SCOPE_ORG       = 0x8, // IPv6 Organization-Local scope
    CA_SCOPE_GLOBAL    = 0xE, // IPv6 Global scope
} CATransportFlags_t;

#define CA_IPFAMILY_MASK (CA_IPV6|CA_IPV4)
#define CA_SCOPE_MASK 0xf     // mask scope bits above

/**
 * @enum CANetworkStatus_t
 * Information about the network status.
 */
typedef enum
{
    CA_INTERFACE_DOWN,   /**< Connection is not available */
    CA_INTERFACE_UP    /**< Connection is Available */
} CANetworkStatus_t;

/*
 * remoteEndpoint identity
 */
typedef struct
{
    uint16_t id_length;
    unsigned char id[CA_MAX_ENDPOINT_IDENTITY_LEN];
} CARemoteId_t;

/**
 * @enum CAMessageType_t
 * Message Type for Base source code
 */
typedef enum
{
    CA_MSG_CONFIRM = 0,  /**< confirmable message (requires ACK/RST) */
    CA_MSG_NONCONFIRM,   /**< non-confirmable message (one-shot message) */
    CA_MSG_ACKNOWLEDGE,  /**< used to acknowledge confirmable messages */
    CA_MSG_RESET         /**< used to indicates not-interested or error (lack of context)in
                                                  received messages */
} CAMessageType_t;

/**
 * @enum CAMethod_t
 * Allowed method to be used by resource model
 */
typedef enum
{
    CA_GET = 1, /**< GET Method  */
    CA_POST,    /**< POST Method */
    CA_PUT,     /**< PUT Method */
    CA_DELETE   /**< DELETE Method */
} CAMethod_t;

/**
 * block size
 * it depends on defined size in libCoAP.
 */
typedef enum
{
    CA_BLOCK_SIZE_16_BYTE = 0,    /**< 16byte */
    CA_BLOCK_SIZE_32_BYTE = 1,    /**< 32byte */
    CA_BLOCK_SIZE_64_BYTE = 2,    /**< 64byte */
    CA_BLOCK_SIZE_128_BYTE = 3,   /**< 128byte */
    CA_BLOCK_SIZE_256_BYTE = 4,   /**< 256byte */
    CA_BLOCK_SIZE_512_BYTE = 5,   /**< 512byte */
    CA_BLOCK_SIZE_1024_BYTE = 6     /**< 1Kbyte */
} CABlockSize_t;

/**
 * Endpoint information for connectivities
 * Must be identical to OCDevAddr.
 */
typedef struct
{
    CATransportAdapter_t    adapter;    // adapter type
    CATransportFlags_t      flags;      // transport modifiers
    uint16_t                port;       // for IP
    char                    addr[MAX_ADDR_STR_SIZE_CA]; // address for all
    uint32_t                iface;  // usually zero for default interface
#if defined (ROUTING_GATEWAY) || defined (ROUTING_EP)
    char                    routeData[MAX_ADDR_STR_SIZE_CA]; /**< GatewayId:ClientId of
                                                                    destination. **/
#endif
} CAEndpoint_t;

/**
 * Endpoint information for secure messages
 */
typedef struct
{
    CAEndpoint_t endpoint;      /**< endpoint */
    CARemoteId_t identity;      /**< endpoint identity */
} CASecureEndpoint_t;

/**
 * @enum CAResult_t
 * Enums for CA return values
 */
typedef enum
{
    /* Result code - START HERE */
    CA_STATUS_OK = 0,               /**< Success */
    CA_STATUS_INVALID_PARAM,        /**< Invalid Parameter */
    CA_ADAPTER_NOT_ENABLED,         /**< Adapter is not enabled */
    CA_SERVER_STARTED_ALREADY,      /**< Server is started already */
    CA_SERVER_NOT_STARTED,          /**< Server is not started*/
    CA_DESTINATION_NOT_REACHABLE,   /**< Destination is not reachable */
    CA_SOCKET_OPERATION_FAILED,     /**< Socket operation failed */
    CA_SEND_FAILED,                 /**< Send request failed */
    CA_RECEIVE_FAILED,              /**< Receive failed */
    CA_MEMORY_ALLOC_FAILED,         /**< Memory allocation failed */
    CA_REQUEST_TIMEOUT,             /**< Request is Timeout */
    CA_DESTINATION_DISCONNECTED,    /**< Destination is disconnected */
    CA_NOT_SUPPORTED,               /**< Not supported */
    CA_STATUS_NOT_INITIALIZED,      /**< Not Initialized*/
    CA_STATUS_FAILED =255           /**< Failure */
    /* Result code - END HERE */
} CAResult_t;

/**
 * @enum CAResponseResult_t
 * Enums for CA Response values
 */
typedef enum
{
    /* Response status code - START HERE */
    CA_EMPTY = 0,                    /**< Empty */
    CA_CREATED = 201,                /**< Created */
    CA_DELETED = 202,                /**< Deleted */
    CA_VALID = 203,                  /**< Valid */
    CA_CHANGED = 204,                /**< Changed */
    CA_CONTENT = 205,                /**< Content */
    CA_CONTINUE = 231,               /**< Continue */
    CA_BAD_REQ = 400,                /**< Bad Request */
    CA_UNAUTHORIZED_REQ = 401,       /**< Unauthorized Request */
    CA_BAD_OPT = 402,                /**< Bad Option */
    CA_FORBIDDEN_REQ = 403,          /**< Forbidden Request */
    CA_NOT_FOUND = 404,              /**< Not found */
    CA_NOT_ACCEPTABLE = 406,         /**< Not Acceptable */
    CA_REQUEST_ENTITY_INCOMPLETE = 408, /**< Request Entity Incomplete */
    CA_REQUEST_ENTITY_TOO_LARGE = 413,  /**< Request Entity Too Large */
    CA_INTERNAL_SERVER_ERROR = 500,  /**< Internal Server Error */
    CA_RETRANSMIT_TIMEOUT = 504      /**< Retransmit timeout */
    /* Response status code - END HERE */
} CAResponseResult_t;

/**
 * @enum CATransportProtocolID_t
 * Transport Protocol IDs for additional options
 */
typedef enum
{
    CA_INVALID_ID = (1 << 0),   /**< Invalid ID */
    CA_COAP_ID = (1 << 1)       /**< COAP ID */
} CATransportProtocolID_t;

/**
 * @enum CAAdapterState_t
 * Adapter State to indicate the network changed notifications.
 */
typedef enum
{
    CA_ADAPTER_DISABLED,   /**< Adapter is Disabled */
    CA_ADAPTER_ENABLED     /**< Adapter is Enabled */
} CAAdapterState_t;

/**
 * Format indicating which encoding has been used on the payload.
 */
typedef enum
{
    CA_FORMAT_UNDEFINED = 0,            /**< Undefined enoding format */
    CA_FORMAT_TEXT_PLAIN,
    CA_FORMAT_APPLICATION_LINK_FORMAT,
    CA_FORMAT_APPLICATION_XML,
    CA_FORMAT_APPLICATION_OCTET_STREAM,
    CA_FORMAT_APPLICATION_RDF_XML,
    CA_FORMAT_APPLICATION_EXI,
    CA_FORMAT_APPLICATION_JSON,
    CA_FORMAT_APPLICATION_CBOR,
    CA_FORMAT_UNSUPPORTED
} CAPayloadFormat_t;

/**
 * Header options structure to be filled
 *
 * This structure is used to hold header information.
 */
typedef struct
{
    CATransportProtocolID_t protocolID;                     /**< Protocol ID of the Option */
    uint16_t optionID;                                      /**< The header option ID which will be
                                                            added to communication packets */
    uint16_t optionLength;                                  /**< Option Length **/
    char optionData[CA_MAX_HEADER_OPTION_DATA_LENGTH];      /**< Optional data values**/
} CAHeaderOption_t;

/**
 * Base Information received
 *
 * This structure is used to hold request & response base information
 */
typedef struct
{

    CAMessageType_t type;       /**< Qos for the request */
#ifdef ROUTING_GATEWAY
    bool skipRetransmission;    /**< Will not attempt retransmission even if type is CONFIRM.
                                     Required for packet forwarding */
#endif
    uint16_t messageId;         /**< Message id.
                                 * if message id is zero, it will generated by CA inside.
                                 * otherwise, you can use it */
    CAToken_t token;            /**< Token for CA */
    uint8_t tokenLength;        /**< token length*/
    CAHeaderOption_t *options;  /** Header Options for the request */
    uint8_t numOptions;         /**< Number of Header options */
    CAPayload_t payload;        /**< payload of the request  */
    size_t payloadSize;         /**< size in bytes of the payload */
    CAPayloadFormat_t payloadFormat;    /**< encoding format of the request payload */
    CAPayloadFormat_t acceptFormat;     /**< accept format for the response payload */
    CAURI_t resourceUri;        /**< Resource URI information **/
    CARemoteId_t identity;      /**< endpoint identity */
} CAInfo_t;

/**
 * Request Information to be sent
 *
 * This structure is used to hold request information
 */
typedef struct
{
    CAMethod_t method;  /**< Name of the Method Allowed */
    CAInfo_t info;      /**< Information of the request. */
    bool isMulticast;   /**< is multicast request */
} CARequestInfo_t;

/**
 * Response information received
 *
 * This structure is used to hold response information
 */
typedef struct
{
    CAResponseResult_t result;  /**< Result for response by resource model */
    CAInfo_t info;              /**< Information of the response */
    bool isMulticast;
} CAResponseInfo_t;

/**
 * Error information from CA
 *        contains error code and message information
 *
 * This structure holds error information
 */
typedef struct
{
    CAResult_t result;  /**< CA API request result  */
    CAInfo_t info;      /**< message information such as token and payload data
                             helpful to identify the error */
} CAErrorInfo_t;

/**
 * CA Remote Access information for XMPP Client
 *
 */
typedef struct
{
    char *hostname;     /**< XMPP server hostname */
    uint16_t port;      /**< XMPP server serivce port */
    char *xmpp_domain;  /**< XMPP login domain */
    char *username;     /**< login username */
    char *password;     /**< login password */
    char *resource;     /**< specific resource for login */
    char *user_jid;     /**< specific JID for login */
} CARAInfo_t;


/**
 * Hold global variables for CA layer (also used by RI layer)
 */
typedef struct
{
    int fd;        /**< socket fd */
    uint16_t port; /**< socket port */
} CASocket_t;

#define HISTORYSIZE (4)

typedef struct
{
    CATransportFlags_t flags;
    uint16_t messageId;
    char token[CA_MAX_TOKEN_LEN];
    uint8_t tokenLength;
} CAHistoryItem_t;

typedef struct
{
    int nextIndex;
    CAHistoryItem_t items[HISTORYSIZE];
} CAHistory_t;

/**
 * Hold interface index for keeping track of comings and goings
 */
typedef struct
{
    int32_t ifIndex; /**< network interface index */
} CAIfItem_t;

typedef struct
{
    CATransportFlags_t clientFlags; /**< flag for client */
    CATransportFlags_t serverFlags; /**< flag for server */
    bool client; /**< client mode */
    bool server; /**< server mode */

    struct sockets
    {
        void *threadpool;   /**< threadpool between Initialize and Start */
        CASocket_t u6;      /**< unicast   IPv6 */
        CASocket_t u6s;     /**< unicast   IPv6 secure */
        CASocket_t u4;      /**< unicast   IPv4 */
        CASocket_t u4s;     /**< unicast   IPv4 secure */
        CASocket_t m6;      /**< multicast IPv6 */
        CASocket_t m6s;     /**< multicast IPv6 secure */
        CASocket_t m4;      /**< multicast IPv4 */
        CASocket_t m4s;     /**< multicast IPv4 secure */
        int netlinkFd;      /**< netlink */
        int shutdownFds[2]; /**< shutdown pipe */
        int selectTimeout;  /**< in seconds */
        int maxfd;          /**< highest fd (for select) */
        bool started;       /**< the IP adapter has started */
        bool terminate;     /**< the IP adapter needs to stop */
        bool ipv6enabled;   /**< IPv6 enabled by OCInit flags */
        bool ipv4enabled;   /**< IPv4 enabled by OCInit flags */
        bool dualstack;     /**< IPv6 and IPv4 enabled */

        struct networkmonitors
        {
            CAIfItem_t *ifItems; /**< current network interface index list */
            size_t sizeIfItems;  /**< size of network interface index array */
            size_t numIfItems;   /**< number of valid network interfaces */
        } nm;
    } ip;

    struct calayer
    {
        CAHistory_t requestHistory;  /**< filter IP family in requests */
        CAHistory_t responseHistory; /**< filter IP family in responses */
        CATransportFlags_t previousRequestFlags;/**< address family filtering */
        uint16_t previousRequestMessageId;      /**< address family filtering */
    } ca;

#ifdef TCP_ADAPTER
    /**
     * Hold global variables for TCP Adapter.
     */
    struct tcpsockets
    {
        void *threadpool;       /**< threadpool between Initialize and Start */
        void *svrlist;          /**< unicast IPv4 TCP server information*/
        int selectTimeout;      /**< in seconds */
        int listenBacklog;      /**< backlog counts*/
        int maxfd;              /**< highest fd (for select) */
        bool started;           /**< the TCP adapter has started */
        bool terminate;         /**< the TCP adapter needs to stop */
        bool ipv4tcpenabled;    /**< IPv4 TCP enabled by OCInit flags */
    } tcp;
#endif
} CAGlobals_t;

extern CAGlobals_t caglobals;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // CA_COMMON_H_
