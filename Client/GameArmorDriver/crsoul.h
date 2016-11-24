#pragma once

#include "public.h"

typedef struct _GAMEARMOR_EXTENSION {
	HANDLE ProcessHandle;
	ULONG ProcessId;
} GAMEARMOR_EXTENSION, *PGAMEARMOR_EXTENSION;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(GAMEARMOR_EXTENSION, ControlGetData)

#define GAMEARMOR_DEVICE_NAME_STRING L"\\Device\\GameArmor"
#define GAMEARMOR_LINK_NAME_STRING L"\\DosDevice\\GameArmor"

#ifdef __cplusplus
extern "C"
{
#endif

NTSTATUS DriverEntry(IN OUT PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath);

NTSTATUS GameArmorDeviceAdd(IN WDFDRIVER Driver, IN PWDFDEVICE_INIT DeviceInit);

EVT_WDF_DRIVER_UNLOAD GameArmorEvtDriverUnload;

EVT_WDF_DEVICE_SHUTDOWN_NOTIFICATION GameArmorShutdown;

EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL FileEvtIoDeviceControl;
EVT_WDF_IO_QUEUE_IO_READ FileEvtIoRead;
EVT_WDF_IO_QUEUE_IO_WRITE FileEvtIoWrite;

EVT_WDF_DEVICE_FILE_CREATE GameArmorEvtDeviceFileCreate;
EVT_WDF_FILE_CLOSE GameArmorEvtFileClose;

#ifdef __cplusplus
}
#endif
