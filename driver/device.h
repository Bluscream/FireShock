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


#include "ds.h"
#include "driver.h"

#define NTDEVICE_NAME_STRING      L"\\Device\\FireShockFilter"
#define SYMBOLIC_NAME_STRING      L"\\DosDevices\\FireShockFilter"

//
// Data used in ViGEm interaction
// 
typedef struct _VIGEM_META
{
    VIGEM_INTERFACE_STANDARD Interface;

    BOOLEAN Available;

    ULONG Serial;

    WDFWAITLOCK Lock;

} VIGEM_META, *PVIGEM_META;

//
// Common device context
// 
typedef struct _DEVICE_CONTEXT
{
    WDFUSBDEVICE UsbDevice;

    DS_DEVICE_TYPE DeviceType;

    ULONG DeviceIndex;

    VIGEM_META ViGEm;

} DEVICE_CONTEXT, *PDEVICE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_CONTEXT, GetCommonContext)

//
// DualShock 3-specific context
// 
typedef struct _DS3_DEVICE_CONTEXT
{
    WDFTIMER OutputReportTimer;

    WDFTIMER InputEnableTimer;

    UCHAR OutputReportBuffer[DS3_HID_OUTPUT_REPORT_SIZE];

    FS3_GAMEPAD_STATE InputState;

} DS3_DEVICE_CONTEXT, *PDS3_DEVICE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DS3_DEVICE_CONTEXT, Ds3GetContext)

//
// DualShock 4-specific context
// 
typedef struct _DS4_DEVICE_CONTEXT
{
    WDFTIMER OutputReportTimer;

    UCHAR OutputReportBuffer[DS4_HID_OUTPUT_REPORT_SIZE];

} DS4_DEVICE_CONTEXT, *PDS4_DEVICE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DS4_DEVICE_CONTEXT, Ds4GetContext)


EVT_WDF_DEVICE_CONTEXT_CLEANUP EvtDeviceContextCleanup;
EVT_WDF_IO_QUEUE_IO_INTERNAL_DEVICE_CONTROL EvtIoInternalDeviceControl;
EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL FilterEvtIoDeviceControl;
EVT_WDF_OBJECT_CONTEXT_CLEANUP EvtCleanupCallback;
EVT_WDF_IO_TARGET_QUERY_REMOVE EvtIoTargetQueryRemove;

_Must_inspect_result_
_Success_(return == STATUS_SUCCESS)
_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS
FilterCreateControlDevice(
    _In_ WDFDEVICE Device
);

_IRQL_requires_max_(PASSIVE_LEVEL)
VOID
FilterDeleteControlDevice(
    _In_ WDFDEVICE Device
);

VOID FilterShutdown(WDFDEVICE Device);

