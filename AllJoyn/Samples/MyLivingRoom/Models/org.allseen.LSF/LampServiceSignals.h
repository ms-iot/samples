#pragma once

namespace org { namespace allseen { namespace LSF {

ref class LampServiceSignals;

public interface class ILampServiceSignals
{
};

public ref class LampServiceSignals sealed : [Windows::Foundation::Metadata::Default] ILampServiceSignals
{
public:
internal:
    void Initialize(_In_ alljoyn_busobject busObject, _In_ alljoyn_sessionid sessionId);

private:
    alljoyn_busobject m_busObject;
    alljoyn_sessionid m_sessionId;

};

} } } 