#include "labyrinthAPI.h"
#include "Labyrinth.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// ------------------------------------------------------- //

int tile_index(const t_labyrinth* lab, int x, int y) {
    return y * lab->sizeX + x;
}

t_tile* tile_at(t_labyrinth* lab, int x, int y) {
    return &lab->labyrinth[tile_index(lab, x, y)];
}

// ------------------------------------------------------- //

t_tile rotate_tile(t_tile t, int nb) {

    nb = ((nb % 4) + 4) % 4;

    for (int k = 0; k < nb; k++) {
        t_tile r = t;          /* On recopie aussi le trésor */
        r.north = t.west;
        r.east  = t.north;
        r.south = t.east;
        r.west  = t.south;
        t = r;
    }

    return t;
}

// ------------------------------------------------------- //