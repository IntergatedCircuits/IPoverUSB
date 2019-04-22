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

#if (NO_SYS == 0)
#include <lwip/sys.h>
/* lwIP API is used for threading,
 * this include is only necessary for portYIELD_FROM_ISR */
#include <FreeRTOS.h>

#define NCM_NETIF_STACKSIZE     1024
#define NCM_NETIF_PRIO          4
#define NCM_NETIF_MBOX_SIZE     4

/* Post an event to the thread's mailbox,
 * and notify scheduler if the thread should be switched to
 * (as it's priority is higher than the current task) */
#define NCM_POST_EVENT_ISR(NCM_NETIF, MSG)   \
    portYIELD_FROM_ISR((ERR_NEED_SCHED == sys_mbox_trypost_fromisr(&(NCM_NETIF)->events, (void*)MSG)))
#endif

/* Ethernet (IEEE 802.3) transfer medium properties */
#define ETH_MAX_PAYLOAD_SIZE    1500
#define ETH_HEADER_SIZE         14
#define ETH_MAX_FRAME_SIZE      (ETH_HEADER_SIZE + ETH_MAX_PAYLOAD_SIZE)

struct ncm_netif {
    struct netif netif;
    USBD_NCM_IfHandleType ncmif;
#if (NO_SYS == 0)
    sys_mbox_t events;
#endif
};

static void ncm_app_init(void *itf);
static void ncm_app_deinit(void *itf);
#if (NO_SYS == 0)
static void ncm_app_received(void *itf);
#endif
static err_t ncm_if_init(struct netif *netif);
static err_t ncm_if_output(struct netif *netif, struct pbuf *p);

/* HW (MAC) address of the USB host */
static const uint8_t ncm_hwaddr[] = { 0x00, 0x80, 0xE1, 0x00, 0x00, 0x00 };

static const USBD_NCM_AppType ncm_app = {
        .Name   = "LwIP gateway",
        .Init   = ncm_app_init,
        .Deinit = ncm_app_deinit,
#if (NO_SYS == 0)
        .Received = ncm_app_received,
#endif
        .NetAddress = &ncm_hwaddr,
};

/* IP address of the lwIP server, connected to the USB host */
static const ip_addr_t ncm_if_ipaddr = NCM_NETIF_IPADDR;
static const ip_addr_t ncm_if_netmask = IPADDR4_INIT_BYTES(255, 255, 255, 0);

#if (NO_SYS == 0)
enum {
    NCM_EV_LINK_DOWN = 0,
    NCM_EV_LINK_UP,
    NCM_EV_RECEIVED,
};
static const uint8_t ncm_ev_link_down   = NCM_EV_LINK_DOWN;
static const uint8_t ncm_ev_link_up     = NCM_EV_LINK_UP;
static const uint8_t ncm_ev_received    = NCM_EV_RECEIVED;
#endif

/* Use a single handle as multiple interfaces are a rare use-case */
struct ncm_netif ncm_net_if;
USBD_NCM_IfHandleType *const ncm_usb_if = &ncm_net_if.ncmif;

/**
 * @brief Called when the USB NCM interface is opened.
 * @param itf: reference to the USB NCM interface
 */
static void ncm_app_init(void *itf)
{
    struct ncm_netif *ncm_netif = container_of(itf, struct ncm_netif, ncmif);

    /* Immediately report Ethernet connected state, with approximated bitrate */
#if (USBD_HS_SUPPORT == 1)
    if (ncm_netif->ncmif.Base.Device->Speed == USB_SPEED_HIGH)
        USBD_NCM_Connect(itf, 100 * 1000000);
    else
#endif
        USBD_NCM_Connect(itf, 10 * 1000000);

#if (NO_SYS == 1)
    /* Set Ethernet link state */
    netif_set_link_up(&ncm_netif->netif);
#else
    NCM_POST_EVENT_ISR(ncm_netif, &ncm_ev_link_up);
#endif
}

/**
 * @brief Called when the USB NCM interface is closed.
 * @param itf: reference to the USB NCM interface
 */
static void ncm_app_deinit(void *itf)
{
    struct ncm_netif *ncm_netif = container_of(itf, struct ncm_netif, ncmif);

#if (NO_SYS == 1)
    /* Set Ethernet link state */
    netif_set_link_down(&ncm_netif->netif);
#else
    NCM_POST_EVENT_ISR(ncm_netif, &ncm_ev_link_down);
#endif
}

#if (NO_SYS == 0)
/**
 * @brief Signals the reception of new datagrams.
 * @param itf: reference to the USB NCM interface
 */
static void ncm_app_received(void *itf)
{
    struct ncm_netif *ncm_netif = container_of(itf, struct ncm_netif, ncmif);

    NCM_POST_EVENT_ISR(ncm_netif, &ncm_ev_received);
}
#endif

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
#if (NO_SYS == 0)
        else
        {
            /* TODO: log occurrences
             * Maybe delay the current thread to prevent starving others,
             * but effect on lwIP call stack needs to be considered. */
        }
#endif
    }
    while (retval != ERR_OK);

    return retval;
}

/**
 * @brief Passes the received datagrams to the lwIP stack as Ethernet packets.
 * @param ncm_netif: reference to the interface container structure
 * @return ERR_OK if a datagram is processed, otherwise ERR_CONN
 */
static err_t ncm_netif_process_one(struct ncm_netif *ncm_netif)
{
    err_t retval = ERR_CONN;
    uint8_t* dg;
    uint16_t len;

    dg = USBD_NCM_GetDatagram(&ncm_netif->ncmif, &len);

    if (len > 0)
    {
        struct pbuf *p = pbuf_alloc_reference(dg, len, PBUF_ROM);

        /* Process the Ethernet frame (== ethernet_input) */
        retval = ncm_netif->netif.input(p, &ncm_netif->netif);
    }
    return retval;
}

#if (NO_SYS == 1)
/**
 * @brief Passes the received datagrams to the lwIP stack as Ethernet packets.
 */
void ncm_netif_process(void)
{
    struct ncm_netif *ncm_netif = &ncm_net_if;

    while (ERR_OK == ncm_netif_process_one(ncm_netif));
}
#else
/**
 * @brief This thread receives the NCM events in blocking mode and processes them.
 * @param ncm_netif: reference to the interface container structure
 */
static void ncm_netif_thread(struct ncm_netif *ncm_netif)
{
    uint8_t *event;

    while (1) /* event loop */
    {
        /* Wait for next event indefinitely */
        if (SYS_ARCH_TIMEOUT != sys_arch_mbox_fetch(&ncm_netif->events, (void**)&event, 0))
        {
            switch (*event)
            {
                case NCM_EV_LINK_DOWN:
                    netif_set_link_down(&ncm_netif->netif);
                    break;
                case NCM_EV_LINK_UP:
                    netif_set_link_up(&ncm_netif->netif);
                    break;
                case NCM_EV_RECEIVED:
                    /* Consume all received datagrams */
                    while (ERR_OK == ncm_netif_process_one(ncm_netif));
                    break;
            }
        }
    }
}
#endif

/**
 * @brief Initializes the NCM network interface as a DHCP server.
 */
void ncm_netif_init(void)
{
    struct ncm_netif *ncm_netif = &ncm_net_if;
    ip4_addr_t dhcp_ip4;

    ncm_netif->ncmif.App = &ncm_app;

    netif_add(&ncm_netif->netif, &ncm_if_ipaddr, &ncm_if_netmask, &ncm_if_ipaddr,
            &ncm_netif->ncmif, &ncm_if_init, &ethernet_input);
    netif_set_default(&ncm_netif->netif);

    /* Start DHCP server with next address */
    ip4_addr_set_u32(&dhcp_ip4, ip_addr_get_ip4_u32(&ncm_if_ipaddr) + lwip_htonl(1));
    dhcp_server_init(&ncm_netif->netif, &dhcp_ip4, 5);
    netif_set_up(&ncm_netif->netif);

#if (NO_SYS == 0)
    /* Create events mailbox and processing thread */
    sys_mbox_new(&ncm_netif->events, NCM_NETIF_MBOX_SIZE);

    sys_thread_new("NCM-IF", (lwip_thread_fn)ncm_netif_thread, ncm_netif,
            NCM_NETIF_STACKSIZE, NCM_NETIF_PRIO);
#endif
}
