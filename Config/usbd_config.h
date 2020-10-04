/**
  ******************************************************************************
  * @file    usbd_config.h
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2018-01-31
  * @brief   Universal Serial Bus Device Driver
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
#ifndef __USBD_CONFIG_H_
#define __USBD_CONFIG_H_

#ifdef __cplusplus
extern "C"
{
#endif

/** @addtogroup USBD
 * @{ */

/** @addtogroup USBD_Exported_Macros USBD Exported Macros
 * @{ */

#define USBD_MAX_IF_COUNT           8

#define USBD_EP0_BUFFER_SIZE        512

/** @brief Set to 1 if DFU STMicroelectronics Extension
 *  protocol (v1.1A) shall be used instead of the standard DFU (v1.1). */
#define USBD_DFU_ST_EXTENSION       1

/** @brief Selects which Microsoft OS descriptor specification should be used (if any).
 * Supported values are: 0, 1, 2
 * @note Microsoft OS 2.0 descriptors are supported by Windows 8.1 and higher.
 * Unless the device is required to operate on earlier Windows OS versions, use version 2. */
#define USBD_MS_OS_DESC_VERSION     2

/** @} */

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __USBD_CONFIG_H_ */
