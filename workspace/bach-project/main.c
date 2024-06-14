#include "controller.h"
#include "navigate.h"
#include "ultrasonic.h"
#include "uart.h"

int main()
{
    //Create our Roboot:
    struct Robot myrobot;

    initSetup();
    robot_init(&myrobot);
    go_go_spiral(&myrobot);
    return 0;
}
