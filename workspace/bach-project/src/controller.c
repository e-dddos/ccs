/*
 * controller.c
 *
 *  Created on: 01.06.2024
 *      Author: Nils
 */



#include "controller.h"

// initialize GPIO pins
void initGPIO(){
    uint32_t ports = 0x2000 | 0x800 | 0x200 | 0x2 | 0x1 | 0x100; // PORT P (0x2000), PORT M (0x800), PORT K (0x200),
                                                   // PORT B (0x2) and PORT A (0x1)

    SYSCTL_RCGCGPIO_R |= ports; // enable selected ports
    while(SYSCTL_PRGPIO_R != ports); // wait for stable clock on PORTs

    // ultrasonic "HC-SR04" sensor; sensor 1: PA0/1, sensor 2: PA2/3, sensor 3: PA4/5, sensor 4: PA6/7
    GPIO_PORTA_AHB_DEN_R = 0xFF;
    GPIO_PORTA_AHB_DIR_R = 0x55; // PAx => x even: trigger; x odd: echo; e.g. PA0 is trigger pin of sensor 1
    GPIO_PORTA_AHB_DATA_R = 0x55; // set triggers to HIGH

    // Receive lines
    GPIO_PORTB_AHB_DEN_R = 0x30; //PB4 and PB5 as inputs
    GPIO_PORTB_AHB_DIR_R = 0x00;
    // GPIO_PORTB_AHB_PUR_R = 0x30; // pull-up resistors for PB5 and PB4
    GPIO_PORTB_AHB_PDR_R = 0x30; // pull-down resistors for PB5 and PB4

    // UART
    GPIO_PORTP_DEN_R = 0x03; // PP1 (U6Tx) and PP0 (U6Rx) for UART6
    GPIO_PORTP_DIR_R = 0x02;
    GPIO_PORTP_AFSEL_R = 0x03; // enable alternate function for PP1 and PP0
    // GPIO_PORTP_DR4R_R = 0x11; // provide 4mA drive for UART6
    // GPIO_PORTP_SLR_R = 0x00; // disable slew rate control
    GPIO_PORTP_PCTL_R = 0x11; // enable UART6 module on PP1 and PP0

    // LEDs, convey information of the ultrasonic sensor; sensor 1: PK0/1, sensor 2: PK2/3, sensor 3: PK4/5, sensor 4: PK6/7
    GPIO_PORTK_DEN_R = 0xFF;
    GPIO_PORTK_DIR_R = 0xFF; // PKx => x even: green (successfully received pulse), x odd: red (object found)
    GPIO_PORTK_DATA_R = 0x00; // can be used to turn LEDs on

    // PWM; PM0
    GPIO_PORTM_DEN_R = 0x01;
    GPIO_PORTM_DIR_R = 0x01;
    GPIO_PORTM_AFSEL_R = 0x1; // enable alternate function for PM0
    GPIO_PORTM_PCTL_R = 0x3; // enable T2CCP0 function for PM0; TIMER2A can now provide PWM signal to PM0
}

// initialize timers
void initTimer(){
    uint32_t timers = 0x8 | 0x4 | 0x2 | 0x1; // timer3 (0x8), timer2 (0x4), timer1 (0x2) and timer0 (0x1)

    SYSCTL_RCGCTIMER_R |= timers; // provide clock to timers
    while(SYSCTL_PRTIMER_R != timers); // wait for stable clock on timers

    // timer0; controls period of big loop
    TIMER0_CTL_R = 0x0; // disable timer
    TIMER0_CFG_R = 0x0; // 32-bit mode
    TIMER0_TAMR_R = 0x2; // periodic mode
    // TIMERx_TnILR_R = (CLK_FREQ / f_timer)-1 sets timer with period of 1/f_timer
    TIMER0_TAILR_R = (unsigned int)(CLK_FREQ / (MEASUREMENTS_PER_SECOND))-1;
    TIMER0_ICR_R = 0x1; // clear time-out flag
    TIMER0_CTL_R |= 0x1; // enable timer
    // while(!(TIMER0_RIS_R & 0x1)); // poll for time-out
    // TIMER0_ICR_R = 0x1; // clear time-out flag
    // TIMER0_CTL_R &= ~0x1; // disable timer

    // timer1; measures length of echo pulse of ultrasonic sensor
    TIMER1_CTL_R = 0x0; // disable timer
    TIMER1_CFG_R = 0x0; // 32-bit mode
    TIMER1_TAMR_R = 0x10 | 0x1; // upwards counting (0x10) and one-shot mode (0x1)
    TIMER1_TAILR_R = ~0x0; // cut-off for time-out event to max value
    TIMER1_ICR_R = 0x1; // clear time-out flag
    // TIMER1_CTL_R |= 0x1; // enable timer
    // TIMER1_CTL_R &= ~0x1; // disable timer
    // TIMER1_ICR_R = 0x1; // clear time-out flag
    // TIMER1_TAV_R = 0; // reset counter to 0, can also be read for current count value

    // timer2; PWM signal
    TIMER2_CTL_R = 0x0; // disable timer
    TIMER2_CFG_R = 0x4; // 16-bit mode
    TIMER2_TAMR_R = 0x8 | 0x2; // PWM mode (0x8) and periodic mode (0x2); also clears 0x4 bit for edge-count mode
    TIMER2_CTL_R = 0x40; // inverted PWM signal
    TIMER2_TAPR_R = 5-1; // divide the clock by 5 (=> 16MHz / 5 = 3.2MHz)
    TIMER2_TAILR_R = 64000-1; // (prescaler*load_interval)/CLK_FREQ = 1/f_timer => set to 50Hz PWM for servo motor
    // 3200-1 => 1ms pulse
    TIMER2_TAMATCHR_R = 0; // match value for PWM, PWM signal has rising edge at pwmMatch value in inverted mode
    TIMER2_CTL_R |= 0x1; // enable timer

    // timer3A; provides 10us trigger TTL pulse
    TIMER3_CTL_R = 0x0; // disable timer
    TIMER3_CFG_R = 0x0; // 16-bit mode (0x4)
    TIMER3_TAMR_R = 0x1; // one-shot mode
    TIMER3_TAILR_R = 160-1; // set 10us timer (160)
    TIMER3_ICR_R = 0x1; // clear time-out flag
    // TIMER3_CTL_R |= 0x1; // enable timer
    // while(!(TIMER3_RIS_R & 0x1)); // poll for time-out
    // TIMER3_ICR_R = 0x1; // clear time-out flag
    // TIMER3_CTL_R &= ~0x1; // disable timer
}

void initUART(){
    uint32_t uart = 0x40; // UART6 (0x40)

    SYSCTL_RCGCUART_R = uart;
    while(SYSCTL_PRUART_R != uart); // wait for stable clock on UART modules

    UART6_CTL_R &= ~0x1; // disable UART6
    UART6_IBRD_R = 8; // IBRD together with FBRD realize a baud rate of 115200
    UART6_FBRD_R = 44;

    UART6_LCRH_R = 0x60; // UART as 8N1 (0x60) and 1-byte-deep FIFO (cleared 0x10 bit)
    UART6_CC_R = 0x0; // use system clock for baud rate
    UART6_CTL_R |= 0x200 | 0x1; // enable Rx (0x200), enable Tx (0x100) and UART6 (0x1)
}

void initSetup(){
    // only initialize board after USR_SW1 button press
    SYSCTL_RCGCGPIO_R |= 0x100; // enable PORT J
    while(SYSCTL_PRGPIO_R != 0x100); // wait for stable clock on PORT J

    // enable PJ0 => USR_SW1
    GPIO_PORTJ_AHB_DEN_R |= 0x1;
    GPIO_PORTJ_AHB_DIR_R &= ~0x1; // PJ0 as input
    GPIO_PORTJ_AHB_PUR_R |= 0x1; // add internal pull-up resistor

    printf("Waiting for button press...\n");
    while((GPIO_PORTJ_AHB_DATA_R && 0x1) == 0x1);
    printf("Initializing Program.\n");

    initGPIO();
    initTimer();
    initUART();
}

// return distance in mm using the ultrasonic sensor
uint32_t getDistance(uint8_t sensor){
    uint32_t duration, distance;

    // send pulse
    GPIO_PORTA_AHB_DATA_R &= ~TRIGGER(sensor); // set trigger to LOW
    TIMER3_CTL_R |= 0x1; // enable timer
    while(!(TIMER3_RIS_R & 0x1)); // poll for time-out
    GPIO_PORTA_AHB_DATA_R |= TRIGGER(sensor); // reset trigger to HIGH

    // measure echo signal's pulse length
    while(!(GPIO_PORTA_AHB_DATA_R & ECHO(sensor))); // wait until echo reaches HIGH
    TIMER1_CTL_R |= 0x1; // enable timer
    while(GPIO_PORTA_AHB_DATA_R & ECHO(sensor)); // wait until echo returns to LOW
    TIMER1_CTL_R &= ~0x1; // disable timer

    // calculate distance
    duration = TIMER1_TAV_R / (CLK_FREQ/1000000); // duration of pulse in us; TIMER1_TAV is the current timer value
    distance = 343 * (duration/2) / 1000; // distance in mm

    // reset for next function call
    TIMER1_TAV_R = 0; // reset count value to 0
    TIMER1_ICR_R = 0x1; // clear time-out flag
    TIMER3_CTL_R &= ~0x1; // disable timer
    TIMER3_ICR_R = 0x1; // clear time-out flag

    GPIO_PORTK_DATA_R |= GREEN(sensor); // turn on green LED
    return distance;
}

// turn LEDs on if object within a specified distance threshold; return 1 if no object within threshold
uint8_t checkDistance(uint32_t threshold, uint32_t distance, uint8_t sensor){
    if(distance <= threshold){
        GPIO_PORTK_DATA_R |= RED(sensor); // turn on red LED
        return 0;
    } else{
        GPIO_PORTK_DATA_R &= ~RED(sensor); // turn off red LED
        return 1;
    }
}

//sleep for 10 micro seconds
void sleep() {
    TIMER3_CTL_R |= 0x1; // enable timer
    while(!(TIMER3_RIS_R & 0x1)); // poll for time-out
    TIMER3_CTL_R &= ~0x1; // disable timer
    TIMER3_ICR_R = 0x1; // clear time-out flag
}
