/*
 * controller.h
 *
 *  Created on: 01.06.2024
 *      Author: Nils
 */

#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include "inc/tm4c1294ncpdt.h"
#include <stdio.h>
#include <stdint.h>

#define CLK_FREQ 16000000 // 16MHz of main PIOSC
// ==!!!== MEASUREMENTS_PER_SECOND has to be lower or equal to 50 ==!!!==
// this is due to the ultrasonic sensor "HC-SR04" only being able to run at a maximum of 50Hz
#define MEASUREMENTS_PER_SECOND 0.5 // sets period of big loop
// shortcuts to control a sensors trigger and echo pin, as well as their green and red LED
#define GREEN(X) (0x01 << (2*((X)-1))) // convert to control sensor X's green LED
#define RED(X) (0x01 << ((2*((X)-1))+1)) // convert to control sensor X's red LED
#define TRIGGER(X) (GREEN(X)) // convert to control sensor X's trigger
#define ECHO(X) (RED(X)) // convert to control sensor X's echo
#define FIELDSIZE 500 // size of one field in mm

// function prototypes
void initGPIO();
void initTimer();
void initUART();
void initSetup();
uint32_t getDistance(uint8_t sensor);
uint8_t checkDistance(uint32_t threshold, uint32_t distance, uint8_t sensor);
void sleep();

#endif /* CONTROLLER_H_ */
