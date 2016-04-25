#pragma once

#include <windows.h>
#include <ppltasks.h>
#include <concrt.h>
#include <collection.h>
#include <windows.devices.alljoyn.interop.h>
#include <map>

#include <alljoyn_c/busattachment.h>
#include <alljoyn_c/dbusstddefines.h>
#include <alljoyn_c/AboutData.h>
#include <alljoyn_c/AboutObj.h>
#include <alljoyn_c/AboutObjectDescription.h>

#include "AllJoynHelpers.h"
#define PROJECT_NAMESPACE org::alljoyn::Control
#include "VolumeStructures.h"
#include "TypeConversionHelpers.h"

#include "VolumeMethodResults.h"
#include "VolumeEventArgs.h"
#include "IVolumeService.h"
#include "VolumeSignals.h"
#include "VolumeProducer.h"
#include "VolumeWatcher.h"
#include "VolumeConsumer.h"

