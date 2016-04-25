#pragma once

namespace org { namespace allseen { namespace LSF {

ref class LampDetailsSignals;

public interface class ILampDetailsSignals
{
};

public ref class LampDetailsSignals sealed : [Windows::Foundation::Metadata::Default] ILampDetailsSignals
{
public:
internal:
    void Initialize(_In_ alljoyn_busobject busObject, _In_ alljoyn_sessionid sessionId);

private:
    alljoyn_busobject m_busObject;
    alljoyn_sessionid m_sessionId;

};

} } } 