#include <windows.h>
#include <stdio.h>
#include <cfgmgr32.h>
#include "common.h"
#include "initguid.h"
#include "HidDevice.h"

// {F990ABA9-C9C0-4CF8-A4A2-5B06D97F6871}
DEFINE_GUID(GUID_DEVCLASS_HIDINJECTOR,
    0xf990aba9, 0xc9c0, 0x4cf8, 0xa4, 0xa2, 0x5b, 0x6, 0xd9, 0x7f, 0x68, 0x71);

// {59819B74-F102-469A-9009-3CAF35FD4686}
DEFINE_GUID(GUID_DEVINTERFACE_HIDINJECTOR,
    0x59819b74, 0xf102, 0x469a, 0x90, 0x9, 0x3c, 0xaf, 0x35, 0xfd, 0x46, 0x86);

// {FFD216E4-A560-4676-8BD5-4F26A5BFF214}
DEFINE_GUID(GUID_BUS_HIDINJECTOR,
    0xffd216e4, 0xa560, 0x4676, 0x8b, 0xd5, 0x4f, 0x26, 0xa5, 0xbf, 0xf2, 0x14);

HANDLE g_hFile = NULL;

BOOLEAN
FindMatchingDevice(
	_Out_ HANDLE* Handle
	)
{
	CONFIGRET cr = CR_SUCCESS;
	PWSTR deviceInterfaceList = NULL;
	ULONG deviceInterfaceListLength = 0;
	PWSTR currentInterface;
	BOOLEAN bRet = FALSE;
	HANDLE devHandle = INVALID_HANDLE_VALUE;
	GUID DevInterface = GUID_DEVINTERFACE_HIDINJECTOR;

	if (NULL == Handle) {
		printf("Error: Invalid device handle parameter\n");
		return FALSE;
	}

	*Handle = INVALID_HANDLE_VALUE;

	cr = CM_Get_Device_Interface_List_Size(
		&deviceInterfaceListLength,
		&DevInterface,
		NULL,
		CM_GET_DEVICE_INTERFACE_LIST_PRESENT);
	if (cr != CR_SUCCESS) {
		printf("Error 0x%x retrieving device interface list size.\n", cr);
		goto clean0;
	}

	if (deviceInterfaceListLength <= 1) {
		bRet = FALSE;
		printf("Error: No active device interfaces found.\n"
			" Is the sample driver loaded?");
		goto clean0;
	}

	deviceInterfaceList = (PWSTR)malloc(deviceInterfaceListLength * sizeof(WCHAR));
	if (deviceInterfaceList == NULL) {
		printf("Error allocating memory for device interface list.\n");
		goto clean0;
	}
	ZeroMemory(deviceInterfaceList, deviceInterfaceListLength * sizeof(WCHAR));

	cr = CM_Get_Device_Interface_List(
		&DevInterface,
		NULL,
		deviceInterfaceList,
		deviceInterfaceListLength,
		CM_GET_DEVICE_INTERFACE_LIST_PRESENT);
	if (cr != CR_SUCCESS) {
		printf("Error 0x%x retrieving device interface list.\n", cr);
		goto clean0;
	}

	//
	// Enumerate devices of this interface class
	//
	printf("\n....looking for our HID device\n");

	for (currentInterface = deviceInterfaceList;
	*currentInterface;
		currentInterface += wcslen(currentInterface) + 1) {

		printf("Trying %S\n", currentInterface);
		devHandle = CreateFile(currentInterface,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL, // no SECURITY_ATTRIBUTES structure
			OPEN_EXISTING, // No special create flags
			0, // No special attributes
			NULL); // No template file

		if (INVALID_HANDLE_VALUE == devHandle) {
			printf("Warning: CreateFile failed: %d\n", GetLastError());
			continue;
		}

		bRet = TRUE;
		*Handle = devHandle;
	}

clean0:
	if (deviceInterfaceList != NULL) {
		free(deviceInterfaceList);
	}
	if (CR_SUCCESS != cr) {
		bRet = FALSE;
	}
	return bRet;
}

BOOLEAN
FindMatchingDevice2(
	_Out_ HANDLE* Handle
	)
{
	*Handle = CreateFile(L"\\\\.\\HidInjectorSample",
		GENERIC_WRITE,
		0,
		NULL, // no SECURITY_ATTRIBUTES structure
		OPEN_EXISTING, // No special create flags
		0, // No special attributes
		NULL); // No template file

	if (INVALID_HANDLE_VALUE == *Handle) {
		printf("Warning: CreateFile failed: %d\n", GetLastError());
		return FALSE;
	}

	return TRUE;
}

BOOL OpenHidInjectorDevice()
{
	BOOL success = FindMatchingDevice(&g_hFile);
	if (!success)
	{
		g_hFile = NULL;
	}
	return success;
}

void CloseHidInjectorDevice()
{
	if (g_hFile != NULL)
	{
		CloseHandle(g_hFile);
		g_hFile = NULL;
	}
}
