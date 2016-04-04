// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "pch.h"

BOOL APIENTRY DllMain(
    HANDLE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved)
{
    UNREFERENCED_PARAMETER(lpReserved);
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        break;

    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

void WINAPI ServiceMain(DWORD argc, __in_ecount(argc) PTSTR argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv); 

    // See MSDN for writing Service code
    //
    //  https://msdn.microsoft.com/en-us/library/windows/desktop/ms687414(v=vs.85).aspx
    //
}
