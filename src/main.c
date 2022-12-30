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


typedef struct can2040 CANHandle;
typedef struct can2040_msg CANMsg;
struct can2040_msg tx_msg, rx_msg;

static CANHandle cbus;
volatile bool cb_called = false;
volatile uint32_t cb_notify;

char *msg_to_str(struct can2040_msg *msg) {

    static char buf[64], buf2[8];

    sprintf(buf, "[ %x ] [ %x ] [ ", msg->id, msg->dlc);
    for (uint32_t i = 0; i < msg->dlc && i < 8; i++) {
    sprintf(buf2, "%x ", msg->data[i]);
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

static void can2040_cb(struct can2040 *cd, uint32_t notify, struct can2040_msg *msg){
    // Add message processing code here...
    cb_called = true;
    cb_notify = notify;
    if (notify == CAN2040_NOTIFY_RX) {
    rx_msg = *msg;
    }
}

static void PIOx_IRQHandler(void){
    can2040_pio_irq_handler(&cbus);
}

void canbus_setup(void){
    uint32_t pio_num = 0;
    uint32_t sys_clock = 125000000, bitrate = 250000;
    uint32_t gpio_rx = 1, gpio_tx = 0;

    // Setup canbus
    can2040_setup(&cbus, pio_num);
    can2040_callback_config(&cbus, can2040_cb);

    // Enable irqs
    irq_set_exclusive_handler(PIO0_IRQ_0_IRQn, PIOx_IRQHandler);
    NVIC_SetPriority(PIO0_IRQ_0_IRQn, 1);

    // Start canbus
    can2040_start(&cbus, clock_get_hz(clk_sys), bitrate, gpio_rx, gpio_tx);
    NVIC_EnableIRQ(PIO0_IRQ_0_IRQn);
}

int main(){
    stdio_init_all();

    sleep_ms(10);

    printf("Starting Initialization of CAN\n");
    canbus_setup();
    printf("Initialized CAN\n");

    sleep_ms(1000);
    while (true) {
        CANMsg msg = {0};
        msg.dlc = 8;
        msg.data[0] = 0xDE;
        msg.data[1] = 0xAD;
        msg.data[2] = 0xBE;
        msg.data[3] = 0xEF;
        msg.data[4] = 0xDE;
        msg.data[5] = 0xAD;
        msg.data[6] = 0xBE;
        msg.data[7] = 0xEF;
        
        if(cb_called){
            switch (cb_notify) {
            // received message
            case CAN2040_NOTIFY_RX:
                printf("cb: received msg: %s\n", msg_to_str(&rx_msg));
                if(rx_msg.id & CAN2040_ID_RTR){
                    msg.id = 0x203;
                    if(can2040_check_transmit(&cbus)){
                        int res = can2040_transmit(&cbus, &msg);
                        printf("ack request for %X\n", msg.id);
                    }  
                    else{
                        printf("bus busy\n");
                    }
                }
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
            cb_called = 0;
        }
        
        uint8_t command = getchar_timeout_us(0);
        if(command == 'u' || command == 'U'){
            msg.id = 0x202 | CAN2040_ID_RTR;
            if(can2040_check_transmit(&cbus)){
                int res = can2040_transmit(&cbus, &msg);
                if (res) printf("Sent request on %X\n", msg.id);
            }  
            else{
                printf("bus busy\n");
            }
        }
        sleep_ms(500);
    }
}