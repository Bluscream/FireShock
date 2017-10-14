﻿#include "Driver.h"
#include "DsUsb.tmh"

//
// Sends a custom buffer to the device's control endpoint.
// 
NTSTATUS
SendControlRequest(
    _In_ PDEVICE_CONTEXT Context,
    _In_ BYTE Type,
    _In_ BYTE Request,
    _In_ USHORT Value,
    _In_ USHORT Index,
    _In_ PVOID Buffer,
    _In_ ULONG BufferLength)
{
    NTSTATUS                        status;
    WDF_USB_CONTROL_SETUP_PACKET    controlSetupPacket;
    WDF_REQUEST_SEND_OPTIONS        sendOptions;
    WDF_MEMORY_DESCRIPTOR           memDesc;
    ULONG                           bytesTransferred;

    WDF_REQUEST_SEND_OPTIONS_INIT(
        &sendOptions,
        WDF_REQUEST_SEND_OPTION_TIMEOUT
    );

    WDF_REQUEST_SEND_OPTIONS_SET_TIMEOUT(
        &sendOptions,
        DEFAULT_CONTROL_TRANSFER_TIMEOUT
    );

    switch (Type)
    {
    case BmRequestClass:
        WDF_USB_CONTROL_SETUP_PACKET_INIT_CLASS(&controlSetupPacket,
            BmRequestHostToDevice,
            BmRequestToDevice,
            Request,
            Value,
            Index);
        break;

    default:
        return STATUS_INVALID_PARAMETER;
    }

    WDF_MEMORY_DESCRIPTOR_INIT_BUFFER(&memDesc,
        Buffer,
        BufferLength);

    status = WdfUsbTargetDeviceSendControlTransferSynchronously(
        Context->UsbDevice,
        WDF_NO_HANDLE,
        &sendOptions,
        &controlSetupPacket,
        &memDesc,
        &bytesTransferred);

    if (!NT_SUCCESS(status)) {

        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DSUSB,
            "WdfUsbTargetDeviceSendControlTransferSynchronously: Failed - 0x%x (%d)\n", status, bytesTransferred);
    }

    return status;
}
