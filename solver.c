#include "solver.h"
#include "API.h"
#include <stdio.h>

typedef struct {
    int x;
    int y;
} Cell;

// wall arrays (i, j)
int horizontalWalls [17][16];
int verticalWalls [16][17];
// distance array (x, y)
int manhattanDistances [16][16];

// starting position and direction
int x = 0;
int y = 0;
int heading = NORTH;

// intitialization state
int inited = 0;

void init() {
    // set all walls to empty and all manhattan distances to blank
    for(int i = 0; i < 16; i++) {
        for(int j = 0; j < 16; j++) {
            horizontalWalls[i][j] = 0;
            verticalWalls[i][j] = 0;
        }
    }

    // set maze walls to solid
    for(int i = 0; i < 16; i++) {
        verticalWalls[i][0] = 1;
        verticalWalls[i][16] = 1;

        horizontalWalls[0][i] = 1;
        horizontalWalls[16][i] = 1;
    }
}

void setWalls() {
    // set horizontal walls
    for(int x = 0; x < 16; x++){
        for(int y = 0; y < 17; y++){
            if(horizontalWalls[y][x] == 1){
                if(y == 16){
                    API_setWall(x, y - 1, 'n');
                } else{
                    API_setWall(x, y, 's');
                }
            }
        }
    }

    // set vertical walls
    for(int x = 0; x < 17; x++) {
        for(int y = 0; y < 16; y++){
            if(verticalWalls[y][x] == 1) {
                if(x == 16){
                    API_setWall(x - 1, y, 'e');
                } else{
                    API_setWall(x, y, 'w');
                }
            }
        }
    }
}

// returns __INT_MAX__ if out of bounds
int getManhattanDistance(int x, int y) {
    if(x < 0 || x > 15 || y < 0 || y > 15)
        return __INT_MAX__;
    return manhattanDistances[y][x];
}

void floodFill() {
    // implementing a queue
    Cell queue[256];
    int front = 0, rear = 0;

    for(int i = 0; i < 16; i++) {
        for(int j = 0; j < 16; j++) {
            manhattanDistances[i][j] = -1;
        }
    }

    manhattanDistances[7][7] = 0;
    manhattanDistances[7][8] = 0;
    manhattanDistances[8][7] = 0;
    manhattanDistances[8][8] = 0;
    
    queue[rear++] = (Cell) {7,7};
    queue[rear++] = (Cell) {7,8};
    queue[rear++] = (Cell) {8,7};
    queue[rear++] = (Cell) {8,8};

    while(front != rear) {
        Cell current = queue[front++];
        front %= 256;
        int curx = current.x;
        int cury = current.y;

        int nextDist = manhattanDistances[cury][curx] + 1;

        //up
        if(!horizontalWalls[cury + 1][curx]) {
            if(manhattanDistances[cury+1][curx] == -1) {
                manhattanDistances[cury+1][curx] = nextDist;
                queue[rear++] = (Cell) {curx, cury + 1};
                rear %= 256;
            }
        }

        // right
        if(!verticalWalls[cury][curx + 1]) {           
            if(manhattanDistances[cury][curx+1] == -1) {
                manhattanDistances[cury][curx+1] = nextDist;
                queue[rear++] = (Cell) {curx + 1, cury};
                rear %= 256;
            }
        }

        // down
        if(!horizontalWalls[cury][curx]) {  
            if(manhattanDistances[cury-1][curx] == -1) {
                manhattanDistances[cury-1][curx] = nextDist;
                queue[rear++] = (Cell) {curx, cury - 1};
                rear %= 256;
            }
        }

        // left
        if(!verticalWalls[cury][curx]) { 
            if(manhattanDistances[cury][curx-1] == -1) {
                manhattanDistances[cury][curx-1] = nextDist;
                queue[rear++] = (Cell) {curx - 1, cury};
                rear %= 256;
            }
        }
    }
    
    char distStr[3];
    int distInt;
    for(int curx = 0; curx < 16; curx++){
        for(int cury = 0; cury < 16; cury++){
            distInt = manhattanDistances[cury][curx];
            sprintf(distStr, "%d", distInt);
            API_setText(curx, cury, distStr);
        }
    }

    setWalls();
}


void turnRight() {
    switch(heading) {
        case NORTH:
            heading = EAST;
            break;
        case SOUTH:
            heading = WEST;
            break;
        case EAST:
            heading = SOUTH;
            break;
        case WEST:
            heading = NORTH;       
    }
}

void turnLeft() {
    switch(heading) {
        case NORTH:
            heading = WEST;
                break;
            case SOUTH:
                heading = EAST;
                break;
            case EAST:
                heading = NORTH;
                break;
            case WEST:
                heading = SOUTH;
    }
}

void goForward() {
    switch(heading) {
        case NORTH:
            y++;
            break;
        case SOUTH:
            y--;
            break;
        case EAST:
            x++;
            break;
        case WEST:
            x--;
    }
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
                verticalWalls[y][x] = 1;
            }
            if(API_wallRight()) {
                verticalWalls[y][x+1] = 1;
            }
            if(API_wallFront()) {
                horizontalWalls[y+1][x] = 1;
            }
            break;
        case SOUTH:
            if(API_wallLeft()) {
                verticalWalls[y][x+1] = 1;
            }
            if(API_wallRight()) {
                verticalWalls[y][x] = 1;
            }
            if(API_wallFront()) {
                horizontalWalls[y][x] = 1;
            }
            break;
        case EAST:
            if(API_wallLeft()) {
                horizontalWalls[y+1][x] = 1;
            }
            if(API_wallRight()) {
                horizontalWalls[y][x] = 1;
            }
            if(API_wallFront()) {
                verticalWalls[y][x + 1] = 1;
            }
            break;
        case WEST:
            if(API_wallLeft()) {
                horizontalWalls[y][x] = 1;
            }
            if(API_wallRight()) {
                horizontalWalls[y+1][x] = 1;
            }
            if(API_wallFront()) {
                verticalWalls[y][x] = 1;
            }
    }

    floodFill();
    
    int left = __INT_MAX__;
    int right = __INT_MAX__;
    int forward = __INT_MAX__;

    switch(heading){
        case NORTH:
            if(verticalWalls[y][x] == 0) {
                left = getManhattanDistance(x-1, y);
            }
            if(verticalWalls[y][x+1] == 0) {
                right = getManhattanDistance(x+1,y);
            }
            if(horizontalWalls[y+1][x] == 0) {
                forward = getManhattanDistance(x,y+1);
            }
            break;
        case SOUTH:
            if(verticalWalls[y][x+1] == 0) {
                left = getManhattanDistance(x+1,y);
            }
            if(verticalWalls[y][x] == 0) {
                right = getManhattanDistance(x-1,y);
            }
            if(horizontalWalls[y][x] == 0) {
                forward = getManhattanDistance(x,y-1);
            }
            break;
        case EAST:
            if(horizontalWalls[y+1][x] == 0) {
                left = getManhattanDistance(x,y+1);
            }
            if(horizontalWalls[y][x] == 0) {
                right = getManhattanDistance(x,y-1);
            }
            if(verticalWalls[y][x+1] == 0) {
                forward = getManhattanDistance(x+1,y);
            }
            break;
        case WEST:
            if(horizontalWalls[y][x] == 0) {
                left = getManhattanDistance(x,y-1);
            }
            if(horizontalWalls[y+1][x] == 0) {
                right = getManhattanDistance(x,y+1);
            }
            if(verticalWalls[y][x] == 0) {
                forward = getManhattanDistance(x-1,y);
            }
    }

    if((left == __INT_MAX__) && (forward == __INT_MAX__) && (right == __INT_MAX__)) {
        turnRight();
        return RIGHT;
    }

    if(forward <= left && forward <= right) {
        goForward();
        return FORWARD;
    }

    if(left < right && left < forward){
        turnLeft();
        return LEFT;
    }

    turnRight();
    return RIGHT;
}