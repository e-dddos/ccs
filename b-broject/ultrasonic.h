/*
 * ultrasonic.h
 *
 *  Created on: 01.06.2024
 *      Author: Nils
 */

#ifndef ULTRASONIC_H_
#define ULTRASONIC_H_

// header
#include "controller.h"
#include <stdio.h>
#include <stdint.h>

// function prototypes
void get_free_directions_from_sensors(uint8_t* free_directions);
void print_free_directions(uint8_t* free_directions);

#endif /* ULTRASONIC_H_ */
