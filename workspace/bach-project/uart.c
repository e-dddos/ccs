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
}

// wait until answer received
void uart_receive_msg(){
    // empty
}
