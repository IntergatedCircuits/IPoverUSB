/**
  ******************************************************************************
  * @file    xpd_config.h
  * @author  Benedek Kupper
  * @version 0.2
  * @date    2018-01-28
  * @brief   STM32 eXtensible Peripheral Drivers configuration template
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
#ifndef __XPD_CONFIG_H_
#define __XPD_CONFIG_H_

/* TODO step 1: specify device header */
#include <stm32f407xx.h>

/* TODO step 2: enable desired used XPD modules error handling */

/* TODO step 3: specify power supplies */
#define VDD_VALUE_mV                    3000 /* Value of VDD in mV */
#define VDDA_VALUE_mV                   3000 /* Value of VDD Analog in mV */

/* TODO step 4: specify oscillator parameters */
#define HSE_VALUE_Hz                    80000000

#endif /* __XPD_CONFIG_H_ */
