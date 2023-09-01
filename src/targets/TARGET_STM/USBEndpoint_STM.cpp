/* mbed USBHost Library
 * Copyright (c) 2006-2013 ARM Limited
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

#define USBHOST_OTHER

#if defined(TARGET_STM) && defined(USBHOST_OTHER)

#include "USBHost/dbg.h"
#include "USBHost/USBEndpoint.h"
#include "USBHost/USBDeviceConnected.h"

extern uint32_t HAL_HCD_HC_GetMaxPacket(HCD_HandleTypeDef *hhcd, uint8_t chn_num);
extern uint32_t HAL_HCD_HC_GetType(HCD_HandleTypeDef *hhcd, uint8_t chn_num);
extern void HAL_HCD_DisableInt(HCD_HandleTypeDef *hhcd, uint8_t chn_num);
extern void HAL_HCD_EnableInt(HCD_HandleTypeDef *hhcd, uint8_t chn_num);

#if defined(TARGET_PORTENTA_H7)
#define USBx_BASE   USB2_OTG_FS_PERIPH_BASE
#elif defined(TARGET_GIGA)
#define USBx_BASE   USB1_OTG_HS_PERIPH_BASE
#else
#define USBx_BASE   USB1_OTG_HS_PERIPH_BASE
#endif

void USBEndpoint::init(HCED *hced_, ENDPOINT_TYPE type_, ENDPOINT_DIRECTION dir_, uint32_t size, uint8_t ep_number, HCTD *td_list_[2])
{
    HCD_HandleTypeDef *hhcd;
    uint32_t *addr;

    hced = hced_;
    type = type_;
    dir = dir_;
    setup = (type == CONTROL_ENDPOINT) ? true : false;

    //TDs have been allocated by the host
    memcpy((HCTD **)td_list, td_list_, sizeof(HCTD *) * 2); //TODO: Maybe should add a param for td_list size... at least a define
    memset(td_list_[0], 0, sizeof(HCTD));
    memset(td_list_[1], 0, sizeof(HCTD));

    td_list[0]->ep = this;
    td_list[1]->ep = this;

    address = (ep_number & 0x7F) | ((dir - 1) << 7);
    this->size = size;
    this->ep_number = ep_number;
    transfer_len = 0;
    transferred = 0;
    buf_start = 0;
    nextEp = NULL;

    td_current = td_list[0];
    td_next = td_list[1];
    /*  remove potential post pending from previous endpoint */
    ep_queue.get(0);
    intf_nb = 0;
    hhcd = (HCD_HandleTypeDef *)hced->hhcd;
    addr = &((uint32_t *)hhcd->pData)[hced->ch_num];
    *addr = 0;
    state = USB_TYPE_IDLE;
    speed = false;
}

void USBEndpoint::setSize(uint32_t size)
{
    this->size = size;
}


void USBEndpoint::setDeviceAddress(uint8_t addr)
{
    HCD_HandleTypeDef *hhcd;
    uint8_t hcd_speed = HCD_SPEED_FULL;
    /* fix me : small speed device with hub not supported
    if (this->speed) hcd_speed = HCD_SPEED_LOW; */
    if (this->speed) {
        USB_WARN("small speed device on hub not supported");
    }
    HAL_HCD_HC_Init((HCD_HandleTypeDef *)hced->hhcd, hced->ch_num, address, addr, hcd_speed,  type, size);;
    this->device_address = addr;

}

void USBEndpoint::setSpeed(uint8_t speed)
{
    this->speed = speed;
}



void USBEndpoint::setState(USB_TYPE st)
{
    /*  modify this state is possible only with a plug   */
    if (state == USB_TYPE_FREE) {
        return;
    }

    state = st;
    if (st == USB_TYPE_FREE) {
        HCD_HandleTypeDef *hhcd = (HCD_HandleTypeDef *)hced->hhcd;
        uint32_t *addr = &((uint32_t *)hhcd->pData)[hced->ch_num];
        if ((*addr) && (type != INTERRUPT_ENDPOINT)) {
            this->ep_queue.put((uint8_t *)1);
        }
        // Flush posted requests
        USBx_HC(hced->ch_num)->HCCHAR |= USB_OTG_HCCHAR_CHDIS;
        // Disable the channel
        USBx_HC(hced->ch_num)->HCCHAR |= USB_OTG_HCCHAR_CHDIS | USB_OTG_HCCHAR_CHENA;
        // Notice the lack of error handling here - after testing a lot of 
        // combinations, it seems that the USB host controller doesn't work fully
        // according to the reference manual in this aspect, which might also
        // be the reason for lacking error handling in the ST USB LL - instead,
        // we use a 50 us delay to wait for the channel to disable
        wait_us(50);

        HAL_HCD_DisableInt((HCD_HandleTypeDef *)hced->hhcd, hced->ch_num);
        *addr = 0;

    }
    if (st == USB_TYPE_ERROR) {
        // Flush posted requests
        USBx_HC(hced->ch_num)->HCCHAR |= USB_OTG_HCCHAR_CHDIS;
        // Disable the channel
        USBx_HC(hced->ch_num)->HCCHAR |= USB_OTG_HCCHAR_CHDIS | USB_OTG_HCCHAR_CHENA;
        // Notice the lack of error handling here - after testing a lot of 
        // combinations, it seems that the USB host controller doesn't work fully
        // according to the reference manual in this aspect, which might also
        // be the reason for lacking error handling in the ST USB LL - instead,
        // we use a 50 us delay to wait for the channel to disable
        wait_us(50);

        HAL_HCD_DisableInt((HCD_HandleTypeDef *)hced->hhcd, hced->ch_num);
        uint8_t hcd_speed = HCD_SPEED_FULL;
        /* small speed device with hub not supported
           if (this->speed) hcd_speed = HCD_SPEED_LOW;*/

        // If the device seems to be gone, there is no use trying to enable the channel again
        if (nullptr != dev)
        {
            // Notice that dev->getAddress() must be used instead of device_address, because the latter will contain
            // incorrect values in certain cases
            HAL_HCD_HC_Init((HCD_HandleTypeDef *)hced->hhcd, hced->ch_num, address, dev->getAddress(), hcd_speed,  type, size);
            // HAL_HCD_HC_Init() doesn't fully enable the channel after disable above, so we do it here -->
            USBx_HC(hced->ch_num)->HCCHAR &= ~USB_OTG_HCCHAR_CHDIS;
            USBx_HC(hced->ch_num)->HCCHAR |= USB_OTG_HCCHAR_CHENA;
            // <--
        }
    }
}


extern uint32_t HAL_HCD_HC_GetMaxPacket(HCD_HandleTypeDef *hhcd, uint8_t chn_num);
extern uint32_t HAL_HCD_HC_GetType(HCD_HandleTypeDef *hhcd, uint8_t chn_num);


USB_TYPE USBEndpoint::queueTransfer()
{
    HCD_HandleTypeDef *hhcd = (HCD_HandleTypeDef *)hced->hhcd;
    uint32_t *addr = &((uint32_t *)hhcd->pData)[hced->ch_num];
    uint32_t type = HAL_HCD_HC_GetType(hhcd, hced->ch_num);
    uint32_t max_size =  HAL_HCD_HC_GetMaxPacket(hhcd, hced->ch_num);
    /*  if a packet is queue on disconnected ; no solution for now */
    if (state == USB_TYPE_FREE)  {
        td_current->state =  USB_TYPE_FREE;
        return USB_TYPE_FREE;
    }
    ep_queue.get(0);
    MBED_ASSERT(*addr == 0);
    transfer_len =   td_current->size <= max_size ? td_current->size : max_size;
    buf_start = (uint8_t *)td_current->currBufPtr;

    //Now add this free TD at this end of the queue
    state = USB_TYPE_PROCESSING;
    /*  one request */
    td_current->nextTD = (hcTd *)0;
#if defined(MAX_NOTREADY_RETRY)
    td_current->retry = 0;
#endif
    td_current->setup = setup;
    *addr = (uint32_t)td_current;
    /*  dir /setup is inverted for ST */
    /* token is useful only ctrl endpoint */
    /*  last parameter is ping ? */
    HAL_HCD_HC_SubmitRequest((HCD_HandleTypeDef *)hced->hhcd, hced->ch_num, dir - 1, type, !setup, (uint8_t *) td_current->currBufPtr, transfer_len, 1);
    HAL_HCD_EnableInt((HCD_HandleTypeDef *)hced->hhcd, hced->ch_num);

    return USB_TYPE_PROCESSING;
}

void USBEndpoint::unqueueTransfer(volatile HCTD *td)
{
    if (state == USB_TYPE_FREE) {
        return;
    }
    uint32_t *addr = &((uint32_t *)((HCD_HandleTypeDef *)hced->hhcd)->pData)[hced->ch_num];
    td->state = 0;
    td->currBufPtr = 0;
    td->size = 0;
    td->nextTD = 0;
    *addr = 0;
    td_current = td_next;
    td_next = td;
}

void USBEndpoint::queueEndpoint(USBEndpoint *ed)
{
    nextEp = ed;
}
#endif
