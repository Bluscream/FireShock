/*++

Module Name:

    device.c - Device handling events for example driver.

Abstract:

   This file contains the device entry points and callbacks.

Environment:

    User-mode Driver Framework 2

--*/

#include "driver.h"
#include "device.tmh"



NTSTATUS
FireShockCreateDevice(
    _Inout_ PWDFDEVICE_INIT DeviceInit
)
/*++

Routine Description:

    Worker routine called to create a device and its software resources.

Arguments:

    DeviceInit - Pointer to an opaque init structure. Memory for this
                    structure will be freed by the framework when the WdfDeviceCreate
                    succeeds. So don't access the structure after that point.

Return Value:

    NTSTATUS

--*/
{
    WDF_PNPPOWER_EVENT_CALLBACKS    pnpPowerCallbacks;
    WDF_OBJECT_ATTRIBUTES           deviceAttributes;
    WDFDEVICE                       device;
    NTSTATUS                        status;

    WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&pnpPowerCallbacks);
    pnpPowerCallbacks.EvtDevicePrepareHardware = FireShockEvtDevicePrepareHardware;
    pnpPowerCallbacks.EvtDeviceD0Entry = FireShockEvtDeviceD0Entry;
    pnpPowerCallbacks.EvtDeviceD0Exit = FireShockEvtDeviceD0Exit;
    WdfDeviceInitSetPnpPowerEventCallbacks(DeviceInit, &pnpPowerCallbacks);

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&deviceAttributes, DEVICE_CONTEXT);

    status = WdfDeviceCreate(&DeviceInit, &deviceAttributes, &device);

    if (NT_SUCCESS(status)) 
    {
        //
        // Create a device interface so that applications can find and talk
        // to us.
        //
        status = WdfDeviceCreateDeviceInterface(
            device,
            &GUID_DEVINTERFACE_FIRESHOCK,
            NULL // ReferenceString
        );

        if (NT_SUCCESS(status)) {
            //
            // Initialize the I/O Package and any Queues
            //
            status = FireShockQueueInitialize(device);

            if (NT_SUCCESS(status))
            {
                status = FireShockIoReadQueueInitialize(device);
            }
        }
    }

    return status;
}

