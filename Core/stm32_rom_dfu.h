/**
  ******************************************************************************
  * @file    stm32_rom_dfu.h
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
#ifndef __STM32_ROM_DFU_H_
#define __STM32_ROM_DFU_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <usbd_dfu.h>

extern USBD_DFU_IfHandleType *const stm32_rom_dfu_if;

void STM32_ROM_DFU_Init(void);
void STM32_ROM_DFU_Main(void);

#ifdef __cplusplus
}
#endif

#endif /* __STM32_ROM_DFU_H_ */
