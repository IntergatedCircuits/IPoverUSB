/**
  ******************************************************************************
  * @file    usb_device.c
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2018-12-16
  * @brief   IPoverUSB USB device definition
  *
  * Copyright (c) 2018 Benedek Kupper
  *
  * Licensed under the Apache License, Version 2.0 (the "License");
  * you may not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  *
  *     http://www.apache.org/licenses/LICENSE-2.0
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  */
#include <usbd.h>
#include <stm32_rom_dfu.h>
#include <ncm_netif.h>

#include <usbd_cdc.h>
extern USBD_CDC_IfHandleType * const console_if;

USBD_DescriptionType _dev_cfg = {
    .Vendor = {
            .Name           = "STMicroelectronics",
            .ID             = 0x0483,
    },
    .Product = {
            .Name           = "IP over USB Demonstrator",
            .ID             = 0xFFFF,
            .Version.bcd    = 0x0100,
    },
    .SerialNumber = (USBD_SerialNumberType*)DEVICE_ID_REG,
    .Config = {
            .Name           = "NCM Device Configuration",
            .MaxCurrent_mA  = 100,
            .RemoteWakeup   = 0,
            .SelfPowered    = 1,
    },
}, *const dev_cfg = &_dev_cfg;

void usb_device_init(USBD_HandleType *usbd)
{
    /* Mount the interfaces to the device */
    USBD_DFU_MountRebootOnly(stm32_rom_dfu_if, usbd);

    /* All fields of Config has to be properly set up */
    ncm_usb_if->Config.InEpNum = 0x81;
    ncm_usb_if->Config.OutEpNum = 0x01;
    ncm_usb_if->Config.NotEpNum = 0x82;

    USBD_NCM_MountInterface(ncm_usb_if, usbd);

#if (STDOUT_BUFFER_SIZE > 0)
    console_if->Config.InEpNum = 0x83;
    console_if->Config.OutEpNum = 0x03;
    console_if->Config.NotEpNum = 0x84;

    USBD_CDC_MountInterface(console_if, usbd);
#endif

    /* Initialize the device */
    USBD_Init(usbd, dev_cfg);
}
