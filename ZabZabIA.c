#include "ZabZabIA.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define BIG 1000000
#define TIME_LIMIT 4
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
    if (to != -1) score += 1.4 * W_MUR * wall_cost_bfs(lab, lab->BadGuy.x,  lab->BadGuy.y,  to);
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

// ------------------------------------------------------- //

static int alpha_beta(t_labyrinth* lab, int prof, int alpha, int beta, int nous) {
    int adv = 1 - lab->me;
    if (player_won(lab->me, lab->myTreasure)) return  BIG;
    if (player_won(adv,     lab->oppTreasure)) return -BIG;
    if (prof == 0 || clock() > fin_du_temps) return evaluate(lab);

    int types[256], indices[256];
    int c = list_insertions(lab, types, indices);
    int best = nous ? -BIG - 1 : BIG + 1;

    for (int i = 0; i < c; i++)
        for (int r = 0; r < 4; r++) {
            t_labyrinth cl;
            copy_board(&cl, lab);
            apply_insertion(&cl, types[i], indices[i], r);
            move_pawn(&cl, nous ? 0 : 1);
            int v = alpha_beta(&cl, prof - 1, alpha, beta, !nous);
            free(cl.labyrinth);

            if (nous) { if (v > best) best = v; if (best > alpha) alpha = best; }
            else      { if (v < best) best = v; if (best < beta)  beta  = best; }
            if (alpha >= beta) return best;
        }
    return best;
}

// ------------------------------------------------------- //

static void build_move(t_labyrinth* lab, int type, int index, int rot, char* out) {
    t_labyrinth cl;
    copy_board(&cl, lab);
    apply_insertion(&cl, type, index, rot);

    int n = cl.sizeX * cl.sizeY;
    int vu[n], dist[n];
    compute_reachable(&cl, cl.GoodGuy.x, cl.GoodGuy.y, vu, dist);
    int len = sprintf(out, "%d %d %d", type, index, rot);
    int pos = find_treasure(&cl, cl.myTreasure);

    if (pos == -1 || !vu[pos]) { /* Rien à prendre : on se rapproche */
        int dx = cl.GoodGuy.x, dy = cl.GoodGuy.y;
        if (pos != -1) {
            int tx = pos % cl.sizeX, ty = pos / cl.sizeX, bd = BIG;
            for (int i = 0; i < n; i++)
                if (vu[i]) {
                    int md = abs_int(i % cl.sizeX - tx) + abs_int(i / cl.sizeX - ty);
                    if (md < bd) { bd = md; dx = i % cl.sizeX; dy = i / cl.sizeX; }
                }
        }
        sprintf(out + len, " %d %d", dx, dy);
        free(cl.labyrinth);
        return;
    }

    int x = pos % cl.sizeX, y = pos / cl.sizeX; /* On prend le premier trésor */
    len += sprintf(out + len, " %d %d", x, y);
    int cible = next_treasure(cl.me, cl.myTreasure);

    int budget = MAX_EXTRA_STEPS; /* Puis on enchaîne sous 10 cases */
    while (!player_won(cl.me, cible)) {
        compute_reachable(&cl, x, y, vu, dist);
        int suiv = find_treasure(&cl, cible);
        if (suiv == -1 || !vu[suiv] || dist[suiv] > budget) break;
        budget -= dist[suiv];
        x = suiv % cl.sizeX; y = suiv / cl.sizeX;
        len += sprintf(out + len, " %d %d", x, y);
        cible = next_treasure(cl.me, cible);
    }
    free(cl.labyrinth);
}

// ------------------------------------------------------- //

void find_best_move(t_labyrinth* lab, char* moveText) {
    fin_du_temps = clock() + (clock_t)(TIME_LIMIT * CLOCKS_PER_SEC);

    int types[256], indices[256];
    int c = list_insertions(lab, types, indices);
    int bType = types[0], bIdx = indices[0], bRot = 0;

    for (int prof = 1; ; prof++) {
        int meilleur = -BIG - 1, t = bType, idx = bIdx, rot = bRot;
        for (int i = 0; i < c; i++)
            for (int r = 0; r < 4; r++) {
                t_labyrinth cl;
                copy_board(&cl, lab);
                apply_insertion(&cl, types[i], indices[i], r);
                move_pawn(&cl, 0);
                int v = alpha_beta(&cl, prof - 1, -BIG - 1, BIG + 1, 0);
                free(cl.labyrinth);
                if (v > meilleur) { meilleur = v; t = types[i]; idx = indices[i]; rot = r; }
            }

        if (clock() > fin_du_temps) break; /* Profondeur interrompue : on garde la précédente */
        bType = t; bIdx = idx; bRot = rot;
        if (meilleur >= BIG) break;
    }

    build_move(lab, bType, bIdx, bRot, moveText);
}