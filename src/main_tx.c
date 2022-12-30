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
#include "hardware/clocks.h"
#include <hardware/regs/intctrl.h>

#include "can2040.h"
#define CANMSG_DATA_LEN(msg) ((msg)->dlc > 8 ? 8 : (msg)->dlc)

typedef struct can2040 CANHandle;
typedef struct can2040_msg CANMsg;

static CANHandle cbus;
bool flag = 0;
uint8_t data_len = 0;
uint8_t rbuf[8];
volatile bool cb_called = false;
volatile uint32_t cb_notify;
struct can2040_msg tx_msg, rx_msg;

/// format CAN message as a string

char *msg_to_str(struct can2040_msg *msg) {

    static char buf[64], buf2[8];

    sprintf(buf, "[ %lu ] [ %lu ] [ ", msg->id, msg->dlc);
    for (uint32_t i = 0; i < msg->dlc && i < 8; i++) {
    sprintf(buf2, "%u ", msg->data[i]);
    strcat(buf, buf2);
    }
    strcat(buf, " ] ");

    if (msg->id & CAN2040_ID_RTR) {
    strcat(buf, "R");
    }

    if (msg->id & CAN2040_ID_EFF) {
    strcat(buf, "X");
    }
    
    return buf;
}

static void can2040_cb(CANHandle *cd, uint32_t notify, CANMsg *msg)
{
    // Add message processing code here...
    // if (notify & CAN2040_NOTIFY_TX) {
    //     printf("CANTX\n");
    //     return;
    // }
    // if (notify & CAN2040_NOTIFY_RX){
    //     printf("CANRX\n");
    //     data_len = CANMSG_DATA_LEN(msg);
    //     memcpy(rbuf, msg->data, data_len);
    //     flag = 1;
    // }

     (void)(cd);
    cb_called = true;
    cb_notify = notify;

    if (notify == CAN2040_NOTIFY_RX) {
    rx_msg = *msg;
    }

  return;
}

static void __time_critical_func(PIO0_IRQHandler)(void)
{
    can2040_pio_irq_handler(&cbus);
}

void
canbus_setup(void)
{
    uint32_t pio_num = 0;
    uint32_t bitrate = 250000;
    uint32_t gpio_rx = 1, gpio_tx = 0;

    // Setup canbus
    can2040_setup(&cbus, pio_num);
    can2040_callback_config(&cbus, can2040_cb);

    // Start canbus
    can2040_start(&cbus, clock_get_hz(clk_sys), bitrate, gpio_rx, gpio_tx);

    // Enable irqs
    irq_set_exclusive_handler(PIO0_IRQ_0_IRQn, PIO0_IRQHandler);
    NVIC_SetPriority(PIO0_IRQ_0_IRQn, 1);
    NVIC_EnableIRQ(PIO0_IRQ_0_IRQn);

}

int main(){
    stdio_init_all();

    sleep_ms(1000);
    printf("Startup delay over\n");

    printf("Starting Initialization of CAN\n");
    canbus_setup();
    printf("Initialized CAN\n");

    sleep_ms(1000);
    while (true) {

        uint8_t command = getchar_timeout_us(0);
        if(command == 'u' || command == 'U'){
            CANMsg txmsg = {0};
            txmsg.dlc = 8;
            txmsg.id = 0x202;
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
            // printf("not transmittin\n");

        }
        else if(command == 't'){
            for(int i=0; i<8; i++){
                printf("%X ", rx_msg.data[i]);
            }
            printf("\n");
        }

        if (cb_called) {

        cb_called = false;
        printf("callback called\n");


        switch (cb_notify) {
        // received message
        case CAN2040_NOTIFY_RX:
            printf("cb: received msg: %s\n", msg_to_str(&rx_msg));
            break;

        // message sent ok
        case CAN2040_NOTIFY_TX:
            printf("cb: message sent ok\n");
            break;

        // error
        case CAN2040_NOTIFY_ERROR:
            printf("cb: an error occurred\n");
            break;

        // unknown
        default:
            printf("cb: unknown notification = %lu\n", cb_notify);
        }
    }
    //     CANMsg txmsg = {0};
    //     txmsg.dlc = 8;
    //     txmsg.id = 0x202;
    //     txmsg.data[0] = 0xDE;
    //     txmsg.data[1] = 0xAD;
    //     txmsg.data[2] = 0xBE;
    //     txmsg.data[3] = 0xEF;
    //     txmsg.data[4] = 0xDE;
    //     txmsg.data[5] = 0xAD;
    //     txmsg.data[6] = 0xBE;
    //     txmsg.data[7] = 0xEF;
        
    //     if(can2040_check_transmit(&cbus)){
    //         int res = can2040_transmit(&cbus, &txmsg);
    //         printf("Returned: %d\n", res);
    //     }
    //     if (flag){
    //         for(int i=0; i<8; i++){
    //             printf("%X ", rbuf[i]);
    //         }
    //         printf("\n");
    //         flag = 0;
    //     }
    //     sleep_ms(500);
    }
}