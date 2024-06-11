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
    for (int x = 0; x < MAXLENGTH; x++)
    {
        for (int y = 0; y < MAXWIDTH; y++)
        {
            if ((x == 0) || (x == (MAXLENGTH - 1)) || (y == 0) || (y == (MAXWIDTH - 1)))
            {
                myrobot->room[x][y] = 255;
            }
            else
            {
                myrobot->room[x][y] = 0;
            }
        }
    }
    // We assume that we always begin at 0,0 coordinates, and this field is of course free
    set_field_visited(myrobot);
    get_free_directions_from_sensors(myrobot->free_directions);
    get_best_directions(myrobot);
}

void get_best_directions(struct Robot *myrobot)
{
    /*We check how many times the neighboured fields were already visired and store that values in
    the myrobot->best_directions array*/
    switch (myrobot->direction)
    {
    case 0:
        myrobot->best_directions[0] = myrobot->room[myrobot->x_pos + 1][myrobot->y_pos];
        myrobot->best_directions[1] = myrobot->room[myrobot->x_pos][myrobot->y_pos + 1];
        myrobot->best_directions[2] = myrobot->room[myrobot->x_pos - 1][myrobot->y_pos];
        myrobot->best_directions[3] = myrobot->room[myrobot->x_pos][myrobot->y_pos - 1];
        break;
    case 1:
        myrobot->best_directions[0] = myrobot->room[myrobot->x_pos][myrobot->y_pos + 1];
        myrobot->best_directions[1] = myrobot->room[myrobot->x_pos - 1][myrobot->y_pos];
        myrobot->best_directions[2] = myrobot->room[myrobot->x_pos][myrobot->y_pos - 1];
        myrobot->best_directions[3] = myrobot->room[myrobot->x_pos + 1][myrobot->y_pos];
        break;
    case 2:
        myrobot->best_directions[0] = myrobot->room[myrobot->x_pos - 1][myrobot->y_pos];
        myrobot->best_directions[1] = myrobot->room[myrobot->x_pos][myrobot->y_pos - 1];
        myrobot->best_directions[2] = myrobot->room[myrobot->x_pos + 1][myrobot->y_pos];
        myrobot->best_directions[3] = myrobot->room[myrobot->x_pos][myrobot->y_pos + 1];
        break;
    case 3:
        myrobot->best_directions[0] = myrobot->room[myrobot->x_pos][myrobot->y_pos - 1];
        myrobot->best_directions[1] = myrobot->room[myrobot->x_pos + 1][myrobot->y_pos];
        myrobot->best_directions[2] = myrobot->room[myrobot->x_pos][myrobot->y_pos + 1];
        myrobot->best_directions[3] = myrobot->room[myrobot->x_pos - 1][myrobot->y_pos];
        break;
    default:
        break;
    }
}

void move_forward(struct Robot *myrobot)
{
    char scan;
    scanf("%c", &scan);
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
    // Send a message to move forward:
    uart_send_msg(/*Michael/Peter, MOVE_FORWARD*/);

    // wait for the answer that we've done moving:

    // while(!uart_receive_msg()) {

    // }

    // After we moved, we want to set the field as visited
    set_field_visited(myrobot);
    printf("I moved forward!\n");
    print_room(myrobot);
}
void turn_right(struct Robot *myrobot)
{
    if (myrobot->direction == 3)
        myrobot->direction = 0;
    else
        myrobot->direction++;

    // Send a message to turn right:
    uart_send_msg(/*Michael/Peter, TURN_RIGHT*/);

    // wait for the answer that we've done turning:
    //  while(!uart_receive_msg()) {
    //  }

    printf("I turned right!\n");
}
void turn_left(struct Robot *myrobot)
{
    if (myrobot->direction == 0)
        myrobot->direction = 3;
    else
        myrobot->direction--;

    // Send a message to turn left:
    uart_send_msg(/*Michael/Peter, TURN_LEFT*/);

    // wait for the answer that we've done turning:
    //  while(!uart_receive_msg()) {
    //  }
    printf("I turned left!\n");
}
void go_go_spiral(struct Robot *myrobot)
{
    /*We want to move to the field that was visited less times than the others, so our
    preferred direction is the minimum of our best_directions array*/
    uint8_t pref_dir = find_min_index(myrobot->best_directions, 4);

    while ((myrobot->cant_move == false) && (myrobot->finished == false))
    {
        /*As long as the preferred direction is blocked,
        we check the other directions, giving the blocked direction the 0xFF value*/
        while (myrobot->free_directions[pref_dir] != true)
        {
            myrobot->best_directions[pref_dir] = 255;
            pref_dir = find_min_index(myrobot->best_directions, 4);
        }
        // turn in our preferred direction:
        if (pref_dir == 3)
        {
            turn_left(myrobot);
        }
        else
            for (int i = 0; i < pref_dir; i++)
            {
                turn_right(myrobot);
            }
        move_forward(myrobot);
        get_free_directions_from_sensors(myrobot->free_directions);
        get_best_directions(myrobot);
        pref_dir = find_min_index(myrobot->best_directions, 4);
    }
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
    uart_send_msg(/*HaiLinh, myrobot->x_pos,myrobot->y_pos*/);
}

void print_room(struct Robot *myrobot)
{
    for (int x = 0; x < MAXLENGTH; x++)
    {
        for (int y = 0; y < MAXWIDTH; y++)
        {
            printf("%x ", myrobot->room[y][x]);
        }
        printf("\n");
    }
    printf("Fields visited: %d\nSteps done: %d\n", myrobot->fields_visited, myrobot->steps_done);
}

uint8_t find_min_index(uint8_t arr[], uint8_t size)
{
    uint8_t min = 0;

    for (uint8_t i = 0; i < size; i++)
    {
        if (arr[i] < arr[min])
        {
            min = i;
        }
    }
    return min;
}