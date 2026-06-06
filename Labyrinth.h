#ifndef LABYRINTH_H
#define LABYRINTH_H

#include "labyrinthAPI.h"

#define NB_TREASURES 24
#define MAX_EXTRA_STEPS 9

#define INSERT_LINE_LEFT   0
#define INSERT_LINE_RIGHT  1
#define INSERT_COL_TOP     2
#define INSERT_COL_BOTTOM  3

// ------------------------------------------------------- //

typedef struct {
    int north;
    int east;
    int south;
    int west;
    int treasure;
} t_tile;

typedef struct {
    int x;
    int y;
} t_player;

typedef struct {
    t_tile*  labyrinth; 
    t_tile   extra;
    t_player GoodGuy;
    t_player BadGuy;
    int sizeX;
    int sizeY;
    int me;
    int myTreasure;
    int oppTreasure;
    int lastType;
    int lastIndex;
} t_labyrinth;

// ------------------------------------------------------- //

/* Accès aux tuiles */
int     tile_index(const t_labyrinth* lab, int x, int y);
t_tile* tile_at(t_labyrinth* lab, int x, int y);

/* Règles du jeu */
t_tile rotate_tile(t_tile t, int nb);
void   apply_insertion(t_labyrinth* lab, int type, int index, int rotation);
int    are_connected(t_labyrinth* lab, int x1, int y1, int x2, int y2);
void   compute_reachable(t_labyrinth* lab, int sx, int sy, int* reachable, int* dist);
int    find_treasure(const t_labyrinth* lab, int treasure);
int    next_treasure(int player, int current);

/* Initialisation et mise à jour */
int  init_labyrinth(t_labyrinth* lab, int sizeX, int sizeY, char* labyData);
void update_labyrinth(t_labyrinth* lab, const char* move, const char* msg, int who);

/* Utilitaires */
void print_labyrinth_data(const t_labyrinth* lab);
void free_labyrinth(t_labyrinth* lab);

#endif