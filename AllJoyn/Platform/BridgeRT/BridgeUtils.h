#pragma once
#include <string>
#include <mutex>

namespace BridgeRT
{
    // Assert
    #if _DEBUG || DBG
        #define DSB_ASSERT(_condition_) if (!(_condition_)) \
        { \
            __debugbreak(); \
        }
    #else   // !_DEBUG && !DBG
        #define DSB_ASSERT(_condition_)
    #endif  // _DEBUG

    // Win32 error codes from HRESULT codes.
    #ifndef WIN32_FROM_HRESULT
        __forceinline DWORD WIN32_FROM_HRESULT(HRESULT HResult) { return HRESULT_FACILITY(HResult) == FACILITY_WIN32 ? HRESULT_CODE(HResult) : HResult; }
    #endif

    __forceinline HRESULT HRESULT_FROM_QSTATUS(int x) { return MAKE_HRESULT(1, FACILITY_ITF, x); }

    // Check status-leave helper macro
    #define CHK_HR(stmt) do { hr=(stmt); if (FAILED(hr)) goto CleanUp; } while(0)

    typedef std::lock_guard<std::recursive_mutex> AutoLock;

    template<typename _TDest>
    inline _TDest ConvertTo(const wchar_t* wStrSrc)
    {
        if (wStrSrc == nullptr)
        {
            return _TDest();
        }
        std::wstring src(wStrSrc);
        return _TDest{ src.begin(), src.end() };
    }


    template<typename _TDest, typename _Tsrc>
    inline _TDest ConvertTo(const _Tsrc& src)
    {
        return _TDest{ src.begin(), src.end() };
    }

    template<typename _TDest>
    inline _TDest ConvertTo(Platform::String^ src)
    {
        if (src == nullptr)
        {
            return _TDest();
        }
        return _TDest{ Platform::begin(src), Platform::end(src) };
    }    

    //
    //  Routine Description:
    //      Sets a Platform::String to a formatted (printf like) string.
    //
    //  Arguments:
    //
    //      FormatWsz - String format (printf like)
    //
    //      ... - arguments
    //
    //  Return Value:
    //
    //      The length (in characters) of result string, or -1 if
    //      failed to convert due to resource limitation, or max string size
    //      has been reached.
    //
#define MAX_STRING_SIZE size_t(1024*1024)
    //
    int FormatString(_Out_ Platform::String^& OutputString, _In_z_ const wchar_t* FormatWsz, ...);

}