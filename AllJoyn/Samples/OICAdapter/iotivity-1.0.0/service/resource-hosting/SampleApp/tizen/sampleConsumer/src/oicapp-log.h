/*
 * Copyright (c) 2010 Samsung Electronics, Inc.
 * All rights reserved.
 *
 * This software is a confidential and proprietary information
 * of Samsung Electronics, Inc. ("Confidential Information").  You
 * shall not disclose such Confidential Information and shall use
 * it only in accordance with the terms of the license agreement
 * you entered into with Samsung Electronics.
 */
#ifndef __OICAPP_LOG_H__
#define __OICAPP_LOG_H__

#define TIZEN_DEBUG_ENABLE
#define LOG_TAG "OIC_TEST"
#include <dlog.h>

#define LOG_COLOR_RED       "\033[0;31m"
#define LOG_COLOR_BROWN     "\033[0;33m"
#define LOG_COLOR_BLUE      "\033[0;34m"
#define LOG_COLOR_END       "\033[0;m"


#if 1
#define _DBG(fmt, arg...) SLOGD(fmt, ##arg)
#define _INFO(fmt, arg...) SLOGI(fmt, ##arg)
#define _WARN(fmt, arg...) SLOGW(fmt, ##arg)
#define _ERR(fmt, arg...) SLOGE(fmt, ##arg)
#else
#define _DBG(fmt, arg...) \
    printf("[OIC_TEST]%s(%d):" fmt "\n", __FUNCTION__, __LINE__, ##arg)
#define _INFO(fmt, arg...) \
    printf("[OIC_TEST]%s(%d):" fmt "\n", __FUNCTION__, __LINE__, ##arg)
#define _WARN(fmt, arg...) \
    printf("[OIC_TEST]%s(%d):" fmt "\n", __FUNCTION__, __LINE__, ##arg)
#define _ERR(fmt, arg...) \
    printf("[OIC_TEST]%s(%d):" fmt "\n", __FUNCTION__, __LINE__, ##arg)
#endif

#define DBG(fmt, arg...) _DBG(fmt, ##arg)
#define WARN(fmt, arg...) _WARN(LOG_COLOR_BROWN fmt LOG_COLOR_END, ##arg)
#define ERR(fmt, arg...) _ERR(LOG_COLOR_RED fmt LOG_COLOR_END, ##arg)
#define INFO(fmt, arg...) _INFO(LOG_COLOR_BLUE fmt LOG_COLOR_END, ##arg)

#define ret_if(expr) \
    do { \
        if (expr) { \
            ERR("(%s)", #expr); \
            return; \
        }\
    } while(0)
#define retv_if(expr, val) \
    do {\
        if (expr) { \
            ERR("(%s)", #expr); \
            return (val); \
        } \
    } while(0)
#define retm_if(expr, fmt, arg...) \
    do {\
        if (expr) { \
            ERR(fmt, ##arg); \
            return; \
        }\
    } while(0)
#define retvm_if(expr, val, fmt, arg...) \
    do {\
        if (expr) { \
            ERR(fmt, ##arg); \
            return (val); \
        } \
    } while(0)
#define warn_if(expr) \
    do { \
        if (expr) { \
            WARN("(%s)", #expr); \
        } \
    } while (0)


#endif //__OICAPP_LOG_H__

