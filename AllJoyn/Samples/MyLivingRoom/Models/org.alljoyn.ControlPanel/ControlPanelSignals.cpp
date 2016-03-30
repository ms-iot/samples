#include "pch.h"

using namespace concurrency;
using namespace Microsoft::WRL;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Devices::AllJoyn;
using namespace org::alljoyn::ControlPanel;

void ControlPanelSignals::Initialize(_In_ alljoyn_busobject busObject, _In_ alljoyn_sessionid sessionId)
{
    m_busObject = busObject;
    m_sessionId = sessionId;

}

