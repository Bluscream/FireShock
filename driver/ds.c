/*
MIT License

Copyright (c) 2016 Benjamin "Nefarius" H�glinger

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


#include "fireshock.h"
#include <usb.h>
#include <wdfusb.h>

NTSTATUS DsInit(WDFDEVICE hDevice)
{
    NTSTATUS                        status;
    WDFMEMORY                       transferBuffer;
    UCHAR                           hidCommandEnable[DS_HID_COMMAND_ENABLE_SIZE] = { 0x42, 0x0C, 0x00, 0x00 };
    PDEVICE_CONTEXT                 pDeviceContext;
    WDFREQUEST                      controlRequest;
    WDF_USB_CONTROL_SETUP_PACKET    packet;
    WDF_OBJECT_ATTRIBUTES           transferAttribs;

    pDeviceContext = WdfObjectGet_DEVICE_CONTEXT(hDevice);

    status = WdfRequestCreate(
        WDF_NO_OBJECT_ATTRIBUTES,
        WdfDeviceGetIoTarget(hDevice),
        &controlRequest);

    if (!NT_SUCCESS(status))
    {
        KdPrint(("WdfRequestCreate failed with status 0x%X\n", status));
        return status;
    }

    WDF_OBJECT_ATTRIBUTES_INIT(&transferAttribs);

    transferAttribs.ParentObject = controlRequest;

    status = WdfMemoryCreate(
        &transferAttribs,
        NonPagedPool,
        0,
        DS_HID_COMMAND_ENABLE_SIZE,
        &transferBuffer,
        NULL);

    if (!NT_SUCCESS(status))
    {
        KdPrint(("WdfMemoryCreate failed with status 0x%X\n", status));
        return status;
    }

    status = WdfMemoryCopyFromBuffer(
        transferBuffer,
        0,
        hidCommandEnable,
        DS_HID_COMMAND_ENABLE_SIZE);

    if (!NT_SUCCESS(status))
    {
        KdPrint(("WdfMemoryCopyFromBuffer failed with status 0x%X\n", status));
        return status;
    }

    WDF_USB_CONTROL_SETUP_PACKET_INIT_CLASS(
        &packet,
        BmRequestHostToDevice,
        BMREQUEST_TO_INTERFACE,
        DS_HID_REQUEST_SET_REPORT,
        DS_HID_SET_REPORT_INIT,
        0);

    status = WdfUsbTargetDeviceFormatRequestForControlTransfer(
        pDeviceContext->UsbDevice,
        controlRequest,
        &packet,
        transferBuffer,
        NULL);

    if (!NT_SUCCESS(status))
    {
        KdPrint(("WdfUsbTargetDeviceFormatRequestForControlTransfer failed with status 0x%X\n", status));
        return status;
    }

    WdfRequestSetCompletionRoutine(
        controlRequest,
        ControlRequestCompletionRoutine,
        NULL);

    if (!WdfRequestSend(
        controlRequest,
        WdfUsbTargetDeviceGetIoTarget(pDeviceContext->UsbDevice),
        NULL))
    {
        KdPrint(("WdfRequestSend failed\n"));
    }

    return status;
}

VOID Ds3OutputEvtTimerFunc(
    _In_ WDFTIMER Timer
)
{
    UNREFERENCED_PARAMETER(Timer);
}

