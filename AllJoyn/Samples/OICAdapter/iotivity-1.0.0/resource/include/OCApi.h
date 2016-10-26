//******************************************************************
//
// Copyright 2014 Intel Mobile Communications GmbH All Rights Reserved.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#ifndef __INTEL_OCAPI_H_2014_07_10
#define __INTEL_OCAPI_H_2014_07_10

#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <memory>
#include <iterator>
#include <functional>

#include "octypes.h"
#include "OCHeaderOption.h"
#include <OCException.h>
#include "StringConstants.h"
#include "oc_logger.hpp"

#include <OCRepresentation.h>

namespace OC
{
    class OCResource;
    class OCResourceRequest;
    class OCResourceResponse;
} // namespace OC

namespace OC
{
    typedef boost::iostreams::stream<OC::oc_log_stream>     log_target_t;

    namespace detail
    {
        /* We'll want to provide some sort of explicit hook for custom logging at some
        point; until then, this should do nicely (note that since these are lambdas,
        later a special target could be captured, allowing much flexibility): */
        static auto oclog_target = []() -> log_target_t&
        {
            static OC::oc_log_stream    ols(oc_make_ostream_logger);
            static log_target_t         os(ols);

            return os;
        };
    } // namespace OC::detail

    static auto oclog = []() -> boost::iostreams::stream<OC::oc_log_stream>&
    {
        return detail::oclog_target();
    };

} // namespace OC

namespace OC
{

    enum class OCPlatformStatus
    {
        PlatformUp,
        PlatformDown
    };

    enum class OCAdvertisementStatus
    {
        None
    };

    typedef std::string URI;

    enum class ServiceType
    {
        InProc,
        OutOfProc
    };

    /**
     * Host Mode of Operation.
     */
    enum class ModeType
    {
        Server,
        Client,
        Both,
        Gateway  /**< Client server mode along with routing capabilities.*/
    };

    /**
     * Quality of Service attempts to abstract the guarantees provided by the underlying transport
     * protocol. The precise definitions of each quality of service level depend on the
     * implementation. In descriptions below are for the current implementation and may changed
     * over time.
     */
    enum class QualityOfService : uint8_t
    {
        /** Packet delivery is best effort. */
        LowQos      = OC_LOW_QOS,

        /** Packet delivery is best effort. */
        MidQos      = OC_MEDIUM_QOS,

        /** Acknowledgments are used to confirm delivery. */
        HighQos     = OC_HIGH_QOS,

        /** No Quality is defined, let the stack decide. */
        NaQos       = OC_NA_QOS
    };

    /**
     *  Data structure to provide the configuration.
     */
    struct PlatformConfig
    {
        /** indicate InProc or OutOfProc. */
        ServiceType                serviceType;

        /** indicate whether we want to do server, client or both. */
        ModeType                   mode;

        /** default flags for server. */
        OCConnectivityType         serverConnectivity;

        /** default flags for client. */
        OCConnectivityType         clientConnectivity;

        /** not used. */
        std::string                ipAddress;

        /** not used. */
        uint16_t                   port;

        /** indicate Quality of Service : LowQos, MidQos,HighQos and NaQos(No quality Defined). */
        QualityOfService           QoS;

        /** persistant storage Handler structure (open/read/write/close/unlink). */
        OCPersistentStorage        *ps;

        public:
            PlatformConfig()
                : serviceType(ServiceType::InProc),
                mode(ModeType::Both),
                serverConnectivity(CT_DEFAULT),
                clientConnectivity(CT_DEFAULT),
                ipAddress("0.0.0.0"),
                port(0),
                QoS(QualityOfService::NaQos),
                ps(nullptr)
        {}
            PlatformConfig(const ServiceType serviceType_,
            const ModeType mode_,
            OCConnectivityType serverConnectivity_,
            OCConnectivityType clientConnectivity_,
            const QualityOfService QoS_,
            OCPersistentStorage *ps_ = nullptr)
                : serviceType(serviceType_),
                mode(mode_),
                serverConnectivity(serverConnectivity_),
                clientConnectivity(clientConnectivity_),
                ipAddress(""),
                port(0),
                QoS(QoS_),
                ps(ps_)
        {}
            // for backward compatibility
            PlatformConfig(const ServiceType serviceType_,
            const ModeType mode_,
            const std::string& ipAddress_,
            const uint16_t port_,
            const QualityOfService QoS_,
            OCPersistentStorage *ps_ = nullptr)
                : serviceType(serviceType_),
                mode(mode_),
                serverConnectivity(CT_DEFAULT),
                clientConnectivity(CT_DEFAULT),
                ipAddress(ipAddress_),
                port(port_),
                QoS(QoS_),
                ps(ps_)
        {}
    };

    enum RequestHandlerFlag
    {
        RequestFlag = 1 << 1,
        ObserverFlag = 1 << 2
    };

    enum class ObserveType
    {
        Observe,
        ObserveAll
    };
    //
    // Typedef for header option vector
    // OCHeaderOption class is in HeaderOption namespace
    typedef std::vector<HeaderOption::OCHeaderOption> HeaderOptions;

    // Typedef for query parameter map
    typedef std::map<std::string, std::string> QueryParamsMap;

    // Typedef for list of observation IDs
    typedef std::vector<OCObservationId> ObservationIds;

    enum class ObserveAction
    {
        ObserveRegister,
        ObserveUnregister
    };

    typedef struct
    {
        // Action associated with observation request
        ObserveAction action;
        // Identifier for observation being registered/unregistered
        OCObservationId obsId;

        OCConnectivityType connectivityType;
        std::string address;
        uint16_t port;
    } ObservationInfo;

    // const strings for different interfaces

    // Default interface
    const std::string DEFAULT_INTERFACE = "oic.if.baseline";

    // Used in discovering (GET) links to other resources of a collection.
    const std::string LINK_INTERFACE = "oic.if.ll";

    // Used in GET, PUT, POST, DELETE methods on links to other resources of a collection.
    const std::string BATCH_INTERFACE = "oic.if.b";

    // Used in GET, PUT, POST methods on links to other remote resources of a group.
    const std::string GROUP_INTERFACE = "oic.mi.grp";


    typedef std::function<void(std::shared_ptr<OCResource>)> FindCallback;

    typedef std::function<void(const OCRepresentation&)> FindDeviceCallback;

    typedef std::function<void(const OCRepresentation&)> FindPlatformCallback;

    typedef std::function<OCEntityHandlerResult(
                            const std::shared_ptr<OCResourceRequest>)> EntityHandler;

    typedef std::function<void(OCStackResult, const unsigned int,
                                const std::string&)> SubscribeCallback;

    typedef std::function<void(const HeaderOptions&,
                                const OCRepresentation&, const int)> GetCallback;

    typedef std::function<void(const HeaderOptions&,
                                const OCRepresentation&, const int)> PostCallback;

    typedef std::function<void(const HeaderOptions&,
                                const OCRepresentation&, const int)> PutCallback;

    typedef std::function<void(const HeaderOptions&, const int)> DeleteCallback;

    typedef std::function<void(const HeaderOptions&,
                                const OCRepresentation&, const int, const int)> ObserveCallback;
} // namespace OC

#endif
