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
#define PROJECT_NAMESPACE org::allseen::LSF
#include "LampServiceStructures.h"
#include "LampParametersStructures.h"
#include "LampDetailsStructures.h"
#include "LampStateStructures.h"
#include "TypeConversionHelpers.h"

#include "LampServiceMethodResults.h"
#include "LampServiceEventArgs.h"
#include "ILampServiceService.h"
#include "LampServiceSignals.h"
#include "LampServiceProducer.h"
#include "LampServiceWatcher.h"
#include "LampServiceConsumer.h"

#include "LampParametersMethodResults.h"
#include "LampParametersEventArgs.h"
#include "ILampParametersService.h"
#include "LampParametersSignals.h"
#include "LampParametersProducer.h"
#include "LampParametersWatcher.h"
#include "LampParametersConsumer.h"

#include "LampDetailsMethodResults.h"
#include "LampDetailsEventArgs.h"
#include "ILampDetailsService.h"
#include "LampDetailsSignals.h"
#include "LampDetailsProducer.h"
#include "LampDetailsWatcher.h"
#include "LampDetailsConsumer.h"

#include "LampStateMethodResults.h"
#include "LampStateEventArgs.h"
#include "ILampStateService.h"
#include "LampStateSignals.h"
#include "LampStateProducer.h"
#include "LampStateWatcher.h"
#include "LampStateConsumer.h"

