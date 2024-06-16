#include "navigate.h"
#include "ultrasonic.h"
#include "uart.h"

void robot_init(struct Robot *myrobot)
{

    // Start position:
    myrobot->x_pos = 1;
    myrobot->y_pos = 1;
    myrobot->direction = 0;
    myrobot->steps_done = 0;
    myrobot->fields_visited = 0;
    myrobot->cant_move = false;
    myrobot->finished = false;

    /*Room array, the walls have the value 0xFF, this will be important later,
     the not yet discovered fields have the value 0*/
    // Create the room array with the walls:
    uint8_t x;
    uint8_t y;
    for (x = 0; x < MAXLENGTH; x++)
    {
        for (y = 0; y < MAXWIDTH; y++)
        {
            if ((x == 0) || (x == (MAXLENGTH - 1)) || (y == 0)
                    || (y == (MAXWIDTH - 1)))
            {
                myrobot->room[x][y] = 255;
            }
            else
            {
                myrobot->room[x][y] = 0;
            }
        }
    }
}

void go_go_spiral(struct Robot *myrobot)
{
    //Main loop. As Long as we haven't finished the room and we can move:
    while (1)
    {
        set_field_visited(myrobot); //Set the field where we are as visited
        printf("Set the new obstacles NOW and press Enter: \n");
        //getchar(); //Press enter to continue
        get_free_directions_from_sensors((uint8_t*) myrobot->free_directions);
        save_obstacles(myrobot);
        check_if_finished(myrobot);
        get_best_directions(myrobot);
        print_room(myrobot);
        print_status(myrobot);
        if (myrobot->finished == true)
        {
            printf("We finished!!!\n");
            break;
        }
        printf("Press Enter to move: \n");
        getchar(); //Press enter to continue
        make_a_turn(myrobot); // turn in our preferred direction:
        move_forward(myrobot); //Move forward:
        printf("I moved forward!\n");
        print_room(myrobot);

    }
}

void check_if_finished(struct Robot *myrobot)
{
    uint8_t x, y;
    for (x = 0; x < MAXLENGTH; x++)
    {
        for (y = 0; y < MAXWIDTH; y++)
        {
            if (myrobot->room[x][y] == 0)
                return;
        }
    }
    //If we're out of the loop, all fields are greater than 0:
    myrobot->finished = true;
}

/*We want to save the obstacles in robot's array to be able to tell if we finished the room*/
void save_obstacles(struct Robot *myrobot)
{

    uint8_t i;
    int8_t offset[7][2] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 }, { 1, 0 },
                            { 0, 1 }, { -1, 0 } };

    for (i = 0; i < 4; i++)
    {
        if (!(myrobot->free_directions[i]))
        {
            myrobot->room[myrobot->x_pos + offset[myrobot->direction + i][0]][myrobot->y_pos
                    + offset[myrobot->direction + i][1]] = 255;
        }
    }
}

void get_best_directions(struct Robot *myrobot)
{
    /*We check how many times the neighboured fields were already visited and store that values in
     the myrobot->best_directions array*/

    switch (myrobot->direction)
    {
    case 0:
        myrobot->best_directions[0] =
                myrobot->room[myrobot->x_pos + 1][myrobot->y_pos];
        myrobot->best_directions[1] =
                myrobot->room[myrobot->x_pos][myrobot->y_pos + 1];
        myrobot->best_directions[2] =
                myrobot->room[myrobot->x_pos - 1][myrobot->y_pos];
        myrobot->best_directions[3] =
                myrobot->room[myrobot->x_pos][myrobot->y_pos - 1];
        break;
    case 1:
        myrobot->best_directions[0] =
                myrobot->room[myrobot->x_pos][myrobot->y_pos + 1];
        myrobot->best_directions[1] =
                myrobot->room[myrobot->x_pos - 1][myrobot->y_pos];
        myrobot->best_directions[2] =
                myrobot->room[myrobot->x_pos][myrobot->y_pos - 1];
        myrobot->best_directions[3] =
                myrobot->room[myrobot->x_pos + 1][myrobot->y_pos];
        break;
    case 2:
        myrobot->best_directions[0] =
                myrobot->room[myrobot->x_pos - 1][myrobot->y_pos];
        myrobot->best_directions[1] =
                myrobot->room[myrobot->x_pos][myrobot->y_pos - 1];
        myrobot->best_directions[2] =
                myrobot->room[myrobot->x_pos + 1][myrobot->y_pos];
        myrobot->best_directions[3] =
                myrobot->room[myrobot->x_pos][myrobot->y_pos + 1];
        break;
    case 3:
        myrobot->best_directions[0] =
                myrobot->room[myrobot->x_pos][myrobot->y_pos - 1];
        myrobot->best_directions[1] =
                myrobot->room[myrobot->x_pos + 1][myrobot->y_pos];
        myrobot->best_directions[2] =
                myrobot->room[myrobot->x_pos][myrobot->y_pos + 1];
        myrobot->best_directions[3] =
                myrobot->room[myrobot->x_pos - 1][myrobot->y_pos];
        break;
    default:
        break;
    }
    //Figure out the bet direction to turn to (find a field that was visited fewer times)
    /*We want to move to the field that was visited less times than the others, so our
     preferred direction is the minimum of our best_directions array*/
    myrobot->pref_dir = find_min_index(myrobot->best_directions, 4);
    /*As long as the preferred direction is blocked,
     we check the other directions, giving the blocked direction the 0xFF value*/
    while (myrobot->free_directions[myrobot->pref_dir] != true)
    {
        myrobot->best_directions[myrobot->pref_dir] = 255;
        myrobot->pref_dir = find_min_index(myrobot->best_directions, 4);
    }
}

void move_forward(struct Robot *myrobot)
{
    //check if the Arduino is busy:
    receive_okay(0);
    // Send a message to move forward:
    uart_send_msg("forward");
    //sleep 10 us
    sleep();
    //wait until the Arduino finished moving:
    receive_okay(0);

    // We check in which direction we're going and move through our room array
    switch (myrobot->direction)
    {
    case 0: // moving north
        myrobot->x_pos++;
        break;
    case 1: // moving east
        myrobot->y_pos++;
        break;
    case 2: // moving south
        myrobot->x_pos--;
        break;
    case 3: // moving west
        myrobot->y_pos--;
        break;
    default:
        break;
    }

}

void make_a_turn(struct Robot *myrobot)
{
    uint8_t i;
    if (myrobot->pref_dir == 3)
    {
        turn_left(myrobot);
    }
    else
        for (i = 0; i < myrobot->pref_dir; i++)
        {
            turn_right(myrobot);
        }
}
void turn_right(struct Robot *myrobot)
{
    if (myrobot->direction == 3)
        myrobot->direction = 0;
    else
        myrobot->direction++;

    //check if the Arduino is busy:
    receive_okay(0);
    // Send a message to turn right:
    uart_send_msg("turn_right");
    //sleep 10 us
    sleep();
    //wait until the Arduino finished turning right:
    receive_okay(0);

    printf("I turned right!\n");
}
void turn_left(struct Robot *myrobot)
{
    if (myrobot->direction == 0)
        myrobot->direction = 3;
    else
        myrobot->direction--;

    //check if the Arduino is busy:
    receive_okay(0);
    // Send a message to turn left:
    uart_send_msg("turn_left");
    //sleep 10 us
    sleep();
    //wait until the Arduino finished turning left:
    receive_okay(0);

    printf("I turned left!\n");
}

void set_field_visited(struct Robot *myrobot)
{
    // If the filed has the value 0, means we visited a new field
    if (myrobot->room[myrobot->x_pos][myrobot->y_pos] == 0)
    {
        myrobot->fields_visited++;
    }
    myrobot->room[myrobot->x_pos][myrobot->y_pos]++;
    myrobot->steps_done++;

    // Send coordinates to Hai Linh:
    char message[10];
    sprintf(message, "<%d,%d>", myrobot->x_pos, myrobot->y_pos);
    uart_send_msg(message);
}

void print_room(struct Robot *myrobot)
{
    uint8_t x;
    uint8_t y;
    for (x = 0; x < MAXLENGTH; x++)
    {
        for (y = 0; y < MAXWIDTH; y++)
        {
            printf("%x ", myrobot->room[y][x]);
        }
        printf("\n");
    }
    printf("Fields visited: %d\nSteps done: %d\n", myrobot->fields_visited,
           myrobot->steps_done);
}

void print_status(struct Robot *myrobot)
{
    printf("x_pos=%d\n", myrobot->x_pos);
    printf("y_pos=%d\n", myrobot->y_pos);
    printf("direction=%d\n", myrobot->direction);
    printf("forward free=%d\n", (uint8_t) myrobot->free_directions[0]);
    printf("right free=%d\n", (uint8_t) myrobot->free_directions[1]);
    printf("back free=%d\n", (uint8_t) myrobot->free_directions[2]);
    printf("left free=%d\n", (uint8_t) myrobot->free_directions[3]);
    printf("steps done=%d\n", (int) myrobot->steps_done);
    printf("fields visited=%d\n", (int) myrobot->fields_visited);
    printf("I want to turn to %d\n", myrobot->pref_dir);

}

uint8_t find_min_index(uint8_t arr[], uint8_t size)
{
    uint8_t min = 0;
    uint8_t i;
    for (i = 0; i < size; i++)
    {
        if (arr[i] < arr[min])
        {
            min = i;
        }
    }
    return min;
}
