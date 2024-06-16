/*
 * uart.h
 *
 *  Created on: 01.06.2024
 *      Author: Nils
 */

#ifndef UART_H_
#define UART_H_

// header
#include "controller.h"
#include <stdio.h>
#include <stdint.h>

// function prototypes
void uart_send_msg(char* msg);
void receive_okay(int dev);

#endif /* UART_H_ */
