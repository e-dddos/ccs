#include "functions.h"

// wait a bit, length specified by [interval]
void wait(uint32_t interval){
    uint32_t i;

    for(i = 0; i < interval; i++); // empty for-loop in order to wait for [interval]
}

// get average value of numbers in [vector] of size [length]
// only works for vectors full of natural numbers
uint32_t getAverage(uint32_t* vector, uint32_t length){
    uint32_t i, average = 0;

    for(i = 0; i < length; i++){
        // printf("s_%u: %umm\n", (i+1), vector[i]); // print each sample
        average += vector[i]; // calculate sum of vector elements
    }

    // WIP: an odd length will result in adding less than 0.5 to the final result
    average += (length/2); // used to account for rounding (=> is equal to adding 0.5 to the final result and flooring)
    average /= length; // divide by length of vector

    return average;
}

/*
// get average value of numbers in [vector] of size [length]
// only works for vectors full of natural numbers
uint32_t getAverage(uint32_t* vector, uint32_t length){
    uint32_t i, average = 0;

    for(i = 0; i < length; i++){
        average += (uint32_t)(vector[i]/length); // calculate average of vector
    }

    return average;
}
*/

// get maximum value of numbers in [vector] of size [length]
// only works for vectors full of natural numbers
uint32_t getMaximum(uint32_t* vector, uint32_t length){
    uint32_t i, maximum = 0;

    for(i = 0; i < length; i++){
        if(vector[i] > maximum) // check for new maximum
            maximum = vector[i]; // update maximum value
    }

    return maximum;
}
