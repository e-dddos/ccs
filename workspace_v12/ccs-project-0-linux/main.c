// ADC configuration in initADC() is unfinished
// => should be implemented in order to use the potentiometer to control the servo motor
// ==> this is done so that the servo motor can later be fixed into specific angles using only PWM registers

#include "inc/tm4c1294ncpdt.h"
#include "functions.h"

#define CLK_FREQ 16000000 // 16MHz of main PIOSC
// ==!!!== MEASUREMENTS_PER_SECOND has to be lower or equal to 50 ==!!!==
// this is due to the ultrasonic sensor "HC-SR04" only being able to run at a maximum of 50Hz
#define MEASUREMENTS_PER_SECOND 6 // sets period of big loop
// shortcuts to control a sensors trigger and echo pin, as well as their green and red LED
#define GREEN(X) (0x01 << (2*((X)-1))) // convert to control sensor X's green LED
#define RED(X) (0x01 << ((2*((X)-1))+1)) // convert to control sensor X's red LED
#define TRIGGER(X) (GREEN(X)) // convert to control sensor X's trigger
#define ECHO(X) (RED(X)) // convert to control sensor X's echo
#define WAIT 50000

// function prototypes
void initGPIO();
void initTimer();
void initADC();
void initUART();
uint32_t getDistance(uint8_t sensor);
void checkDistance(uint32_t distance, uint8_t sensor);
// functions for Eduard
void uart_send_msg();
int uart_receive_msg();
void get_free_directions_from_sensors(uint8_t* free_directions);

// global variables
// pwmMatch sets duty cycle for PWM signal; 63999 which is 0xF9FF is 100% duty cycle
// uint16_t pwmMatch = 63999;

void main(){
    uint8_t i = 0;
    uint32_t distance;
    uint8_t sensor = 1; // sensor X

    // setup
    initGPIO();
    initTimer();
    initUART();

    // loop
    while(1){
        /*
        // while ((UART6_FR_R & 0x20) !=0);
        UART6_DR_R = '<';
        wait(WAIT);
        UART6_DR_R = 'x';
        wait(WAIT);
        UART6_DR_R = 48 + i;
        wait(WAIT);
        UART6_DR_R = '>';
        wait(WAIT);
        printf("finished sending '<x%d>'\n", i);

        if(i < 9)
            i++;
        else
            i = 0;
        */

        while(!(TIMER0_RIS_R & 0x1)); // poll for time-out
        TIMER0_ICR_R = 0x1; // clear time-out flag
        GPIO_PORTK_DATA_R &= ~0x55; // reset green LEDs

        distance = getDistance(sensor); // retrieve distance through sensor
        //printf("distance: %umm\n", distance);
        checkDistance(distance, sensor); // check if an object is in front of the sensor

        // iterate available sensors
        if(sensor < 4)
            sensor++;
        else
            sensor = 1;
    }
}

// initialize GPIO pins
void initGPIO(){
    uint32_t ports = 0x2000 | 0x800 | 0x200 | 0x10 | 0x1; // PORT P (0x2000), PORT M (0x800), PORT K (0x200), PORT E (0x10) and PORT A (0x1)

    SYSCTL_RCGCGPIO_R |= ports; // enable selected ports
    while(SYSCTL_PRGPIO_R != ports); // wait for stable clock on PORTs

    // ultrasonic "HC-SR04" sensor; sensor 1: PA0/1, sensor 2: PA2/3, sensor 3: PA4/5, sensor 4: PA6/7
    GPIO_PORTA_AHB_DEN_R = 0xFF;
    GPIO_PORTA_AHB_DIR_R = 0x55; // PAx => x even: trigger; x odd: echo; e.g. PA0 is trigger pin of sensor 1
    GPIO_PORTA_AHB_DATA_R = 0x55; // set triggers to HIGH

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

    // ADC; PE3
    GPIO_PORTE_AHB_DEN_R = 0x00;
    GPIO_PORTE_AHB_DIR_R = 0x00;
    GPIO_PORTE_AHB_AFSEL_R = 0x08; // enable alternate function for PE3
    GPIO_PORTE_AHB_AMSEL_R = 0x08; // enable analog function for PE3
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

// PE3 as analog input
// WIP - function unfinished
void initADC(){
    uint32_t modules = 0x1; // ADC module 0 (0x1)

    SYSCTL_RCGCADC_R |= modules; // provide clock to ADC modules
    while(SYSCTL_PRADC_R != modules); // wait for stable clock on ADC modules

    // !!! WIP: unfinished !!!
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

// turn LEDs on, depending on the distance
void checkDistance(uint32_t distance, uint8_t sensor){
    if(distance <= 200)
        GPIO_PORTK_DATA_R |= RED(sensor); // turn on red LED
    else
        GPIO_PORTK_DATA_R &= ~RED(sensor); // turn off red LED
}


