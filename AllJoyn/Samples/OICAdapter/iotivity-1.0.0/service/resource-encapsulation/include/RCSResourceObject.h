//******************************************************************
//
// Copyright 2015 Samsung Electronics All Rights Reserved.
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

/**
 * @file
 *
 * This file contains the declaration of classes and its members related to RCSResourceObject
 */
#ifndef SERVER_RCSRESOURCEOBJECT_H
#define SERVER_RCSRESOURCEOBJECT_H

#include <string>
#include <mutex>
#include <thread>

#include <RCSResourceAttributes.h>
#include <RCSResponse.h>
#include <RCSRequest.h>

namespace OC
{
    class OCResourceRequest;
}

namespace OIC
{
    namespace Service
    {

        /**
         * @brief Thrown when lock has not been acquired.
         *
         * @see RCSResourceObject::LockGuard
         * @see RCSResourceObject::getAttributes
         */
        class NoLockException: public RCSException
        {
            public:
                NoLockException(std::string &&what) : RCSException { std::move(what) } {}
        };

        //! @cond
        template < typename T >
        class AtomicWrapper;
        //! @endcond

        /**
         * RCSResourceObject represents a resource and handles any requests from clients
         * automatically with attributes.
         *
         * It also provides an auto notification mechanism that notifies to the observers.
         *
         * Requests are handled automatically by defaultAction of RCSGetResponse and
         * RCSSetResponse. You can override them and send your own response.
         * <p>
         * For simple resources, they are simply required to notify whenever attributes are changed
         * by a set request. In this case, add an AttributeUpdatedListener with a key interested
         * in instead of overriding SetRequestHandler.
         * </p>
         */
        class RCSResourceObject
        {
            private:
                class WeakGuard;

                typedef AtomicWrapper< std::thread::id > AtomicThreadId;

            public:
                /**
                 * Represents the policy of auto-notify function.
                 * In accord with this policy, observers are notified of attributes
                 * when the attributes are set.
                 *
                 * @note Attributes are set according to the execution of some functions which
                 * modify attributes or receipt of set requests.
                 *
                 * @see RCSResourceObject::setAttribute
                 * @see RCSResourceObject::removeAttribute
                 * @see RCSResourceObject::getAttributes
                 * @see RCSResourceObject::LockGuard
                 */
                enum class AutoNotifyPolicy
                {
                    NEVER,  /**< Never*/
                    ALWAYS, /**< Always*/
                    UPDATED /**< Only when attributes are changed*/
                };

                /**
                 * Represents the policy of set-request handler.
                 * In accord with this, the RCSResourceObject decides whether a set-request is
                 * acceptable or not.
                 */
                enum class SetRequestHandlerPolicy
                {
                    NEVER,     /**< Requests will be ignored if attributes of the request contain
                                    a new key or a value that has different type from the current
                                    value of the key. */
                    ACCEPTANCE /**< The attributes of the request will be applied unconditionally
                                    even if there are new name or type conflicts. */
                };

                typedef std::shared_ptr< RCSResourceObject > Ptr;
                typedef std::shared_ptr< const RCSResourceObject > ConstPtr;

                /**
                 * This is a builder to create resource with properties and attributes.
                 *
                 * The resource will be observable and discoverable by default, to make them disable
                 * set these properties explicitly with setDiscoverable and setObservable.
                 */
                class Builder
                {
                    public:
                        /**
                         * Constructs a Builder.
                         *
                         * @param uri Resource uri
                         * @param type Resource type
                         * @param interface Resource interface
                         *
                         */
                        Builder(const std::string& uri, const std::string& type,
                                const std::string& interface);

                        /**
                         * Sets whether the resource is discoverable.
                         *
                         * @param discoverable whether to be discoverable.
                         *
                         */
                        Builder& setDiscoverable(bool discoverable);

                        /**
                         * Sets the observable property of the resource.
                         *
                         * @param observable whether to be observable.
                         *
                         */
                        Builder& setObservable(bool observable);

                        /**
                         * Sets attributes for the resource.
                         *
                         * @param attributes attributes to set
                         *
                         */
                        Builder& setAttributes(const RCSResourceAttributes &attributes);

                        /**
                         * @overload
                         */
                        Builder& setAttributes(RCSResourceAttributes &&attributes);

                        /**
                         * Register a resource and returns a RCSResourceObject.
                         *
                         * @throw RCSPlatformException if resource registration is failed.
                         *
                         */
                        RCSResourceObject::Ptr build();

                    private:
                        std::string m_uri;
                        std::string m_type;
                        std::string m_interface;
                        uint8_t m_properties;
                        RCSResourceAttributes m_resourceAttributes;
                };

                class LockGuard;

                /**
                 * Callback definition for a handler to be invoked when a get request is received.
                 *
                 * The handler will be called first when a get request is received, before the
                 * RCSResourceObject handles.
                 *
                 * @param request the request information
                 * @param attributes attributes of the request
                 *
                 * @return response to be sent and that indicates how the request to be handled by
                 *         the RCSResourceObject.
                 *
                 * @see setGetRequestHandler
                 */
                typedef std::function < RCSGetResponse(const RCSRequest& request,
                        RCSResourceAttributes& attributes) > GetRequestHandler;

                /**
                 * Callback definition for a handler to be invoked when a set request is received.
                 *
                 * The handler will be called first when a get request is received, before the
                 * RCSResourceObject handles. If the attributes are modified in the callback,
                 * the modified attributes will be set in the RCSResourceObject if the request is
                 * not ignored.
                 *
                 * @param request the request information
                 * @param attributes attributes of the request
                 *
                 * @return response to be sent and that indicates how the request to be handled by
                 *         the RCSResourceObject.
                 *
                 * @see setGetRequestHandler
                 */
                typedef std::function < RCSSetResponse(const RCSRequest& request,
                        RCSResourceAttributes& attributes) > SetRequestHandler;

                /**
                 * Callback definition to be invoked when an attribute is updated.
                 *
                 * @param oldValue the value before being changed
                 * @param newValue changed value
                 */
                typedef std::function < void(const RCSResourceAttributes::Value& oldValue,
                            const RCSResourceAttributes::Value& newValue) > AttributeUpdatedListener;

            public:
                RCSResourceObject(RCSResourceObject&&) = delete;
                RCSResourceObject(const RCSResourceObject&) = delete;

                RCSResourceObject& operator=(RCSResourceObject&&) = delete;
                RCSResourceObject& operator=(const RCSResourceObject&) = delete;

                virtual ~RCSResourceObject();

                /**
                 * Sets a particular attribute value.
                 *
                 * @param key key of attribute
                 * @param value value to be mapped against the key
                 *
                 * @note Thread-safety is guaranteed for the attributes.
                 */
                void setAttribute(const std::string& key, const RCSResourceAttributes::Value& value);

                /**
                 * @overload
                 */
                void setAttribute(const std::string& key, RCSResourceAttributes::Value&& value);

                /**
                 * @overload
                 */
                void setAttribute(std::string&& key, const RCSResourceAttributes::Value& value);

                /**
                 * @overload
                 */
                void setAttribute(std::string&& key, RCSResourceAttributes::Value&& value);

                /**
                 * Returns an attribute value corresponding to a key.
                 *
                 * @param key key of the attribute
                 *
                 * @throws RCSInvalidKeyException If key is invalid.
                 *
                 * @note Thread-safety is guaranteed for the attributes.
                 */
                RCSResourceAttributes::Value getAttributeValue(const std::string& key) const;

                /**
                 * Returns the attribute value as T.
                 *
                 * @param key key of the attribute
                 *
                 * @throws RCSBadGetException If type of the underlying value is not T.
                 * @throws RCSInvalidKeyException If @a key doesn't match the key of any value.
                 *
                 * @note Thread-safety is guaranteed for the attributes.
                 */
                template< typename T >
                T getAttribute(const std::string& key) const
                {
                    WeakGuard lock(*this);
                    return m_resourceAttributes.at(key).get< T >();
                }

                /**
                 * Removes a particular attribute of the resource.
                 *
                 * @param key key of the attribute.
                 *
                 * @return True if the key exists and matched attribute is removed, otherwise false.
                 *
                 * @note Thread-safety is guaranteed for the attributes.
                 */
                bool removeAttribute(const std::string& key);

                /**
                 * Checks whether a particular attribute exists or not.
                 *
                 * @param key key of the attribute
                 *
                 * @return True if the key exists, otherwise false.
                 *
                 * @note Thread-safety is guaranteed for the attributes.
                 */
                bool containsAttribute(const std::string& key) const;

                /**
                 * Returns reference to the attributes of the RCSResourceObject.
                 *
                 * @pre The call must be guarded by LockGuard.
                 *
                 *
                 * @return Reference to the attributes
                 *
                 * @throws NoLockException If the call is not guarded by LockGuard.
                 *
                 * @note Here is the standard idiom for LockGuard:
                 * @code
                   {
                      RCSResourceObject::LockGuard lock(rcsResourceObject);

                      auto &attributes = server->getAttributes();
                      ...
                   }
                 * @endcode
                 */
                RCSResourceAttributes& getAttributes();

                /**
                 * @overload
                 */
                const RCSResourceAttributes& getAttributes() const;

                /**
                 * Checks whether the resource is observable or not.
                 */
                virtual bool isObservable() const;

                /**
                 * Checks whether the resource is discoverable or not.
                 */
                virtual bool isDiscoverable() const;

                /**
                 * Sets the get request handler.
                 * To remove handler, pass empty handler or nullptr.
                 *
                 * Default behavior is RCSGetResponse::defaultAction().
                 *
                 * @param handler a get request handler
                 *
                 * @see RCSGetResponse
                 *
                 */
                virtual void setGetRequestHandler(GetRequestHandler handler);

                /**
                 * Sets the set request handler.
                 * To remove handler, pass empty handler or nullptr.
                 *
                 * Default behavior is RCSSetResponse::defaultAction().
                 *
                 * @param handler a set request handler
                 *
                 * @see RCSSetResponse
                 *
                 */
                virtual void setSetRequestHandler(SetRequestHandler handler);

                /**
                 * Adds a listener for a particular attribute updated.
                 *
                 * @param key the interested attribute's key
                 * @param listener listener to be invoked
                 *
                 */
                virtual void addAttributeUpdatedListener(const std::string& key,
                        AttributeUpdatedListener listener);

                /**
                 * @overload
                 */
                virtual void addAttributeUpdatedListener(std::string&& key,
                        AttributeUpdatedListener listener);

                /**
                 * Removes a listener for a particular attribute updated.
                 *
                 * @param key the key associated with the listener to be removed
                 *
                 * @return True if the listener added with same key exists and is removed.
                 *
                 */
                virtual bool removeAttributeUpdatedListener(const std::string& key);

                /**
                 * Notifies all observers of the current attributes.
                 *
                 * @throws RCSPlatformException If the operation failed.
                 */
                virtual void notify() const;

                /**
                 * Sets auto notify policy
                 *
                 * @param policy policy to be set
                 *
                 */
                void setAutoNotifyPolicy(AutoNotifyPolicy policy);

                /**
                 * Returns the current policy
                 *
                 */
                AutoNotifyPolicy getAutoNotifyPolicy() const;

                /**
                 * Sets the policy for handling a set request.
                 *
                 * @param policy policy to be set
                 *
                 */
                void setSetRequestHandlerPolicy(SetRequestHandlerPolicy policy);

                /**
                 * Returns the current policy.
                 *
                 */
                SetRequestHandlerPolicy getSetRequestHandlerPolicy() const;

        private:
            RCSResourceObject(uint8_t, RCSResourceAttributes&&);

            OCEntityHandlerResult entityHandler(std::shared_ptr< OC::OCResourceRequest >);

            OCEntityHandlerResult handleRequest(std::shared_ptr< OC::OCResourceRequest >);
            OCEntityHandlerResult handleRequestGet(std::shared_ptr< OC::OCResourceRequest >);
            OCEntityHandlerResult handleRequestSet(std::shared_ptr< OC::OCResourceRequest >);
            OCEntityHandlerResult handleObserve(std::shared_ptr< OC::OCResourceRequest >);

            void expectOwnLock() const;

            std::thread::id getLockOwner() const noexcept;

            void setLockOwner(std::thread::id&&) const noexcept;

            void autoNotify(bool, AutoNotifyPolicy) const;
            void autoNotify(bool) const;

            bool testValueUpdated(const std::string&, const RCSResourceAttributes::Value&) const;

            template< typename K, typename V >
            void setAttributeInternal(K&&, V&&);

            bool applyAcceptanceMethod(const RCSSetResponse&, const RCSResourceAttributes&);

        private:
            const uint8_t m_properties;

            OCResourceHandle m_resourceHandle;

            RCSResourceAttributes m_resourceAttributes;

            GetRequestHandler m_getRequestHandler;
            SetRequestHandler m_setRequestHandler;

            AutoNotifyPolicy m_autoNotifyPolicy;
            SetRequestHandlerPolicy m_setRequestHandlerPolicy;

            std::unordered_map< std::string, std::shared_ptr< AttributeUpdatedListener > >
                    m_attributeUpdatedListeners;

            mutable std::unique_ptr< AtomicThreadId > m_lockOwner;
            mutable std::mutex m_mutex;

            std::mutex m_mutexAttributeUpdatedListeners;

        };

        /**
         * The class provides a convenient RAII-style mechanism for the attributes of a
         * RCSResourceObject. When a LockGuard is created, it attempts to lock the attributes of
         * the RCSResourceObject it is given. When control leaves the scope in which the LockGuard
         * object was created, the LockGuard is destructed and the attributes is unlocked.
         *
         * Additionally when this is destructed, it tries to notify depending on AutoNotifyPolicy
         * of the RCSResourceObject.
         */
        class RCSResourceObject::LockGuard
        {
        public:
            LockGuard(const RCSResourceObject& rcsResourceObject);

            LockGuard(const RCSResourceObject::Ptr);

           /**
            * Constructs a LockGuard with auto notify policy.
            *
            * @param object an object to be locked
            * @param autoNotifyPolicy the policy to indicate how auto notification is handled
            *        when the LockGuard is destructed.
            *
            */
            LockGuard(const RCSResourceObject& object, AutoNotifyPolicy autoNotifyPolicy);

           /**
            * @overload
            */
            LockGuard(const RCSResourceObject::Ptr, AutoNotifyPolicy);
            ~LockGuard();

            LockGuard(const LockGuard&) = delete;
            LockGuard(LockGuard&&) = delete;

            LockGuard& operator=(const LockGuard&) = delete;
            LockGuard& operator=(LockGuard&&) = delete;

        private:
            void init();

        private:
            const RCSResourceObject& m_resourceObject;

            AutoNotifyPolicy m_autoNotifyPolicy;

            bool m_isOwningLock;

            std::function<void()> m_autoNotifyFunc;
        };

        //! @cond
        class RCSResourceObject::WeakGuard
        {
        public:
            WeakGuard(const RCSResourceObject&);
            ~WeakGuard();

            WeakGuard(const WeakGuard&) = delete;
            WeakGuard(WeakGuard&&) = delete;

            WeakGuard& operator=(const WeakGuard&) = delete;
            WeakGuard& operator=(WeakGuard&&) = delete;

            bool hasLocked() const;

        private:
            bool m_isOwningLock;
            const RCSResourceObject& m_resourceObject;
        };
        //! @endcond
    }
}

#endif // SERVER_RCSRESOURCEOBJECT_H
