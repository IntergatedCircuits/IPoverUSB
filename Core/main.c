/**
  ******************************************************************************
  * @file    main.c
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2018-12-16
  * @brief   IPoverUSB application main entry point
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
#include <bsp_system.h>
#include <xpd_systick.h>

#include <usbd.h>
#include <stm32_rom_dfu.h>
#include <ncm_netif.h>

#include <lwip/apps/httpd.h>
#include <lwip/init.h>
#include <lwip/timeouts.h>

#if (NO_SYS == 0)
#include <lwip/tcpip.h>
#include <xpd_utils.h>
#include <FreeRTOS.h>
#include <task.h>

extern const XPD_TimeServiceType xRtosTimeService;
#endif

USB_HandleType _UsbDevice, *const UsbDevice = &_UsbDevice;

extern void usb_device_init(USBD_HandleType *usbd);

#if (NO_SYS == 1)

#if (LWIP_TIMERS == 1)
/* Minimal lwIP global time service */
static volatile uint32_t globalTime_ms = 0;

void SysTick_Handler(void)
{
    globalTime_ms++;
}
uint32_t sys_jiffies(void)
{
    return globalTime_ms;
}
uint32_t sys_now(void)
{
    return globalTime_ms;
}
#endif

int main(void)
{
    /* Prepare hardware for operation */
    BSP_USB_Bind();
    SystemClock_Config();
#if (LWIP_TIMERS == 1)
    SysTick_IT_Enable();
#endif

    /* Initialize applications */
    /* DFU interface can be issued a Detach request,
     * which puts the STM32 to ROM bootloader mode */
    STM32_ROM_DFU_Init();

    /* init lwIP stack and network interface */
    lwip_init();
    ncm_netif_init();
    usb_device_init(UsbDevice);

    /* use default HTTP server for demonstration */
    httpd_init();

    /* Attach the USB device to the host (soft-connect) */
    USBD_Connect(UsbDevice);

    while (1)
    {
        /* perform networking */
        ncm_netif_process();

        sys_check_timeouts();

        /* switch to bootloader when Detached */
        STM32_ROM_DFU_Main();
    }
}
#else

static void init_from_thread(void *arg)
{
    USB_HandleType *usbd = arg;

    /* DFU interface can be issued a Detach request,
     * which puts the STM32 to ROM bootloader mode */
    STM32_ROM_DFU_Init();

    ncm_netif_init();
    usb_device_init(usbd);

    /* use default HTTP server for demonstration */
    httpd_init();

    /* Attach the USB device to the host (soft-connect) */
    USBD_Connect(usbd);
}

void vApplicationIdleHook(void)
{
    /* switch to bootloader when Detached */
    STM32_ROM_DFU_Main();
}

int main(void)
{
    /* Prepare hardware for operation */
    BSP_USB_Bind();
    SystemClock_Config();

    /* init lwIP stack and continue initialization from thread context */
    tcpip_init(init_from_thread, UsbDevice);

    /* change XPD time service for RTOS operation */
    XPD_vSetTimeService(&xRtosTimeService);

    vTaskStartScheduler();

    /* unreachable */
    while (1);
}
#endif
