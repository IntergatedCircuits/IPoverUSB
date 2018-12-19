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

static const RCC_PLL_InitType pllconf = {
    .State = ENABLE,
    .Source = HSE,  /* 8000000 */
    .M = 8,
    .N = 336,
    .P = 2,         /* HSE / 8 * 336 / 2 = PLLP = 168000000 -> SYSCLK */
    .Q = 7          /* HSE / 8 * 336 / 7 = PLLQ =  48000000 -> USB */
};

/* System clocks configuration */
void SystemClock_Config(void)
{
    RCC_eHSE_Config(OSC_ON);
    RCC_ePLL_Config(&pllconf);

    /* System clocks configuration */
    RCC_eHCLK_Config(PLL, CLK_DIV1, 5);

    RCC_vPCLK1_Config(CLK_DIV4);
    RCC_vPCLK2_Config(CLK_DIV2);
}
