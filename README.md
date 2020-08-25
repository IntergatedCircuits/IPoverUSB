# IP over USB [![Build Status](https://travis-ci.org/IntergatedCircuits/IPoverUSB.svg?branch=master)](https://travis-ci.org/IntergatedCircuits/IPoverUSB)

This repository contains the STM32 embedded firmware demonstration of a virtual network
with a single lwIP server (DNS, DHCP, HTTP) presented on the USB network adapter interface.
It's meant to be a starting point for IP over USB applications (using CDC-NCM protocol)
and a low-cost IP stack development platform.

There are two possible supported software configurations:
1. A bare-metal implementation using the lwIP native API.
2. An RTOS implementation using FreeRTOS and the lwIP netconn API.

## How to use

1. Build and flash the image on an STM32F4DISCOVERY or a 32L476GDISCOVERY (easily portable to other STM32 targets)
2. Connect to PC via USB
3. Fix any driver issues, see below
4. Go to http://www.lwip.home

## Features

* Using NCM implementation of [USBDevice][USBDevice] (double-buffered, efficient, simple API)
* [lwIP][lwIP] 2.1.0 with additional dummy DHCP server for out of the box operation
* DNS server implementation allows domain name based access
* Reprogramming via USB supported by DFU interface (DFU standard implementation to reboot to ROM)
* [FreeRTOS][FreeRTOS] variant allows the choice of any lwIP APIs to be used by the application

[FreeRTOS]: https://www.freertos.org/
[lwIP]: https://savannah.nongnu.org/projects/lwip/
[USBDevice]: https://github.com/IntergatedCircuits/USBDevice
