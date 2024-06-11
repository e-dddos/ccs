/*
 * ultrasonic.c
 *
 *  Created on: 01.06.2024
 *      Author: Nils
 */



#include "ultrasonic.h"

void get_free_directions_from_sensors(uint8_t* free_directions){
    uint32_t distance[4] =  {0}; // keep track of individual distances of the sensors
    uint8_t i = 0; // for-loop iterator

    while(!(TIMER0_RIS_R & 0x1)); // poll for time-out
    TIMER0_ICR_R = 0x1; // clear time-out flag
    GPIO_PORTK_DATA_R &= ~0x55; // reset green LEDs

    // iterate available sensors
    for(i = 0; i < 4; i++){
        distance[i] = getDistance(i+1);
        free_directions[i] = checkDistance(FIELDSIZE, distance[i], i+1); // check if an object is in front of the sensor
    }
}

void print_free_directions(uint8_t* free_directions){
    uint8_t i = 0; // for-loop iterator

    for(i = 0; i < 4; i++){
        switch(i){
            case 0: printf("Front: ");
                break;
            case 1: printf("Right: ");
                break;
            case 2: printf("Back : ");
                break;
            case 3: printf("Left : ");
                break;
        }

        printf("%s\n", free_directions[i]?"free":"object found");
    }
    printf("\n\n\n");
}
