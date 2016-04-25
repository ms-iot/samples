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

#ifndef COMMON_UTILS_ATOMICHELPER_H
#define COMMON_UTILS_ATOMICHELPER_H

#include <atomic>
#include <boost/version.hpp>

#ifdef __GNUC__
#define RCS_GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif

#if defined(RCS_GCC_VERSION) && RCS_GCC_VERSION >= 40700
#define RCS_USE_STD_ATOMIC
#elif BOOST_VERSION >= 105300
#include <boost/atomic.hpp>
#define RCS_USE_BOOST_ATOMIC
#else
#define RCS_USE_CUSTOM_ATOMIC
#endif

namespace OIC
{
    namespace Service
    {
        namespace Detail
        {

            class ScopedLock
            {
            public:
                explicit ScopedLock(std::atomic_flag& flag) noexcept :
                        m_flag(flag)
                {
                    while (m_flag.test_and_set(std::memory_order_acquire));
                }

                ~ScopedLock() noexcept
                {
                    m_flag.clear(std::memory_order_release);
                }

                ScopedLock(const ScopedLock&) = delete;
                ScopedLock& operator=(const ScopedLock&) = delete;

            private:
                std::atomic_flag& m_flag;
            };

            template< typename T >
            class CustomAtomic
            {
            public:
                CustomAtomic() noexcept :
                    m_value { },
                    m_flag { ATOMIC_FLAG_INIT }
                {

                }
                ~CustomAtomic() = default;

                CustomAtomic(const CustomAtomic&) = delete;

                CustomAtomic& operator=(const CustomAtomic&) = delete;
                CustomAtomic& operator=(const CustomAtomic&) volatile = delete;

                explicit CustomAtomic(T v) noexcept :
                    m_value{ v },
                    m_flag { ATOMIC_FLAG_INIT }
                {
                }

                operator T() const noexcept
                {
                    return load();
                }

                operator T() const volatile noexcept
                {
                    return load();
                }

                T operator=(T v) noexcept
                {
                    store(v);
                    return v;
                }

                T operator=(T v) volatile noexcept
                {
                    store(v);
                    return v;
                }

                bool is_lock_free() const noexcept
                {
                    return false;
                }

                bool is_lock_free() const volatile noexcept
                {
                    return false;
                }

                void store(T v, std::memory_order = std::memory_order_seq_cst) noexcept
                {
                    ScopedLock guard(m_flag);
                    memcpy(&m_value, &v, sizeof(T));
                }

                void store(T v, std::memory_order = std::memory_order_seq_cst) volatile noexcept
                {
                    ScopedLock guard(m_flag);
                    memcpy(&m_value, &v, sizeof(T));
                }

                T load(std::memory_order = std::memory_order_seq_cst) const noexcept
                {
                    ScopedLock guard(m_flag);

                    T v;
                    memcpy(&v, &m_value, sizeof(T));
                    return v;
                }

                T load(std::memory_order = std::memory_order_seq_cst) const volatile noexcept
                {
                    ScopedLock guard(m_flag);

                    T v;
                    memcpy(&v, &m_value, sizeof(T));
                    return v;
                }

                T exchange(T v, std::memory_order = std::memory_order_seq_cst) noexcept
                {
                    ScopedLock guard(m_flag);

                    T tmp;
                    memcpy(&tmp, &m_value, sizeof(T));

                    memcpy(&m_value, &v, sizeof(T));
                    return tmp;
                }

                T exchange(T v, std::memory_order = std::memory_order_seq_cst) volatile noexcept
                {
                    ScopedLock guard(m_flag);

                    T tmp;
                    memcpy(&tmp, &m_value, sizeof(T));

                    memcpy(&m_value, &v, sizeof(T));
                    return tmp;
                }

                bool compare_exchange_weak(T& expected, T desired, std::memory_order success,
                        std::memory_order failure) volatile noexcept
                {
                    return compare_exchange_strong(expected, desired, success, failure);
                }

                bool compare_exchange_weak(T& expected, T desired,
                        std::memory_order m = std::memory_order_seq_cst) volatile noexcept
                {
                    return compare_exchange_weak(expected, desired, m, m);
                }

                bool compare_exchange_strong(T& expected, T desired, std::memory_order /*success*/,
                        std::memory_order /*failure*/) volatile noexcept
                {
                    ScopedLock guard(m_flag);

                    if (memcmp(&m_value, &expected, sizeof(T)) == 0) {
                        memcpy(&m_value, &desired, sizeof(T));
                        return true;
                    } else {
                        memcpy(&expected, &m_value, sizeof(T));
                        return false;
                    }
                }

                bool compare_exchange_strong(T& expected, T desired,
                        std::memory_order m = std::memory_order_seq_cst) volatile noexcept
                {
                    return compare_exchange_strong(expected, desired, m, m);
                }

            private:
                T m_value;
                mutable std::atomic_flag m_flag;
            };

#if defined(RCS_USE_BOOST_ATOMIC)
            inline boost::memory_order convertMemoryOrder(std::memory_order m) noexcept
            {
                switch(m)
                {
                    case std::memory_order_relaxed: return boost::memory_order_relaxed;
                    case std::memory_order_consume: return boost::memory_order_consume;
                    case std::memory_order_acquire: return boost::memory_order_acquire;
                    case std::memory_order_release: return boost::memory_order_release;
                    case std::memory_order_acq_rel: return boost::memory_order_acq_rel;
                    case std::memory_order_seq_cst: return boost::memory_order_seq_cst;
                }
                return boost::memory_order_seq_cst;
            }
#else
            inline std::memory_order convertMemoryOrder(std::memory_order m) noexcept
            {
                return m;
            }
#endif

        } // namespace detail

        template < typename T >
        struct AtomicBase
        {
#if defined(RCS_USE_STD_ATOMIC)
        typedef std::atomic< T > type;
#elif defined(RCS_USE_BOOST_ATOMIC)
        typedef boost::atomic< T > type;
#else
        typedef typename std::conditional< std::is_integral< T >::value, std::atomic< T >,
                Detail::CustomAtomic< T > >::type type;
#endif
        };

        template< typename T >
        class AtomicWrapper
        {
        public:
            AtomicWrapper() = default;

            AtomicWrapper(T v) :
                m_base{ v }
            {
            }

            AtomicWrapper(const AtomicWrapper&) = delete;
            AtomicWrapper& operator=(const AtomicWrapper&) = delete;

            operator T() const noexcept
            {
                return load();
            }

            operator T() const volatile noexcept
            {
                return load();
            }

            T operator=(T v) noexcept
            {
                store(v);
                return v;
            }

            T operator=(T v) volatile noexcept
            {
                store(v);
                return v;
            }

            T load(std::memory_order order = std::memory_order_seq_cst) const noexcept
            {
                return m_base.load(Detail::convertMemoryOrder(order));
            }

            T load(std::memory_order order = std::memory_order_seq_cst) const volatile noexcept
            {
                return m_base.load(Detail::convertMemoryOrder(order));
            }

            void store(T v, std::memory_order order = std::memory_order_seq_cst) noexcept
            {
                m_base.store(v, Detail::convertMemoryOrder(order));
            }

        private:
            typename AtomicBase< T >::type m_base;
        };

    }
}

#endif // COMMON_UTILS_ATOMICHELPER_H
