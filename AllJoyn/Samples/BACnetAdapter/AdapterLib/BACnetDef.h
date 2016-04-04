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

#pragma prefast(push)
#pragma prefast(disable:28718, "Open source files")

//
// Additional libraries required for BACnet
//
#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")


//
// BACnet stack files
//
#include "bacenum.h"
#include "bacdef.h"
#include "datalink.h"
#include "device.h"
#include "apdu.h"
#include "handlers.h"
#include "address.h"
#include "client.h"
#include "dlenv.h"
#include "iam.h"
#include "tsm.h"
#include "bvlc.h"
#include "bip.h"

#pragma prefast(pop)
