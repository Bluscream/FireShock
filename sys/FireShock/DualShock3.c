﻿/*
MIT License

Copyright (c) 2017 Benjamin "Nefarius" Höglinger

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


#include "Driver.h"
#include "DualShock3.tmh"

//
// Sends output report updates to the DS3 via control endpoint.
// 
VOID Ds3OutputEvtTimerFunc(
    _In_ WDFTIMER Timer
)
{
    WDFDEVICE   hDevice;
    NTSTATUS    status;

    hDevice = WdfTimerGetParentObject(Timer);

    status = SendControlRequest(
        DeviceGetContext(hDevice),
        BmRequestClass,
        SetReport,
        USB_SETUP_VALUE(Output, One),
        0,
        Ds3GetContext(hDevice)->OutputReportBuffer,
        DS3_HID_OUTPUT_REPORT_SIZE
    );

    if (!NT_SUCCESS(status))
    {
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_DUALSHOCK3,
            "SendControlRequest failed with status %!STATUS!",
            status);
    }
}

//
// Sends the "magic packet" to the DS3 so it starts its interrupt endpoint.
// 
NTSTATUS Ds3Init(PDEVICE_CONTEXT Context)
{
    // "Magic packet"
    UCHAR hidCommandEnable[DS3_HID_COMMAND_ENABLE_SIZE] =
    {
        0x42, 0x0C, 0x00, 0x00
    };

    return SendControlRequest(
        Context,
        BmRequestClass,
        SetReport,
        Ds3StartDevice,
        0,
        hidCommandEnable,
        DS3_HID_COMMAND_ENABLE_SIZE
    );
}
