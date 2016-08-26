#pragma once

#ifdef __cplusplus
extern "C" {
#endif

	BOOL OpenHidInjectorDevice();
	void CloseHidInjectorDevice();

	extern HANDLE g_hFile;

#ifdef __cplusplus
}	// extern "C"
#endif



