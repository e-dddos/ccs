#include "controller.h"
#include "navigate.h"
#include "ultrasonic.h"
#include "uart.h"

int main()
{
    struct Robot myrobot;

    initSetup();
    robot_init(&myrobot);
    printf("%d", MAXLENGTH);
    go_go_spiral(&myrobot);
    print_room(&myrobot);
    printf("We finished!!!\n");
    return 0;
}
