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
#include <wdf.h>


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FireShockEvtDevicePrepareHardware)
#pragma alloc_text(PAGE, FireShockEvtDeviceD0Entry)
#pragma alloc_text(PAGE, FireShockEvtDeviceD0Exit)
#endif


_Use_decl_annotations_
NTSTATUS
FireShockEvtDevicePrepareHardware(
    WDFDEVICE  Device,
    WDFCMRESLIST  ResourcesRaw,
    WDFCMRESLIST  ResourcesTranslated
)
{
    NTSTATUS                        status = STATUS_SUCCESS;
    PDEVICE_CONTEXT                 pDeviceContext;
    USB_DEVICE_DESCRIPTOR           deviceDescriptor;
    PDS3_DEVICE_CONTEXT             pDs3Context;
    PDS4_DEVICE_CONTEXT             pDs4Context;
    WDF_OBJECT_ATTRIBUTES           attributes;
    WDF_TIMER_CONFIG                timerCfg;
    USHORT                          xusbVid = 0;
    USHORT                          xusbPid = 0;
    BOOLEAN                         supported = FALSE;

    UNREFERENCED_PARAMETER(ResourcesRaw);
    UNREFERENCED_PARAMETER(ResourcesTranslated);

    PAGED_CODE();

    KdPrint((DRIVERNAME "FireShockEvtDevicePrepareHardware called\n"));

    pDeviceContext = GetCommonContext(Device);

    // Create USB framework object
    if (pDeviceContext->UsbDevice == NULL)
    {
        status = WdfUsbTargetDeviceCreate(Device, WDF_NO_OBJECT_ATTRIBUTES, &pDeviceContext->UsbDevice);
        if (!NT_SUCCESS(status))
        {
            KdPrint((DRIVERNAME "WdfUsbTargetDeviceCreate failed with status 0x%X\n", status));
            return status;
        }
    }

    // Use device descriptor to identify the device
    WdfUsbTargetDeviceGetDeviceDescriptor(pDeviceContext->UsbDevice, &deviceDescriptor);

    // Device is a DualShock 3 or Move Navigation Controller
    if (deviceDescriptor.idVendor == DS3_VENDOR_ID
        && (deviceDescriptor.idProduct == DS3_PRODUCT_ID || deviceDescriptor.idProduct == PS_MOVE_NAVI_PRODUCT_ID))
    {
        pDeviceContext->DeviceType = DualShock3;
        xusbVid = 0x1337;
        xusbPid = 0x0001;
        supported = TRUE;

        // Add DS3-specific context to device object
        WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&attributes, DS3_DEVICE_CONTEXT);

        status = WdfObjectAllocateContext(Device, &attributes, (PVOID)&pDs3Context);
        if (!NT_SUCCESS(status))
        {
            KdPrint((DRIVERNAME "WdfObjectAllocateContext failed status 0x%x\n", status));
            return status;
        }

        // 
        // Initial output state (rumble & LEDs off)
        // 
        // Note: no report ID because sent over control endpoint
        // 
        UCHAR DefaultOutputReport[DS3_HID_OUTPUT_REPORT_SIZE] =
        {
            0x00, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0xFF, 0x27, 0x10, 0x00, 0x32, 0xFF,
            0x27, 0x10, 0x00, 0x32, 0xFF, 0x27, 0x10, 0x00,
            0x32, 0xFF, 0x27, 0x10, 0x00, 0x32, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

        RtlCopyBytes(pDs3Context->OutputReportBuffer, DefaultOutputReport, DS3_HID_OUTPUT_REPORT_SIZE);

        // Set initial LED index to device arrival index (max. 4 possible for DS3)
        switch (pDeviceContext->DeviceIndex)
        {
        case 0:
            pDs3Context->OutputReportBuffer[DS3_OFFSET_LED_INDEX] |= DS3_OFFSET_LED_0;
            break;
        case 1:
            pDs3Context->OutputReportBuffer[DS3_OFFSET_LED_INDEX] |= DS3_OFFSET_LED_1;
            break;
        case 2:
            pDs3Context->OutputReportBuffer[DS3_OFFSET_LED_INDEX] |= DS3_OFFSET_LED_2;
            break;
        case 3:
            pDs3Context->OutputReportBuffer[DS3_OFFSET_LED_INDEX] |= DS3_OFFSET_LED_3;
            break;
        default:
            // TODO: what do we do in this case? Light animation?
            break;
        }

        // Initialize output report timer
        WDF_TIMER_CONFIG_INIT_PERIODIC(&timerCfg, Ds3OutputEvtTimerFunc, DS3_OUTPUT_REPORT_SEND_DELAY);
        WDF_OBJECT_ATTRIBUTES_INIT(&attributes);

        attributes.ParentObject = Device;

        status = WdfTimerCreate(&timerCfg, &attributes, &pDs3Context->OutputReportTimer);
        if (!NT_SUCCESS(status)) {
            KdPrint((DRIVERNAME "Error creating output report timer 0x%x\n", status));
            return status;
        }

        // Initialize input enable timer
        WDF_TIMER_CONFIG_INIT_PERIODIC(&timerCfg, Ds3EnableEvtTimerFunc, DS3_INPUT_ENABLE_SEND_DELAY);
        WDF_OBJECT_ATTRIBUTES_INIT(&attributes);

        attributes.ParentObject = Device;

        status = WdfTimerCreate(&timerCfg, &attributes, &pDs3Context->InputEnableTimer);
        if (!NT_SUCCESS(status)) {
            KdPrint((DRIVERNAME "Error creating input enable timer 0x%x\n", status));
            return status;
        }

        // We can start the timer here since the callback is called again on failure
        WdfTimerStart(pDs3Context->InputEnableTimer, WDF_REL_TIMEOUT_IN_MS(DS3_INPUT_ENABLE_SEND_DELAY));
    }

    // Device is a DualShock 4 or Wireless USB Adapter
    if (deviceDescriptor.idVendor == DS4_VENDOR_ID
        && (deviceDescriptor.idProduct == DS4_PRODUCT_ID 
            || deviceDescriptor.idProduct == DS4_WIRELESS_ADAPTER_PRODUCT_ID
            || deviceDescriptor.idProduct == DS4_2_PRODUCT_ID))
    {
        pDeviceContext->DeviceType = DualShock4;
        xusbVid = 0x1337;
        xusbPid = 0x0002;
        supported = TRUE;

        // Add DS4-specific context to device object
        WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&attributes, DS4_DEVICE_CONTEXT);

        status = WdfObjectAllocateContext(Device, &attributes, (PVOID)&pDs4Context);
        if (!NT_SUCCESS(status))
        {
            KdPrint((DRIVERNAME "WdfObjectAllocateContext failed status 0x%x\n", status));
            return status;
        }

        // 
        // Initial output state (rumble & LEDs off)
        // 
        UCHAR DefaultOutputReport[DS4_HID_OUTPUT_REPORT_SIZE] =
        {
            0x05, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF,
            0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

        RtlCopyBytes(pDs4Context->OutputReportBuffer, DefaultOutputReport, DS4_HID_OUTPUT_REPORT_SIZE);
    }

    if (deviceDescriptor.idVendor == 0x1337)
    {
        KdPrint((DRIVERNAME "Device is XUSB HID device\n"));

        return status;
    }

    if (!supported)
    {
        return STATUS_NOT_SUPPORTED;
    }

    VIGEM_PLUGIN_TARGET plugIn;
    WDF_MEMORY_DESCRIPTOR inBuffer;

    if (pDeviceContext->ViGEm.IoTarget)
    {
        for (
            pDeviceContext->ViGEm.Serial = VIGEM_SERIAL_BEGIN;
            pDeviceContext->ViGEm.Serial <= VIGEM_SERIAL_END;
            pDeviceContext->ViGEm.Serial++)
        {
            VIGEM_PLUGIN_TARGET_INIT(&plugIn, pDeviceContext->ViGEm.Serial, Xbox360Wired);
            WDF_MEMORY_DESCRIPTOR_INIT_BUFFER(&inBuffer, &plugIn, plugIn.Size);

            KdPrint((DRIVERNAME "pDeviceContext->ViGEm.Serial = %d", pDeviceContext->ViGEm.Serial));

            // TODO: replace with async call!
            status = WdfIoTargetSendInternalIoctlSynchronously(
                pDeviceContext->ViGEm.IoTarget,
                NULL,
                IOCTL_VIGEM_PLUGIN_TARGET,
                &inBuffer,
                NULL,
                NULL,
                NULL);

            if (!NT_SUCCESS(status))
            {
                KdPrint((DRIVERNAME "Couldn't request XUSB device: 0x%X", status));
                continue;
            }

            break;
        }
    }

    return status;
}

//
// Gets called when the device reaches D0 state.
// 
NTSTATUS FireShockEvtDeviceD0Entry(
    _In_ WDFDEVICE              Device,
    _In_ WDF_POWER_DEVICE_STATE PreviousState
)
{
    UNREFERENCED_PARAMETER(Device);
    UNREFERENCED_PARAMETER(PreviousState);

    PAGED_CODE();

    KdPrint((DRIVERNAME "FireShockEvtDeviceD0Entry called\n"));

    return STATUS_SUCCESS;
}

//
// Gets called when the device leaves D0 state.
// 
NTSTATUS FireShockEvtDeviceD0Exit(
    _In_ WDFDEVICE              Device,
    _In_ WDF_POWER_DEVICE_STATE TargetState
)
{
    PDEVICE_CONTEXT         pDeviceContext;
    NTSTATUS                status;
    VIGEM_UNPLUG_TARGET     unPlug;
    WDF_MEMORY_DESCRIPTOR   inBuffer;

    UNREFERENCED_PARAMETER(TargetState);

    KdPrint((DRIVERNAME "FireShockEvtDeviceD0Exit called\n"));

    PAGED_CODE();

    pDeviceContext = GetCommonContext(Device);

    //
    // Unplug ViGEm device
    // 
    if (pDeviceContext->ViGEm.IoTarget)
    {
        VIGEM_UNPLUG_TARGET_INIT(&unPlug, pDeviceContext->ViGEm.Serial);
        WDF_MEMORY_DESCRIPTOR_INIT_BUFFER(&inBuffer, &unPlug, unPlug.Size);

        KdPrint((DRIVERNAME "pDeviceContext->ViGEm.Serial = %d", pDeviceContext->ViGEm.Serial));

        // TODO: replace with async call!
        status = WdfIoTargetSendInternalIoctlSynchronously(
            pDeviceContext->ViGEm.IoTarget,
            NULL,
            IOCTL_VIGEM_UNPLUG_TARGET,
            &inBuffer,
            NULL,
            NULL,
            NULL);

        if (!NT_SUCCESS(status))
        {
            KdPrint((DRIVERNAME "IOCTL_VIGEM_UNPLUG_TARGET failed with status: 0x%X", status));
        }
    }

    // Perform clean-up tasks
    FilterShutdown(Device);

    return STATUS_SUCCESS;
}

