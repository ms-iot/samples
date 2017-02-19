#include "pch.h"
#include "LSFSignalHandler.h"
#include "LampState.h"

using namespace BridgeRT;

LSFSignalHandler::LSFSignalHandler(LampState* pLampState)
    : m_pLampState(pLampState)
{
}

LSFSignalHandler::~LSFSignalHandler()
{
}

void
LSFSignalHandler::AdapterSignalHandler(
    _In_ IAdapterSignal^ Signal,
    _In_opt_ Platform::Object^ Context)
{
    UNREFERENCED_PARAMETER(Signal);
    UNREFERENCED_PARAMETER(Context);

    m_pLampState->RaiseStateChangedSignal();
}
