/**
  ******************************************************************************
  * @file    ncm_netif.h
  * @author  Benedek Kupper
  * @version 0.1
  * @date    2018-12-16
  * @brief   USB-NCM interface virtually wired to lwIP server
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
#ifndef __NCM_NETIF_H_
#define __NCM_NETIF_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <lwip/netif.h>
#include <usbd_ncm.h>

extern USBD_NCM_IfHandleType *const ncm_usb_if;

void ncm_netif_init(void);
void ncm_netif_process(void);

#ifdef __cplusplus
}
#endif

#endif /* __NCM_NETIF_H_ */
