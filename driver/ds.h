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


#pragma once

#define DS3_CONFIGURATION_DESCRIPTOR_SIZE       0x29
#define DS3_OUTPUT_REPORT_SEND_DELAY            0x0A
#define DS3_HID_COMMAND_ENABLE_SIZE             0x04
#define DS3_HID_OUTPUT_REPORT_SIZE              0x30
#define DS3_HID_REPORT_DESCRIPTOR_SIZE          0xB0
#define DS3_INTERRUPT_IN_BUFFER_SIZE            0x40
#define DS3_ORIGINAL_HID_REPORT_SIZE            0x31
#define DS3_INPUT_ENABLE_SEND_DELAY             0x012C

#define DS3_OFFSET_LED_0                        0x02
#define DS3_OFFSET_LED_1                        0x04
#define DS3_OFFSET_LED_2                        0x08
#define DS3_OFFSET_LED_3                        0x10

#define DS3_OFFSET_LED_INDEX                    0x09


typedef enum _USB_HID_REQUEST
{
    // Class-Specific Requests
    GetReport = 0x01,
    GetIdle = 0x02,
    GetProtocol = 0x03,
    SetReport = 0x09,
    SetIdle = 0x0A,
    SetProtocol = 0x0B,
    // Standard Requests
    GetDescriptor = 0x06,
    SetDescriptor = 0x07
} USB_HID_REQUEST;

typedef enum _USB_HID_REPORT_REQUEST_TYPE
{
    Input = 0x01,
    Output = 0x02,
    Feature = 0x03
} USB_HID_REPORT_REQUEST_TYPE;

typedef enum _USB_HID_REPORT_REQUEST_ID
{
    One = 0x01
} USB_HID_REPORT_REQUEST_ID;

typedef enum _USB_HID_CLASS_DESCRIPTOR_TYPE
{
    Hid = 0x21,
    Report = 0x22,
    PhysicalDescriptor = 0x23
} USB_HID_CLASS_DESCRIPTOR_TYPE;

typedef enum _DS3_FEATURE_VALUE
{
    Ds3GetDeviceAddress = 0x03F2,
    Ds3StartDevice = 0x03F4,
    Ds3SetHostAddress = 0x03F5,
} DS3_FEATURE_VALUE;

#define USB_SETUP_VALUE(_type_, _id_) (USHORT)((_type_ << 8) | _id_)

typedef enum _DS_DEVICE_TYPE
{
	DualShock3,
	DualShock4
} DS_DEVICE_TYPE, *PDS_DEVICE_TYPE;

EVT_WDF_TIMER Ds3OutputEvtTimerFunc;

EVT_WDF_TIMER Ds3EnableEvtTimerFunc;

NTSTATUS Ds3Init(WDFDEVICE hDevice);

VOID Ds3GetConfigurationDescriptorType(PUCHAR Buffer, ULONG Length);

VOID Ds3GetDescriptorFromInterface(PUCHAR Buffer);

SHORT ScaleAxis(SHORT value, BOOLEAN flip);
