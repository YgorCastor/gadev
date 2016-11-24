#include "stdafx.h"
#include "crsoul.h"

extern NTSTATUS GameArmorDeviceAdd(IN WDFDRIVER Driver, IN PWDFDEVICE_INIT DeviceInit);

NTSTATUS DriverEntry(IN OUT PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
{
    NTSTATUS                       status;
    WDF_DRIVER_CONFIG              config;
    WDFDRIVER                      hDriver;
    PWDFDEVICE_INIT                pInit = NULL;
    WDF_OBJECT_ATTRIBUTES          attributes;

	DECLARE_CONST_UNICODE_STRING(
    SDDL_DEVOBJ_SYS_ALL_ADM_RWX_WORLD_RW_RES_R,
    L"D:P(A;;GA;;;SY)(A;;GRGWGX;;;BA)(A;;GRGW;;;WD)(A;;GR;;;RC)"
    );

	DbgPrint("[GameArmor] Loading GameArmor modules...\n");

	WDF_DRIVER_CONFIG_INIT(&config, WDF_NO_EVENT_CALLBACK);

	config.DriverInitFlags |= WdfDriverInitNonPnpDriver;
	config.EvtDriverUnload = GameArmorEvtDriverUnload;

	WDF_OBJECT_ATTRIBUTES_INIT(&attributes);

	status = WdfDriverCreate(DriverObject,
                            RegistryPath,
                            &attributes,
                            &config,
                            &hDriver);
    if (!NT_SUCCESS(status)) 
	{
        DbgPrint("[GameArmor] WdfDriverCreate failed with status 0x%x\n", status);

        return status;
    }

	pInit = WdfControlDeviceInitAllocate(
                        hDriver,
                        &SDDL_DEVOBJ_SYS_ALL_ADM_RWX_WORLD_RW_RES_R
                        );

	if (pInit == NULL) 
	{
        status = STATUS_INSUFFICIENT_RESOURCES;
        return status;
    }

	status = GameArmorDeviceAdd(hDriver, pInit);

	return STATUS_SUCCESS;
}

NTSTATUS GameArmorDeviceAdd(IN WDFDRIVER Driver, IN PWDFDEVICE_INIT DeviceInit)
{
	NTSTATUS status;
    WDF_OBJECT_ATTRIBUTES attributes;
    WDF_IO_QUEUE_CONFIG ioQueueConfig;
    WDF_FILEOBJECT_CONFIG fileConfig;
    WDFQUEUE queue;
    WDFDEVICE controlDevice;
    DECLARE_CONST_UNICODE_STRING(ntDeviceName, GAMEARMOR_DEVICE_NAME_STRING);
    DECLARE_CONST_UNICODE_STRING(symbolicLinkName, GAMEARMOR_LINK_NAME_STRING);

    UNREFERENCED_PARAMETER(Driver);

    PAGED_CODE();

    WdfDeviceInitSetExclusive(DeviceInit, TRUE);
	WdfDeviceInitSetIoType(DeviceInit, WdfDeviceIoDirect);

    status = WdfDeviceInitAssignName(DeviceInit, &ntDeviceName);

    if (!NT_SUCCESS(status)) 
	{
        DbgPrint("[GameArmor] WdfDeviceInitAssignName failed %!STATUS!", status);

        goto End;
    }

    WdfControlDeviceInitSetShutdownNotification(DeviceInit,
                                                GameArmorShutdown,
                                                WdfDeviceShutdown);

    WDF_FILEOBJECT_CONFIG_INIT(
                        &fileConfig,
                        GameArmorEvtDeviceFileCreate,
                        GameArmorEvtFileClose,
                        WDF_NO_EVENT_CALLBACK
                        );

    WdfDeviceInitSetFileObjectConfig(DeviceInit,
								&fileConfig,
                                WDF_NO_OBJECT_ATTRIBUTES);

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&attributes,
                                    GAMEARMOR_EXTENSION);

    status = WdfDeviceCreate(&DeviceInit,
                             &attributes,
                             &controlDevice);
    if (!NT_SUCCESS(status)) 
	{
        DbgPrint("WdfDeviceCreate failed %!STATUS!", status);

        goto End;
    }

    status = WdfDeviceCreateSymbolicLink(controlDevice,
                                &symbolicLinkName);

    if (!NT_SUCCESS(status)) 
	{
        DbgPrint("WdfDeviceCreateSymbolicLink failed %!STATUS!", status);

        goto End;
    }

    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&ioQueueConfig,
                                    WdfIoQueueDispatchSequential);

    ioQueueConfig.EvtIoRead = FileEvtIoRead;
    ioQueueConfig.EvtIoWrite = FileEvtIoWrite;
    ioQueueConfig.EvtIoDeviceControl = FileEvtIoDeviceControl;

    WDF_OBJECT_ATTRIBUTES_INIT(&attributes);

    status = WdfIoQueueCreate(controlDevice,
                              &ioQueueConfig,
                              &attributes,
                              &queue
                              );
    if (!NT_SUCCESS(status)) 
	{
        DbgPrint("WdfIoQueueCreate failed %!STATUS!", status);

        goto End;
    }

    WdfControlFinishInitializing(controlDevice);

End:
    if (DeviceInit != NULL) 
	{
        WdfDeviceInitFree(DeviceInit);
    }

    return status;
}

VOID GameArmorEvtDeviceFileCreate(IN WDFDEVICE Device, IN WDFREQUEST Request, IN WDFFILEOBJECT FileObject)
{
	NTSTATUS status;

	PAGED_CODE();

	status = STATUS_SUCCESS;

	WdfRequestComplete(Request, status);
}

VOID GameArmorEvtFileClose(IN WDFFILEOBJECT FileObject)
{
	PGAMEARMOR_EXTENSION devExt;

	PAGED_CODE();

	devExt = ControlGetData((WDFOBJECT)WdfFileObjectGetDevice(FileObject));

	if (devExt->ProcessHandle)
	{
		ZwTerminateProcess(devExt->ProcessHandle, STATUS_SUCCESS);

		ZwClose(devExt->ProcessHandle);
	}
}

VOID FileEvtIoRead(IN WDFQUEUE Queue, IN WDFREQUEST Request, IN size_t Length)
{
	PAGED_CODE();

	WdfRequestComplete(Request, STATUS_SUCCESS);
}

VOID FileEvtIoWrite(IN WDFQUEUE Queue, IN WDFREQUEST Request, IN size_t Length)
{
	PAGED_CODE();

	WdfRequestComplete(Request, STATUS_SUCCESS);
}

VOID FileEvtIoDeviceControl(IN WDFQUEUE Queue, IN WDFREQUEST Request, IN size_t OutputBufferLength, IN size_t InputBufferLength, IN ULONG IoControlCode)
{
	NTSTATUS status = STATUS_SUCCESS;

	UNREFERENCED_PARAMETER(Queue);
	
	PAGED_CODE();

	switch (IoControlCode)
	{
	case IOCTL_GAMEARMOR_BOOT:
		DbgPrint("IOCTL_GAMEARMOR_BOOT received.\n");
		break;
	default:
		status = STATUS_INVALID_DEVICE_REQUEST;
		break;
	}

	WdfRequestComplete(Request, status);
}

VOID GameArmorEvtDeviceIoInCallerContext(IN WDFDEVICE Device, IN WDFREQUEST Request)
{
	PAGED_CODE();
}

VOID GameArmorShutdown(WDFDEVICE Device)
{
	UNREFERENCED_PARAMETER(Device);

	PAGED_CODE();
}

VOID GameArmorEvtDriverUnload(IN WDFDRIVER Driver)
{
	UNREFERENCED_PARAMETER(Driver);

	PAGED_CODE();
}
