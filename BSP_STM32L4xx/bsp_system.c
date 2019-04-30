/**
  ******************************************************************************
  * @file    bsp_system.c
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2018-12-16
  * @brief   IPoverUSB BSP for system clocking
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
#include <bsp_system.h>
#include <xpd_rcc.h>

static const RCC_MSI_InitType msiconf = {
    .ClockFreq = MSI_48MHz,
    .State = ENABLE
};

static const RCC_PLL_InitType pllconf = {
    .State = ENABLE,
    .Source = MSI,
    .M = 6,
    .N = 20,
    .R = 2, /* MSI / 6 * 20 / 2 = PLLR = 80000000 -> SYSCLK */

    .Q = 2,
    .P = 7
};

/* System clocks configuration */
void SystemClock_Config(void)
{
    RCC_eMSI_Config(&msiconf);

#ifdef LSE_VALUE_Hz
    /* Use LSE to synchronize MSI */
    RCC_eLSE_Config(OSC_ON);
#endif

    RCC_ePLL_Config(&pllconf);

    /* System clocks configuration */
    RCC_eHCLK_Config(PLL, CLK_DIV1, 4);

    RCC_vPCLK1_Config(CLK_DIV1);
    RCC_vPCLK2_Config(CLK_DIV1);
}
