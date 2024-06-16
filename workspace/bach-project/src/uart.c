/*
 * uart.c
 *
 *  Created on: 01.06.2024
 *      Author: Nils
 */



#include "uart.h"

// send custom message [msg] via UART
void uart_send_msg(char* msg){
    uint8_t i = 0; // for-loop iterator

    while(msg[i] != '\0'){
        while ((UART6_FR_R & 0x20) != 0x0);
        UART6_DR_R = msg[i];
        i++;
    }
    printf("finished sending '%s'\n", msg);
    //getchar();
}

// busy-waiting checking if device is ready (HIGH) or busy (LOW)
void receive_okay(int dev){
    switch(dev){
        case 0: while((GPIO_PORTB_AHB_DATA_R && 0x10) == 0x0); // PB4
                break;
        case 1: while((GPIO_PORTB_AHB_DATA_R && 0x20) == 0x0); // PB5
                break;
        default: printf("device couldn't be located.\n");
    }
}
