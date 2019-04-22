/**
  ******************************************************************************
  * @file    stm32_rom_dfu.c
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2018-12-16
  * @brief   STM32 ROM DFU interface
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
#include <stm32_rom_dfu.h>
#include <xpd_utils.h>

static const char* stm32_dfu_if_name = "STM32 BOOTLOADER";
static volatile bool dfuRequested = false;

static USBD_DFU_IfHandleType _dfu_if;
USBD_DFU_IfHandleType *const stm32_rom_dfu_if = &_dfu_if;

static void bootto_dfu_isr(void)
{
    dfuRequested = true;
}

/**
 * @brief Initializes the STM32 ROM DFU interface.
 */
void STM32_ROM_DFU_Init(void)
{
    /* Instead of the App struct, only the string pointer is passed
     * the rest of the App fields aren't used by Reboot-only DFU */
    stm32_rom_dfu_if->App = (void*)&stm32_dfu_if_name;

    stm32_rom_dfu_if->Config.Reboot = bootto_dfu_isr;
    stm32_rom_dfu_if->Config.DetachTimeout_ms = 100;
}

/**
 * @brief When a DFU detach is requested, the device must start running
 *        the STM32 embedded ROM bootloader. The entry must be done in thread mode.
 */
void STM32_ROM_DFU_Main(void)
{
    if (dfuRequested)
    {
        XPD_vBootTo(SYSTEM_MEMORY_ADDR);
    }
}
