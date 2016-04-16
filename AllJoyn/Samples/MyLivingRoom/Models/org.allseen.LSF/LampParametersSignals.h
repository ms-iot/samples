#pragma once

namespace org { namespace allseen { namespace LSF {

ref class LampParametersSignals;

public interface class ILampParametersSignals
{
};

public ref class LampParametersSignals sealed : [Windows::Foundation::Metadata::Default] ILampParametersSignals
{
public:
internal:
    void Initialize(_In_ alljoyn_busobject busObject, _In_ alljoyn_sessionid sessionId);

private:
    alljoyn_busobject m_busObject;
    alljoyn_sessionid m_sessionId;

};

} } } 