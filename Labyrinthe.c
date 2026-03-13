#include "labyrinthAPI.h"
#include <stdlib.h>

t_return_code PlayMove(char* move, char* msg) {
    int type, indice, rotation, x, y;

    if (scanf("%d %d %d %d %d", &type, &indice, &rotation, &x, &y) == 5) {
        sprintf(move, "%d %d %d %d %d", type, indice, rotation, x, y);
        return sendMove(move, msg);
    }
    
    return EXIT_FAILURE;
}

int main() {

    int sizeX;
    int sizeY;

    char* labyrinthName = malloc(50 * sizeof(char));

    connectToServer("localhost", 1234, "Zib");
    waitForLabyrinth("TRAINING DONTMOVE timeout=150", labyrinthName, &sizeX, &sizeY);

    char* labyData = malloc((sizeX * sizeY + 5) * 11 * sizeof(char));
    int DoitJouer = !getLabyrinth(labyData);
    char* move = malloc(128);
    char* msg = malloc(200);

    t_return_code fin = 0;

    while(!fin) {
        
        if(DoitJouer % 2) {
            fin = PlayMove(move, msg);
        } else {
            fin = getMove(move, msg);
            printf("\n\n");
        }

        printLabyrinth();
        DoitJouer ++;
    }

    closeConnection();

    free(labyData);
    free(labyrinthName);
    free(move);
    free(msg);
}