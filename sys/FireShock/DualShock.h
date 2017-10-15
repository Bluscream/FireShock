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

#define DS3_VENDOR_ID                           0x054C
#define DS3_PRODUCT_ID                          0x0268

#define DS4_HID_OUTPUT_REPORT_SIZE              0x20
#define DS4_VENDOR_ID                           0x054C
#define DS4_PRODUCT_ID                          0x05C4
#define DS4_2_PRODUCT_ID                        0x09CC
#define DS4_WIRELESS_ADAPTER_PRODUCT_ID         0x0BA0

#define PS_MOVE_NAVI_PRODUCT_ID                 0x042F

#define VIGEM_SERIAL_BEGIN                      0x01
#define VIGEM_SERIAL_END                        0x04

#define IS_DS3(_ctx_)             ((_ctx_->DeviceType == DualShock3))
#define IS_DS4(_ctx_)             ((_ctx_->DeviceType == DualShock4))
#define IS_INTERRUPT_IN(_urb_)    ((_urb_->UrbBulkOrInterruptTransfer.TransferFlags & USBD_TRANSFER_DIRECTION_IN))


/**
* \typedef struct _BD_ADDR
*
* \brief   Defines a Bluetooth client MAC address.
*/
typedef struct _BD_ADDR
{
    BYTE Address[6];

} BD_ADDR, *PBD_ADDR;

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
    HidReportRequestTypeInput = 0x01,
    HidReportRequestTypeOutput = 0x02,
    HidReportRequestTypeFeature = 0x03

} USB_HID_REPORT_REQUEST_TYPE;

typedef enum _USB_HID_REPORT_REQUEST_ID
{
    HidReportRequestIdOne = 0x01

} USB_HID_REPORT_REQUEST_ID;

typedef enum _USB_HID_CLASS_DESCRIPTOR_TYPE
{
    Hid = 0x21,
    Report = 0x22,
    PhysicalDescriptor = 0x23

} USB_HID_CLASS_DESCRIPTOR_TYPE;

typedef enum _DS3_FEATURE_VALUE
{
    Ds3FeatureDeviceAddress = 0x03F2,
    Ds3FeatureStartDevice = 0x03F4,
    Ds3FeatureHostAddress = 0x03F5

} DS3_FEATURE_VALUE;

#define USB_SETUP_VALUE(_type_, _id_) (USHORT)((_type_ << 8) | _id_)

typedef enum _DS_DEVICE_TYPE
{
    DsTypeUnknown,
    DualShock3,
    DualShock4

} DS_DEVICE_TYPE, *PDS_DEVICE_TYPE;

EVT_WDF_TIMER Ds3OutputEvtTimerFunc;

