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

void apply_insertion(t_labyrinth* lab, int type, int index, int rotation) {

    int sx = lab->sizeX, sy = lab->sizeY;
    t_tile inserted = rotate_tile(lab->extra, rotation);
    t_tile popped;

    t_player* players[2] = { &lab->GoodGuy, &lab->BadGuy };

    switch (type) {

        case INSERT_LINE_LEFT:  /* On glisse la ligne vers la droite */
            popped = *tile_at(lab, sx - 1, index);
            for (int x = sx - 1; x >= 1; x--)
                *tile_at(lab, x, index) = *tile_at(lab, x - 1, index);
            *tile_at(lab, 0, index) = inserted;

            for (int p = 0; p < 2; p++)
                if (players[p]->y == index) {
                    players[p]->x++;
                    if (players[p]->x == sx) players[p]->x = 0;  /* Pion expulsé */
                }
            break;

        case INSERT_LINE_RIGHT: /* On glisse la ligne vers la gauche */
            popped = *tile_at(lab, 0, index);
            for (int x = 0; x <= sx - 2; x++)
                *tile_at(lab, x, index) = *tile_at(lab, x + 1, index);
            *tile_at(lab, sx - 1, index) = inserted;

            for (int p = 0; p < 2; p++)
                if (players[p]->y == index) {
                    players[p]->x--;
                    if (players[p]->x < 0) players[p]->x = sx - 1;  /* Pion expulsé */
                }
            break;

        case INSERT_COL_TOP:    /* On glisse la colonne vers le bas */
            popped = *tile_at(lab, index, sy - 1);
            for (int y = sy - 1; y >= 1; y--)
                *tile_at(lab, index, y) = *tile_at(lab, index, y - 1);
            *tile_at(lab, index, 0) = inserted;

            for (int p = 0; p < 2; p++)
                if (players[p]->x == index) {
                    players[p]->y++;
                    if (players[p]->y == sy) players[p]->y = 0;  /* Pion expulsé */
                }
            break;

        case INSERT_COL_BOTTOM: /* On glisse la colonne vers le haut */
            popped = *tile_at(lab, index, 0);
            for (int y = 0; y <= sy - 2; y++)
                *tile_at(lab, index, y) = *tile_at(lab, index, y + 1);
            *tile_at(lab, index, sy - 1) = inserted;

            for (int p = 0; p < 2; p++)
                if (players[p]->x == index) {
                    players[p]->y--;
                    if (players[p]->y < 0) players[p]->y = sy - 1;  /* Pion expulsé */
                }
            break;

        default:
            return;   /* Coup invalide : rien à faire */
    }

    lab->extra     = popped;   /* La tuile expulsée devient la nouvelle extra */
    lab->lastType  = type;
    lab->lastIndex = index;
}

// ------------------------------------------------------- //