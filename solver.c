#include "solver.h"
#include "API.h"
#include <stdio.h>

typedef struct {
    int row;
    int col;
} Cell;

// state arrays
int horizontalWalls [17][16];
int verticalWalls [16][17];
int manhattanDistances [16][16];

// starting position and direction
int x;
int y;
int heading;

// intitialization state
int inited = 0;

void init() {
    // set all walls to empty and all manhattan distances to blank
    for(int i = 0; i < 17; i++){
        for(int j = 0; j < 16; j++){
            horizontalWalls[i][j] = 0;
        }
    }
    for(int i = 0; i < 16; i++){
        for(int j = 0; j < 17; j++){
            verticalWalls[i][j] = 0;
        }
    }
    for(int i = 0; i < 16; i++) {
        for(int j = 0; j < 16; j++) {
            manhattanDistances[i][j] = -1;
        }
    }

    // set maze walls to solid
    for(int i = 0; i < 16; i++) {
        verticalWalls[i][0] = 1;
        verticalWalls[i][16] = 1;

        horizontalWalls[16][i] = 1;
        horizontalWalls[0][i] = 1;
    }

    // init goals to zero
    manhattanDistances[7][7] = 0;
    manhattanDistances[7][8] = 0;
    manhattanDistances[8][7] = 0;
    manhattanDistances[8][8] = 0;

    // init vars
    x = 0;
    y = 0;
    heading = NORTH;
}

void setWalls(){
    // set vertical walls
    for(int i = 0; i < 16; i++){
        for(int j = 0; j < 17; j++){
            if(verticalWalls[i][j] == 1){
                if(j == 16){
                    API_setWall(i, j - 1, 'n');
                } else{
                    API_setWall(i, j, 's');
                }
            }
        }
    }

    // set horizontal walls
    for(int i = 0; i < 17; i++) {
        for(int j = 0; j < 16; j++){
            if(horizontalWalls[i][j] == 1) {
                if(i == 16){
                    API_setWall(i - 1, j, 'e');
                } else{
                    API_setWall(i, j, 'w');
                }
            }
        }
    }
}

// returns __INT_MAX__ if out of bounds
int getManhattanDistance(int x, int y) {
    if(x < 0 || x > 15 || y < 0 || y > 15)
        return __INT_MAX__;
    return manhattanDistances[x][y];
}

void floodFill() {
    // implementing a queue
    Cell queue[256];
    int front = 0, rear = 0;

    queue[rear++] = (Cell) {7,7};
    queue[rear++] = (Cell) {7,8};
    queue[rear++] = (Cell) {8,7};
    queue[rear++] = (Cell) {8,8};

    while(front != rear) {
        Cell current = queue[front++];
        front %= 256;
        int curx = current.row;
        int cury = current.col;

        int nextDist = manhattanDistances[curx][cury] + 1;

        //up
        if(!horizontalWalls[curx][cury + 1]) {
            if(manhattanDistances[curx][cury + 1] == -1) {
                manhattanDistances[curx][cury + 1] = nextDist;
                queue[rear++] = (Cell) {curx, cury + 1};
                rear %= 256;
            }
        }

        // right
        if(!verticalWalls[curx + 1][cury]) {           
            if(manhattanDistances[curx + 1][cury] == -1) {
                manhattanDistances[curx + 1][cury] = nextDist;
                queue[rear++] = (Cell) {curx + 1, cury};
                rear %= 256;
            }
        }

        // down
        if(!horizontalWalls[curx][cury]) {  
            if(manhattanDistances[curx][cury - 1] == -1) {
                manhattanDistances[curx][cury - 1] = nextDist;
                queue[rear++] = (Cell) {curx, cury - 1};
                rear %= 256;
            }
        }

        // left
        if(!verticalWalls[curx][cury]) { 
            if(manhattanDistances[curx - 1][cury] == -1) {
                manhattanDistances[curx - 1][cury] = nextDist;
                queue[rear++] = (Cell) {curx - 1, cury};
                rear %= 256;
            }
        }
    }
    
    for(int curx = 0; curx < 16; curx++){
        for(int cury = 0; cury < 16; cury++){
            char distStr[3];
            int distInt = manhattanDistances[curx][cury];
            sprintf(distStr, "%d", distInt);
            API_setText(curx, cury, distStr);
        }
    }

    setWalls();
}

Action solver() {
    // debug buffer
    char buf[20];

    if(!inited){
        init();
        inited = 1;
        floodFill();
    }

    // scan surrounded walls
    switch(heading) {
        case NORTH:
            if(API_wallLeft()) {
                verticalWalls[x][y] = 1;
            }
            if(API_wallRight()) {
                verticalWalls[x + 1][y] = 1;
            }
            if(API_wallFront()) {
                horizontalWalls[x][y + 1] = 1;
            }
            break;
        case SOUTH:
            if(API_wallLeft()) {
                verticalWalls[x + 1][y] = 1;
            }
            if(API_wallRight()) {
                verticalWalls[x][y] = 1;
            }
            if(API_wallFront()) {
                horizontalWalls[x][y] = 1;
            }
            break;
        case EAST:
            if(API_wallLeft()) {
                horizontalWalls[x][y + 1] = 1;
            }
            if(API_wallRight()) {
                horizontalWalls[x][y] = 1;
            }
            if(API_wallFront()) {
                verticalWalls[x + 1][y] = 1;
            }
            break;
        case WEST:
            if(API_wallLeft()) {
                horizontalWalls[x][y] = 1;
            }
            if(API_wallRight()) {
                horizontalWalls[x][y + 1] = 1;
            }
            if(API_wallFront()) {
                verticalWalls[x][y] = 1;
            }
    }

    floodFill();
    
    int left = __INT_MAX__;
    int right = __INT_MAX__;
    int forward = __INT_MAX__;

    switch(heading){
        case NORTH:
            if(verticalWalls[x][y] == 0) {
                left = getManhattanDistance(x-1, y);
            }
            if(verticalWalls[x + 1][y] == 0) {
                right = getManhattanDistance(x+1,y);
            }
            if(horizontalWalls[x][y + 1] == 0) {
                forward = getManhattanDistance(x,y+1);
            }
            break;
        case SOUTH:
            if(verticalWalls[x + 1][y] == 0) {
                left = getManhattanDistance(x+1,y);
            }
            if(verticalWalls[x][y] == 0) {
                right = getManhattanDistance(x-1,y);
            }
            if(horizontalWalls[x][y] == 0) {
                forward = getManhattanDistance(x,y-1);
            }
            break;
        case EAST:
            if(horizontalWalls[x][y + 1] == 0) {
                left = getManhattanDistance(x,y+1);
            }
            if(horizontalWalls[x][y] == 0) {
                right = getManhattanDistance(x,y-1);
            }
            if(verticalWalls[x + 1][y] == 0) {
                forward = getManhattanDistance(x+1,y);
            }
            break;
        case WEST:
            if(horizontalWalls[x][y] == 0) {
                left = getManhattanDistance(x,y-1);
            }
            if(horizontalWalls[x][y + 1] == 0) {
                right = getManhattanDistance(x,y+1);
            }
            if(verticalWalls[x][y] == 0) {
                forward = getManhattanDistance(x-1,y);
            }
    }

    sprintf(buf, "%d", left);
    debug_log(buf);
    sprintf(buf, "%d", forward);
    debug_log(buf);
    sprintf(buf, "%d", right);
    debug_log(buf);
    debug_log("");

    if(left <= right && left <= forward){
        return LEFT;
    }
    else if(right <= left && right <= forward){
        return RIGHT;
    }
    return FORWARD;
}

// This is an example of a simple left wall following algorithm.
Action leftWallFollower() {
    if(API_wallFront()) {
        if(API_wallLeft()){
            return RIGHT;
        }
        return LEFT;
    }
    return FORWARD;
}