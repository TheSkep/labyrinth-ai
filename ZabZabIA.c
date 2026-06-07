#include "ZabZabIA.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define BIG 1000000
#define TIME_LIMIT 8.0
#define W_TRESOR 10000
#define W_MUR 60

static clock_t fin_du_temps;

// ------------------------------------------------------- //

static int treasures_collected(int joueur, int cible) {
    return (joueur == 0) ? cible - 1 : NB_TREASURES - cible;
}

static int player_won(int joueur, int cible) {
    return (joueur == 0) ? (cible > NB_TREASURES) : (cible < 1);
}

static int abs_int(int v) {
    return v < 0 ? -v : v;
}

// ------------------------------------------------------- //

static void copy_board(t_labyrinth* dst, const t_labyrinth* src) {
    *dst = *src;
    int n = src->sizeX * src->sizeY;
    dst->labyrinth = malloc(sizeof(t_tile) * n);
    memcpy(dst->labyrinth, src->labyrinth, sizeof(t_tile) * n);
}

// ------------------------------------------------------- //

static int wall_cost_bfs(t_labyrinth* lab, int sx, int sy, int cible) {
    int n = lab->sizeX * lab->sizeY, cap = 4 * n + 4;
    int cost[n], vu[n], dq[cap];
    for (int i = 0; i < n; i++) { cost[i] = BIG; vu[i] = 0; }

    int head = 0, tail = 0, nb = 0;
    int debut = tile_index(lab, sx, sy);
    cost[debut] = 0;
    dq[tail] = debut; tail = (tail + 1) % cap; nb++;

    int dx[4] = { 1, -1, 0, 0 }, dy[4] = { 0, 0, 1, -1 };
    while (nb > 0) {
        int cur = dq[head]; head = (head + 1) % cap; nb--;
        if (vu[cur]) continue;
        vu[cur] = 1;
        int x = cur % lab->sizeX, y = cur / lab->sizeX;
        for (int d = 0; d < 4; d++) {
            int nx = x + dx[d], ny = y + dy[d];
            if (nx < 0 || nx >= lab->sizeX || ny < 0 || ny >= lab->sizeY) continue;
            int ni = tile_index(lab, nx, ny);
            int w  = are_connected(lab, x, y, nx, ny) ? 0 : 1;
            if (cost[cur] + w < cost[ni]) {
                cost[ni] = cost[cur] + w;
                if (w == 0) { head = (head - 1 + cap) % cap; dq[head] = ni; }
                else        { dq[tail] = ni; tail = (tail + 1) % cap; }
                nb++;
            }
        }
    }
    return cost[cible];
}

// ------------------------------------------------------- //

static int evaluate(t_labyrinth* lab) {
    int adv = 1 - lab->me;
    if (player_won(lab->me, lab->myTreasure)) return  BIG;
    if (player_won(adv, lab->oppTreasure)) return -BIG;

    int score = W_TRESOR * (treasures_collected(lab->me, lab->myTreasure) - treasures_collected(adv, lab->oppTreasure));
    int tp = find_treasure(lab, lab->myTreasure);
    int to = find_treasure(lab, lab->oppTreasure);
    if (tp != -1) score -= W_MUR * wall_cost_bfs(lab, lab->GoodGuy.x, lab->GoodGuy.y, tp);
    if (to != -1) score += W_MUR * wall_cost_bfs(lab, lab->BadGuy.x,  lab->BadGuy.y,  to);
    return score;
}

// ------------------------------------------------------- //

static void move_pawn(t_labyrinth* lab, int qui) {
    t_player* pion   = (qui == 0) ? &lab->GoodGuy : &lab->BadGuy;
    int       joueur = (qui == 0) ? lab->me : 1 - lab->me;
    int*      cible  = (qui == 0) ? &lab->myTreasure : &lab->oppTreasure;

    int n = lab->sizeX * lab->sizeY;
    int vu[n], dist[n];
    compute_reachable(lab, pion->x, pion->y, vu, dist);
    int pos = find_treasure(lab, *cible);

    if (pos != -1 && vu[pos]) { /* Trésor atteignable : on le prend */
        pion->x = pos % lab->sizeX;
        pion->y = pos / lab->sizeX;
        *cible = next_treasure(joueur, *cible);
        return;
    }
    if (pos != -1) { /* Sinon on s'en rapproche */
        int tx = pos % lab->sizeX, ty = pos / lab->sizeX, best = -1, bestd = BIG;
        for (int i = 0; i < n; i++)
            if (vu[i]) {
                int md = abs_int(i % lab->sizeX - tx) + abs_int(i / lab->sizeX - ty);
                if (md < bestd) { bestd = md; best = i; }
            }
        if (best != -1) { pion->x = best % lab->sizeX; pion->y = best / lab->sizeX; }
    }
}

// ------------------------------------------------------- //

static int list_insertions(const t_labyrinth* lab, int* types, int* indices) {
    int interditT = (lab->lastType >= 0) ? (lab->lastType ^ 1) : -1;
    int interditI = lab->lastIndex, c = 0;

    for (int t = INSERT_LINE_LEFT; t <= INSERT_LINE_RIGHT; t++)
        for (int i = 1; i < lab->sizeY; i += 2)
            if (!(t == interditT && i == interditI)) { types[c] = t; indices[c] = i; c++; }
    for (int t = INSERT_COL_TOP; t <= INSERT_COL_BOTTOM; t++)
        for (int i = 1; i < lab->sizeX; i += 2)
            if (!(t == interditT && i == interditI)) { types[c] = t; indices[c] = i; c++; }
    return c;
}

