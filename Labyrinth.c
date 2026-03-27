#include "labyrinthAPI.h"
#include "Labyrinth.h"
#include <stdlib.h>
#include <stdio.h>

// ------------------------------------------------------- //

t_return_code PlayMove(char* move, char* msg) {
    int type, indice, rotation, x, y;

    if (scanf("%d %d %d %d %d", &type, &indice, &rotation, &x, &y) == 5) {
        sprintf(move, "%d %d %d %d %d", type, indice, rotation, x, y);
        return sendMove(move, msg);
    }
    
    return EXIT_FAILURE;
}

// ------------------------------------------------------- //

void init(char** labyrinthName, char** labyData, int* Playing, char** move, char** msg) {

    int sizeX, sizeY;

    *labyrinthName = malloc(50 * sizeof(char));

    connectToServer("localhost", 1234, "ZabZab");
    waitForLabyrinth("TRAINING DONTMOVE timeout=150", *labyrinthName, &sizeX, &sizeY);

    *labyData = malloc((sizeX * sizeY + 5) * 11 * sizeof(char));
    *Playing = !getLabyrinth(*labyData);
    *move = malloc(128);
    *msg = malloc(200);

    printLabyrinth();
    printf("%s",*labyData);
}

// ------------------------------------------------------- //

int init_labyrinth(t_labyrinth* labyrinth, int sizeX, int sizeY, char* labyData) {

    int Playing = getLabyrinth(labyData);
    int Offset = 0;
    labyrinth -> labyrinth = malloc(sizeof(t_tile)*sizeX*sizeY);

    for(int i = 0; i < sizeX*sizeY; i++){

        sscanf(labyData + Offset, "%d %d %d %d %d",
            &(labyrinth->labyrinth[i].north),
            &(labyrinth->labyrinth[i].east),
            &(labyrinth->labyrinth[i].south),
            &(labyrinth->labyrinth[i].west),
            &(labyrinth->labyrinth[i].treasure));
        
        if(labyrinth -> labyrinth[i].treasure > 9) Offset += 11;
        else Offset += 10;
    }

    sscanf(labyData, "%d %d %d %d %d",
        &(labyrinth -> extra.north),
        &(labyrinth -> extra.east),
        &(labyrinth -> extra.south),
        &(labyrinth -> extra.west),
        &(labyrinth -> extra.treasure));
    

    if(Playing) {
        labyrinth->GoodGuy.x=0;
        labyrinth->GoodGuy.y=0;
        labyrinth->BadGuy.x=sizeX-1;
        labyrinth->BadGuy.y=sizeY-1;
    } else {
        labyrinth->GoodGuy.x=sizeX-1;
        labyrinth->GoodGuy.y=sizeY-1;
        labyrinth->BadGuy.x=0;
        labyrinth->BadGuy.y=0;
    }

    return Playing;
}

// ------------------------------------------------------- //

int update_labyrinth(t_labyrinth* labyrinth, int sizeX, int sizeY, ) {
    getMove(char* move, char* msg);
    // lire entrée
    // update_tableau 
    // update_extra
    // update_position_joueur
}

// ------------------------------------------------------- //

int main() {

    char *labyrinthName, *labyData, *move, *msg; 
    int Playing;
    t_return_code ended = 0;

    init(&labyrinthName, &labyData, &Playing, &move, &msg);

    while(!ended) {

        if(Playing % 2) {
            ended = PlayMove(move, msg);
        } else {
            ended = getMove(move, msg);
            printf("\n\n");
        }

        printLabyrinth();
        printf("\n\n");
        printf("%s",labyData);
        printf("\n\n");
        Playing ++;
    }

    closeConnection();

    free(labyData);
    free(labyrinthName);
    free(move);
    free(msg);
}

// ------------------------------------------------------- //