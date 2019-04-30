/**
  ******************************************************************************
  * @file    bsp_usb.c
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2018-12-16
  * @brief   IPoverUSB BSP for USB communication
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
#include <bsp_io.h>
#include <bsp_usb.h>
#include <xpd_nvic.h>
#include <xpd_pwr.h>

void OTG_FS_IRQHandler(void);

/* USB dependencies initialization */
static void BSP_USB_Init(void * handle)
{
    /* GPIO settings */
    GPIO_vInitPin(USB_DM_PIN, USB_DM_CFG);
    GPIO_vInitPin(USB_DP_PIN, USB_DP_CFG);

    NVIC_SetPriorityConfig(OTG_FS_IRQn, 0x6, 0);
    NVIC_EnableIRQ(OTG_FS_IRQn);

    USB_vClockConfig(USB_CLOCKSOURCE_MSI);

    PWR_vVddUSB(ENABLE);
}

/* USB dependencies deinitialization */
static void BSP_USB_Deinit(void * handle)
{
    GPIO_vDeinitPin(USB_DM_PIN);
    GPIO_vDeinitPin(USB_DP_PIN);
    NVIC_DisableIRQ(OTG_FS_IRQn);
    PWR_vVddUSB(DISABLE);
}

extern USB_HandleType *const UsbDevice;

void BSP_USB_Bind(void)
{
    USB_INST2HANDLE(UsbDevice, USB_OTG_FS);
    UsbDevice->Callbacks.DepInit = BSP_USB_Init;
    UsbDevice->Callbacks.DepDeinit = BSP_USB_Deinit;
}

/* Common interrupt handler for USB core and WKUP line */
void OTG_FS_IRQHandler(void)
{
    EXTI_vClearFlag(USB_OTG_FS_WAKEUP_EXTI_LINE);

    /* Handle USB interrupts */
    USB_vIRQHandler(UsbDevice);
}
