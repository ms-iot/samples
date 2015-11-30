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

#include <collection.h>
#include <exception>
#include <string>
#include <ppltasks.h>
#include <algorithm>

#define QCC_OS_GROUP_WINDOWS

#include <alljoyn_c/Init.h>
#include <alljoyn_c/dbusstddefines.h>
#include <alljoyn_c/BusAttachment.h>
#include <alljoyn_c/Session.h>
#include <alljoyn_c/BusListener.h>
#include <alljoyn_c/SessionPortListener.h>
#include <alljoyn_c/InterfaceDescription.h>

#include "Misc.h"
#include "IAdapter.h"

#include <windows.h>
#include <tchar.h>

#include "ControlPanel.h"
#include "Widget.h"

#define CHK_AJSTATUS(x) { status = (x); if ((ER_OK != status))  { goto leave; }}
#define CHK_POINTER(x) {if ((nullptr == x)) { status = ER_OUT_OF_MEMORY; goto leave; }}