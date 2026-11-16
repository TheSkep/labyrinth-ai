#include "labyrinthAPI.h"
#include "Labyrinth.h"
#include "ZabZabIA.h"
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
        t_tile r = t;
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

        case INSERT_LINE_LEFT: /* Shift the row right */
            popped = *tile_at(lab, sx - 1, index);
            for (int x = sx - 1; x >= 1; x--)
                *tile_at(lab, x, index) = *tile_at(lab, x - 1, index);
            *tile_at(lab, 0, index) = inserted;

            for (int p = 0; p < 2; p++)
                if (players[p]->y == index) {
                    players[p]->x++;
                    if (players[p]->x == sx) players[p]->x = 0; /* Pawn pushed off: wraps around */
                }
            break;

        case INSERT_LINE_RIGHT: /* Shift the row left */
            popped = *tile_at(lab, 0, index);
            for (int x = 0; x <= sx - 2; x++)
                *tile_at(lab, x, index) = *tile_at(lab, x + 1, index);
            *tile_at(lab, sx - 1, index) = inserted;

            for (int p = 0; p < 2; p++)
                if (players[p]->y == index) {
                    players[p]->x--;
                    if (players[p]->x < 0) players[p]->x = sx - 1; /* Pawn pushed off: wraps around */
                }
            break;

        case INSERT_COL_TOP:    /* Shift the column down */
            popped = *tile_at(lab, index, sy - 1);
            for (int y = sy - 1; y >= 1; y--)
                *tile_at(lab, index, y) = *tile_at(lab, index, y - 1);
            *tile_at(lab, index, 0) = inserted;

            for (int p = 0; p < 2; p++)
                if (players[p]->x == index) {
                    players[p]->y++;
                    if (players[p]->y == sy) players[p]->y = 0; /* Pawn pushed off: wraps around */
                }
            break;

        case INSERT_COL_BOTTOM: /* Shift the column up */
            popped = *tile_at(lab, index, 0);
            for (int y = 0; y <= sy - 2; y++)
                *tile_at(lab, index, y) = *tile_at(lab, index, y + 1);
            *tile_at(lab, index, sy - 1) = inserted;

            for (int p = 0; p < 2; p++)
                if (players[p]->x == index) {
                    players[p]->y--;
                    if (players[p]->y < 0) players[p]->y = sy - 1; /* Pawn pushed off: wraps around */
                }
            break;

        default:
            return; /* Invalid move: nothing to do */
    }

    lab->extra     = popped; /* The pushed-out tile becomes the spare */
    lab->lastType  = type;
    lab->lastIndex = index; /* Remembered so the reverse push is banned next turn */
}

// ------------------------------------------------------- //

int are_connected(t_labyrinth* lab, int x1, int y1, int x2, int y2) {

    t_tile* a = tile_at(lab, x1, y1);
    t_tile* b = tile_at(lab, x2, y2);

    if (x2 == x1 + 1 && y2 == y1) return !a->east  && !b->west;
    if (x2 == x1 - 1 && y2 == y1) return !a->west  && !b->east;
    if (y2 == y1 + 1 && x2 == x1) return !a->south && !b->north;
    if (y2 == y1 - 1 && x2 == x1) return !a->north && !b->south;

    return 0;
}

// ------------------------------------------------------- //

void compute_reachable(t_labyrinth* lab, int sx, int sy, int* reachable, int* dist) {

    int n = lab->sizeX * lab->sizeY;
    for (int i = 0; i < n; i++) { reachable[i] = 0; dist[i] = -1; }

    int* queue = malloc(n * sizeof(int));
    int head = 0, tail = 0;

    int start = tile_index(lab, sx, sy);
    reachable[start] = 1;
    dist[start] = 0;
    queue[tail++] = start;

    int dx[4] = { 1, -1, 0, 0 };
    int dy[4] = { 0, 0, 1, -1 };

    while (head < tail) {
        int cur = queue[head++];
        int x = cur % lab->sizeX;
        int y = cur / lab->sizeX;

        for (int d = 0; d < 4; d++) {
            int nx = x + dx[d], ny = y + dy[d];
            if (nx < 0 || nx >= lab->sizeX || ny < 0 || ny >= lab->sizeY) continue;

            int ni = tile_index(lab, nx, ny);
            if (!reachable[ni] && are_connected(lab, x, y, nx, ny)) {
                reachable[ni] = 1;
                dist[ni] = dist[cur] + 1;
                queue[tail++] = ni;
            }
        }
    }

    free(queue);
}

// ------------------------------------------------------- //

int find_treasure(const t_labyrinth* lab, int treasure) {

    if (treasure <= 0) return -1;

    int n = lab->sizeX * lab->sizeY;
    for (int i = 0; i < n; i++)
        if (lab->labyrinth[i].treasure == treasure) return i;

    return -1;
}

// ------------------------------------------------------- //

int next_treasure(int player, int current) {
    return (player == 0) ? current + 1 : current - 1;
}

// ------------------------------------------------------- //

int init_labyrinth(t_labyrinth* lab, int sizeX, int sizeY, char* labyData) {

    int me = getLabyrinth(labyData); /* 0 if we start, 1 otherwise */

    lab->sizeX = sizeX;
    lab->sizeY = sizeY;
    lab->me    = me;
    lab->labyrinth = malloc(sizeof(t_tile) * sizeX * sizeY);

    /* Read the sizeX*sizeY board tiles */
    int offset = 0;
    for (int i = 0; i < sizeX * sizeY; i++) {
        int consumed;
        sscanf(labyData + offset, "%d %d %d %d %d%n",
               &lab->labyrinth[i].north,
               &lab->labyrinth[i].east,
               &lab->labyrinth[i].south,
               &lab->labyrinth[i].west,
               &lab->labyrinth[i].treasure,
               &consumed);
        offset += consumed;
    }

    /* Then the spare tile, right after */
    sscanf(labyData + offset, "%d %d %d %d %d",
           &lab->extra.north,
           &lab->extra.east,
           &lab->extra.south,
           &lab->extra.west,
           &lab->extra.treasure);

    if (me == 0) {
        lab->GoodGuy.x = 0;         lab->GoodGuy.y = 0;
        lab->BadGuy.x  = sizeX - 1; lab->BadGuy.y  = sizeY - 1;
        lab->myTreasure  = 1;
        lab->oppTreasure = NB_TREASURES;
    } else {
        lab->GoodGuy.x = sizeX - 1; lab->GoodGuy.y = sizeY - 1;
        lab->BadGuy.x  = 0;         lab->BadGuy.y  = 0;
        lab->myTreasure  = NB_TREASURES;
        lab->oppTreasure = 1;
    }

    lab->lastType  = -1;
    lab->lastIndex = -1;

    return me;
}

// ------------------------------------------------------- //

void update_labyrinth(t_labyrinth* lab, const char* move, const char* msg, int who) {

    /* Read every integer of the move */
    int vals[128];
    int n = 0, consumed;
    const char* p = move;

    while (n < 128 && sscanf(p, "%d%n", &vals[n], &consumed) == 1) {
        n++;
        p += consumed;
    }

    if (n < 3) return; /* Malformed move: nothing to do */

    int type     = vals[0];
    int index    = vals[1];
    int rotation = vals[2];

    /* Apply the insertion */
    apply_insertion(lab, type, index, rotation);

    if (n >= 5) {
        int dx = vals[n - 2];
        int dy = vals[n - 1];
        if (who == 0) { lab->GoodGuy.x = dx; lab->GoodGuy.y = dy; }
        else          { lab->BadGuy.x  = dx; lab->BadGuy.y  = dy; }
    }

    /* The server sends back the new spare tile and the next treasure */
    int en, ee, es, ew, et, nextItem;
    if (msg && sscanf(msg, "%d %d %d %d %d %d", &en, &ee, &es, &ew, &et, &nextItem) == 6) {

        lab->extra.north = en;  lab->extra.east  = ee;
        lab->extra.south = es;  lab->extra.west  = ew;
        lab->extra.treasure = et;

        if (who == 0) lab->myTreasure  = nextItem;
        else          lab->oppTreasure = nextItem;
    }
}

// ------------------------------------------------------- //

void print_labyrinth_data(const t_labyrinth* lab) {

    for (int y = 0; y < lab->sizeY; y++) {
        for (int x = 0; x < lab->sizeX; x++) {
            const t_tile* t = &lab->labyrinth[y * lab->sizeX + x];
            printf("%d%d%d%d%2d ", t->north, t->east, t->south, t->west, t->treasure);
        }
        printf("\n");
    }

    printf("extra : %d%d%d%d %d\n",
           lab->extra.north, lab->extra.east, lab->extra.south,
           lab->extra.west, lab->extra.treasure);
    printf("us (%d,%d) looking for %d | opp (%d,%d) looking for %d\n",
           lab->GoodGuy.x, lab->GoodGuy.y, lab->myTreasure,
           lab->BadGuy.x,  lab->BadGuy.y,  lab->oppTreasure);
}

// ------------------------------------------------------- //

void free_labyrinth(t_labyrinth* lab) {
    free(lab->labyrinth);
    lab->labyrinth = NULL;
}

// ------------------------------------------------------- //

static t_return_code PlayMove(t_labyrinth* lab, char* move, char* msg) {

    find_best_move(lab, move);

    return sendMove(move, msg);
}

// ------------------------------------------------------- //

int main(int argc, char** argv) {

    char labyrinthName[50];
    int  sizeX, sizeY;
    char move[256], msg[256];

    connectToServer("localhost", 1234, "ZabZabIA_DEPALLE");

    while (1) {

        waitForLabyrinth("", labyrinthName, &sizeX, &sizeY);

        /* The size can change between matches: (re)allocate here */
        char* labyData = malloc((sizeX * sizeY + 5) * 11 * sizeof(char));

        t_labyrinth lab;
        init_labyrinth(&lab, sizeX, sizeY, labyData);

        printLabyrinth();

        int turn = 0;
        t_return_code ended = NORMAL_MOVE;

        while (ended == NORMAL_MOVE) {

            if (turn % 2 == lab.me) { /* Our turn */
                ended = PlayMove(&lab, move, msg);
                if (ended != LOSING_MOVE)
                    update_labyrinth(&lab, move, msg, 0);
            } else { /* Opponent's turn */
                ended = getMove(move, msg);
                if (ended != LOSING_MOVE)
                    update_labyrinth(&lab, move, msg, 1);
            }

            if (ended == NORMAL_MOVE) printLabyrinth();

            turn++;
        }

        int our_move = ((turn - 1) % 2 == lab.me);
        int won = our_move ? (ended == WINNING_MOVE) : (ended == LOSING_MOVE);
        if (won) printf("ZabZabIA won!\n");
        else printf("ZabZabIA lost: %s\n", msg);

        /* End of the match: free and wait for the next opponent */
        free_labyrinth(&lab);
        free(labyData);
    }

    closeConnection(); /* Never reached, but polite */
    return 0;
}

// ------------------------------------------------------- //