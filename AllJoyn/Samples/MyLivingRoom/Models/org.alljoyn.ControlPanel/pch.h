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
#define PROJECT_NAMESPACE org::alljoyn::ControlPanel
#include "ControlPanelStructures.h"
#include "ContainerStructures.h"
#include "PropertyStructures.h"
#include "TypeConversionHelpers.h"

#include "ControlPanelMethodResults.h"
#include "ControlPanelEventArgs.h"
#include "IControlPanelService.h"
#include "ControlPanelSignals.h"
#include "ControlPanelProducer.h"
#include "ControlPanelWatcher.h"
#include "ControlPanelConsumer.h"

#include "ContainerMethodResults.h"
#include "ContainerEventArgs.h"
#include "IContainerService.h"
#include "ContainerSignals.h"
#include "ContainerProducer.h"
#include "ContainerWatcher.h"
#include "ContainerConsumer.h"

#include "PropertyMethodResults.h"
#include "PropertyEventArgs.h"
#include "IPropertyService.h"
#include "PropertySignals.h"
#include "PropertyProducer.h"
#include "PropertyWatcher.h"
#include "PropertyConsumer.h"

