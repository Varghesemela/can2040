#include <RP2040.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <hardware/pio.h>
#include <hardware/dma.h>
#include <hardware/uart.h>
#include <hardware/flash.h>
#include <hardware/watchdog.h>
#include <pico/bootrom.h>
#include <hardware/irq.h>
#include <core_cm0plus.h>
#include <hardware/regs/intctrl.h>

#include "can2040.h"
#define CANMSG_DATA_LEN(msg) ((msg)->dlc > 8 ? 8 : (msg)->dlc)

typedef struct can2040 CANHandle;
typedef struct can2040_msg CANMsg;

static CANHandle cbus;
bool flag = 0;
uint8_t data_len = 0;
uint8_t rbuf[8];

static void can2040_cb(CANHandle *cd, uint32_t notify, CANMsg *msg)
{
    // Add message processing code here...
    if (notify & CAN2040_NOTIFY_TX) {
        printf("CANTX\n");
        return;
    }
    if (notify & CAN2040_NOTIFY_RX){
        printf("CANRX\n");
        if((msg->id == 0x202) || (msg->id == 0x036)){
            data_len = CANMSG_DATA_LEN(msg);
            memcpy(rbuf, msg->data, data_len);
            flag = 1;
        }
    }
}

static void
PIOx_IRQHandler(void)
{
    can2040_pio_irq_handler(&cbus);
}

void
canbus_setup(void)
{
    uint32_t pio_num = 0;
    uint32_t sys_clock = 125000000, bitrate = 500000;
    uint32_t gpio_rx = 1, gpio_tx = 0;

    // Setup canbus
    can2040_setup(&cbus, pio_num);
    can2040_callback_config(&cbus, can2040_cb);

    // Enable irqs
    irq_set_exclusive_handler(PIO0_IRQ_0_IRQn, PIOx_IRQHandler);
    NVIC_SetPriority(PIO0_IRQ_0_IRQn, 1);
    NVIC_EnableIRQ(PIO0_IRQ_0_IRQn);

    // Start canbus
    can2040_start(&cbus, sys_clock, bitrate, gpio_rx, gpio_tx);
}

int main(){
    stdio_init_all();

    sleep_ms(10);
    printf("Startup delay over\n");

    printf("Starting Initialization of CAN\n");
    canbus_setup();
    printf("Initialized CAN\n");

    sleep_ms(1000);
    while (true) {
        CANMsg txmsg = {0};
        txmsg.dlc = 8;
        txmsg.id = 0x303;
        txmsg.data[0] = 0xDE;
        txmsg.data[1] = 0xAD;
        txmsg.data[2] = 0xBE;
        txmsg.data[3] = 0xEF;
        txmsg.data[4] = 0xDE;
        txmsg.data[5] = 0xAD;
        txmsg.data[6] = 0xBE;
        txmsg.data[7] = 0xEF;
        
        // if(can2040_check_transmit(&cbus)){
        //     int res = can2040_transmit(&cbus, &txmsg);
        //     printf("Returned: %d\n", res);
        // }
        if (flag){
            for(int i=0; i<8; i++){
                printf("%X ", rbuf[i]);
            }
            printf("\n");
            flag = 0;
        }
        sleep_ms(500);
    }
}