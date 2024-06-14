#ifndef NAVIGATE_H
#define NAVIGATE_H

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
// Set the max room size, for now don't change.
#define MAXLENGTH 5
#define MAXWIDTH 5
#define NUM_FIELDS (MAXLENGTH-2)*(MAXWIDTH-2)

struct Robot
{
    uint8_t x_pos;
    uint8_t y_pos;
    /*Which direction the robot is looking at (When standing in the left down corner,
     the wall is on the left, forward direction is north).
    0 - north
    1 - east
    2 - south
    3 - west*/
    uint8_t direction;
    /*Information about next free fields in all 4 directions
    (Looking in the direction where the robot is looking). true = field is free, false - there's an obstacle
    free_directions[0] - forward
    free_directions[1] - to the right
    free_directions[2] - back
    free_directions[3] - to the left   */
    bool free_directions[4];
    uint8_t best_directions[4];
    uint8_t pref_dir; //The direction we want to turn to.
    // create a room array:
    uint8_t room[MAXLENGTH][MAXWIDTH];
    int steps_done;
    int fields_visited;
    bool cant_move;
    bool finished;
};

void robot_init(struct Robot *myrobot);
void check_if_finished(struct Robot *myrobot);
void save_obstacles(struct Robot *myrobot);
void get_best_directions(struct Robot *myrobot);
void move_forward(struct Robot *myrobot);
void make_a_turn(struct Robot *myrobot);
void turn_right(struct Robot *myrobot);
void turn_left(struct Robot *myrobot);
void set_field_visited(struct Robot *myrobot);
void print_room(struct Robot *myrobot);
void go_go_spiral(struct Robot *myrobot);
void print_status(struct Robot *myrobot);
uint8_t find_min_index(uint8_t arr[], uint8_t size);

#endif /*NAVIGATE_H*/
