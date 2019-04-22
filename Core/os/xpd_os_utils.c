/**
  ******************************************************************************
  * @file    xpd_os_utils.c
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2018-09-11
  * @brief   STM32 eXtensible Peripheral Drivers FreeRTOS-specific Utilities
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
#include <xpd_utils.h>

#include <FreeRTOS.h>
#include <task.h>

static const TickType_t xWaitIntervalTick = 1;

extern void vPortSetupTimerInterrupt(void);

static void prvSysTimerSetup(uint32_t ulUnused)
{
    portDISABLE_INTERRUPTS();

    vPortSetupTimerInterrupt();

    portENABLE_INTERRUPTS();
}

static void prvTaskDelay_ms(uint32_t ulMilliseconds)
{
    vTaskDelay(pdMS_TO_TICKS(ulMilliseconds));
}

static XPD_ReturnType prvTaskDelayToMatch(
        volatile uint32_t * pulVarAddress,
        uint32_t            ulBitSelector,
        uint32_t            ulMatch,
        uint32_t *          pulTimeout)
{
    XPD_ReturnType eResult = XPD_OK;
    uint32_t ulStartTick = xTaskGetTickCount();
    uint32_t ulSysTicks = pdMS_TO_TICKS(*pulTimeout);
    uint32_t ulTickTimeout = ulStartTick + ulSysTicks;

    do
    {
        if ((*pulVarAddress & ulBitSelector) == ulMatch)
        {
            break;
        }

        vTaskDelay(xWaitIntervalTick);

        if (ulTickTimeout < xTaskGetTickCount())
        {
            eResult = XPD_TIMEOUT;
            break;
        }
    }
    while (1);

    /* Subtract elapsed time from budget */
    ulSysTicks -= xTaskGetTickCount() - ulStartTick;

    /* Implicitly return the remaining time */
    *pulTimeout = (ulSysTicks * 1000) / configTICK_RATE_HZ;

    return eResult;
}

static XPD_ReturnType prvTaskDelayToDiff(
        volatile uint32_t * pulVarAddress,
        uint32_t ulBitSelector,
        uint32_t ulMatch,
        uint32_t * pulTimeout)
{
    XPD_ReturnType eResult = XPD_OK;
    uint32_t ulStartTick = xTaskGetTickCount();
    uint32_t ulSysTicks = pdMS_TO_TICKS(*pulTimeout);
    uint32_t ulTickTimeout = ulStartTick + ulSysTicks;

    do
    {
        if ((*pulVarAddress & ulBitSelector) != ulMatch)
        {
            break;
        }

        vTaskDelay(xWaitIntervalTick);

        if (ulTickTimeout < xTaskGetTickCount())
        {
            eResult = XPD_TIMEOUT;
            break;
        }
    }
    while (1);

    /* Subtract elapsed time from budget */
    ulSysTicks -= xTaskGetTickCount() - ulStartTick;

    /* Implicitly return the remaining time */
    *pulTimeout = (ulSysTicks * 1000) / configTICK_RATE_HZ;

    return eResult;
}

/**
 * @brief This API uses FreeRTOS task delay to implement the XPD time services.
 *        Call @ref XPD_vSetTimeService with its reference right before
 *        calling @ref vTaskStartScheduler .
 */
const XPD_TimeServiceType xRtosTimeService = {
        .Init           = prvSysTimerSetup,
        .Block_ms       = prvTaskDelay_ms,
        .MatchBlock_ms  = prvTaskDelayToMatch,
        .DiffBlock_ms   = prvTaskDelayToDiff,
};
