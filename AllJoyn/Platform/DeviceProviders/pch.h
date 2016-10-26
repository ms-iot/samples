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

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <iomanip>
#include <memory>
#include <vector>
#include <set>
#include <string>
#include <sstream>
#include <map>
#include <queue>
#include <functional>

#include <ppltasks.h>
#include <collection.h>

#define QCC_OS_GROUP_WINDOWS

#include <alljoyn_c/Init.h>
#include <alljoyn_c/AboutObjectDescription.h>
#include <alljoyn_c/AboutData.h>
#include <alljoyn_c/AboutIcon.h>
#include <alljoyn_c/AboutProxy.h>
#include <alljoyn_c/BusAttachment.h>
#include <alljoyn_c/ProxyBusObject.h>
#include <alljoyn_c/Status.h>

#include "Locks.h"
#include "DebugLifetime.h"

static const alljoyn_sessionport DEFAULT_SERVICE_PORT = 900;
static const std::string DEFAULT_LANGUAGE = "en";
static const std::string ALLJOYN_APPLICATION_NAME = "AllJoynWinRTProvider";
