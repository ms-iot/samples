#pragma once

namespace org { namespace alljoyn { namespace ControlPanel {

ref class ControlPanelSignals;

public interface class IControlPanelSignals
{
};

public ref class ControlPanelSignals sealed : [Windows::Foundation::Metadata::Default] IControlPanelSignals
{
public:
internal:
    void Initialize(_In_ alljoyn_busobject busObject, _In_ alljoyn_sessionid sessionId);

private:
    alljoyn_busobject m_busObject;
    alljoyn_sessionid m_sessionId;

};

} } } 