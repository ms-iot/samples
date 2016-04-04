// Copyright (c) Microsoft. All rights reserved.

//
// pch.h
// Header for standard system include files.
//

#pragma once

#include <collection.h>
#include <ppltasks.h>
#include <strsafe.h>
#include <winioctl.h>

// Undefine symbols to avoid collision with ntddser.h
#undef SERIAL_LSRMST_ESCAPE
#undef SERIAL_LSRMST_LSR_DATA
#undef SERIAL_LSRMST_LSR_NODATA
#undef SERIAL_LSRMST_MST
#undef SERIAL_IOC_FCR_FIFO_ENABLE
#undef SERIAL_IOC_FCR_RCVR_RESET
#undef SERIAL_IOC_FCR_XMIT_RESET
#undef SERIAL_IOC_FCR_DMA_MODE
#undef SERIAL_IOC_FCR_RES1
#undef SERIAL_IOC_FCR_RES2
#undef SERIAL_IOC_FCR_RCVR_TRIGGER_LSB
#undef SERIAL_IOC_FCR_RCVR_TRIGGER_MSB
#undef SERIAL_IOC_MCR_DTR
#undef SERIAL_IOC_MCR_RTS
#undef SERIAL_IOC_MCR_OUT1
#undef SERIAL_IOC_MCR_OUT2
#undef SERIAL_IOC_MCR_LOOP

#include <ntddser.h>
#include <wrl.h>

#include "App.xaml.h"
