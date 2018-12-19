/**
  ******************************************************************************
  * @file    ncm_netif.c
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
#include "ncm_netif.h"
#include <string.h>

#include <netif/ethernet.h>
#include <lwip/etharp.h>
#include <lwip/apps/dhcp_server.h>

/* Ethernet (IEEE 802.3) transfer medium properties */
#define ETH_MAX_PAYLOAD_SIZE    1500
#define ETH_HEADER_SIZE         14
#define ETH_MAX_FRAME_SIZE      (ETH_HEADER_SIZE + ETH_MAX_PAYLOAD_SIZE)


static struct netif ncm_netif;
USBD_NCM_IfHandleType _ncm_usb_if, *const ncm_usb_if = &_ncm_usb_if;

/**
 * @brief Called when the USB NCM interface is opened.
 */
static void ncm_app_init(void *itf)
{
    /* Immediately report Ethernet connected state, with approximated bitrate */
#if (USBD_HS_SUPPORT == 1)
    if (ncm_usb_if->Base.Device->Speed == USB_SPEED_HIGH)
        USBD_NCM_Connect(ncm_netif.state, 100 * 1000000);
    else
#endif
        USBD_NCM_Connect(ncm_netif.state, 10 * 1000000);

    /* Set Ethernet link state */
    netif_set_link_up(&ncm_netif);
}

/**
 * @brief Called when the USB NCM interface is closed.
 */
static void ncm_app_deinit(void *itf)
{
    /* Set Ethernet link state */
    netif_set_link_down(&ncm_netif);
}

static const uint8_t ncm_hwaddr[] = { 0x00, 0x80, 0xE1, 0x00, 0x00, 0x00 };

static const USBD_NCM_AppType ncm_app = {
        .Name   = "LwIP gateway",
        .Init   = ncm_app_init,
        .Deinit = ncm_app_deinit,
        .NetAddress = &ncm_hwaddr,
};


static err_t ncm_if_init(struct netif *netif);
static err_t ncm_if_output(struct netif *netif, struct pbuf *p);

static const ip_addr_t ncm_if_ipaddr = NCM_NETIF_IPADDR;
static const ip_addr_t ncm_if_netmask = IPADDR4_INIT_BYTES(255, 255, 255, 0);

/**
 * @brief Initializes the required fields of the network interface.
 * @param netif: reference of the network interface
 * @return OK
 */
static err_t ncm_if_init(struct netif *netif)
{
    /* MAC address of the virtual server is the USB IF's MAC + 1 */
    SMEMCPY(netif->hwaddr, ncm_hwaddr, ETH_HWADDR_LEN);
    netif->hwaddr[ETH_HWADDR_LEN - 1]++;
    netif->hwaddr_len = ETH_HWADDR_LEN;
    netif->mtu        = ETH_MAX_PAYLOAD_SIZE;
    netif->flags      = 0;
    netif->output     = etharp_output;
    netif->linkoutput = ncm_if_output;
    netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;

    return ERR_OK;
}

/**
 * @brief Initializes the NCM network interface as a DHCP server.
 */
void ncm_netif_init(void)
{
    ip4_addr_t dhcp_ip4;

    ncm_usb_if->App = &ncm_app;

    netif_add(&ncm_netif, &ncm_if_ipaddr, &ncm_if_netmask, &ncm_if_ipaddr,
            ncm_usb_if, &ncm_if_init, &ethernet_input);
    netif_set_default(&ncm_netif);

    /* Start DHCP server with next address */
    ip4_addr_set_u32(&dhcp_ip4, ip_addr_get_ip4_u32(&ncm_if_ipaddr) + lwip_htonl(1));
    dhcp_server_init(&ncm_netif, &dhcp_ip4, 5);
    netif_set_up(&ncm_netif);
}

/**
 * @brief This function copies the passed datagram to the NCM transfer block.
 * @param netif: reference of the network interface
 * @param p: the MAC packet to send
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 */
static err_t ncm_if_output(struct netif *netif, struct pbuf *p)
{
    err_t retval = ERR_BUF;
    uint8_t* dest;

    do /* As lwIP doesn't retransmit, loop here until successful */
    {
        /* Cannot use USBD_NCM_PutDatagram as chained pbufs are non-linear in memory */
        dest = USBD_NCM_AllocDatagram(netif->state, p->tot_len);

        if (dest != NULL)
        {
            /* Copy all segments to the datagram */
            while (p != NULL)
            {
                SMEMCPY(dest, p->payload, p->len);
                dest += p->len;

                if (p->len == p->tot_len)
                {   break; }

                p = p->next;
            }

            /* SetDatagram must be called after a successful AllocDatagram */
            if (USBD_E_OK == USBD_NCM_SetDatagram(netif->state))
            {
                retval = ERR_OK;
            }
        }
    }
    while (retval != ERR_OK);

    return retval;
}

/**
 * @brief Passes the received datagrams to the lwIP stack as Ethernet packets.
 * @return ERR_OK if a datagram is processed, otherwise ERR_CONN
 */
static err_t ncm_netif_process_one(void)
{
    err_t retval = ERR_CONN;
    uint8_t* dg;
    uint16_t len;

    dg = USBD_NCM_GetDatagram(ncm_netif.state, &len);

    if (len > 0)
    {
        struct pbuf *p = pbuf_alloc_reference(dg, len, PBUF_ROM);

        /* Process the Ethernet frame (== ethernet_input) */
        retval = ncm_netif.input(p, &ncm_netif);
    }
    return retval;
}

/**
 * @brief Passes the received datagrams to the lwIP stack as Ethernet packets.
 */
void ncm_netif_process(void)
{
    while (ERR_OK == ncm_netif_process_one());
}
