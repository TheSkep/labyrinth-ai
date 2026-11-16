#include "ZabZabAI.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define BIG 1000000
#define TIME_LIMIT 4
#define W_TREASURE 10000   /* one treasure outweighs any wall count on the board */
#define W_WALL 60          /* per wall between a pawn and its treasure */

static clock_t deadline;

// ------------------------------------------------------- //

static int treasures_collected(int player, int target) {
    return (player == 0) ? target - 1 : NB_TREASURES - target;
}

static int player_won(int player, int target) {
    return (player == 0) ? (target > NB_TREASURES) : (target < 1);
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

static int wall_cost_bfs(t_labyrinth* lab, int sx, int sy, int target) {
    int n = lab->sizeX * lab->sizeY, cap = 4 * n + 4;
    int cost[n], seen[n], dq[cap];
    for (int i = 0; i < n; i++) { cost[i] = BIG; seen[i] = 0; }

    int head = 0, tail = 0, nb = 0;
    int start = tile_index(lab, sx, sy);
    cost[start] = 0;
    dq[tail] = start; tail = (tail + 1) % cap; nb++;

    int dx[4] = { 1, -1, 0, 0 }, dy[4] = { 0, 0, 1, -1 };
    while (nb > 0) {
        int cur = dq[head]; head = (head + 1) % cap; nb--;
        if (seen[cur]) continue;
        seen[cur] = 1;
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
    return cost[target];
}

// ------------------------------------------------------- //

static int evaluate(t_labyrinth* lab) {
    int opp = 1 - lab->me;
    if (player_won(lab->me, lab->myTreasure)) return  BIG;
    if (player_won(opp, lab->oppTreasure)) return -BIG;

    int score = W_TREASURE * (treasures_collected(lab->me, lab->myTreasure) - treasures_collected(opp, lab->oppTreasure));
    int my_pos = find_treasure(lab, lab->myTreasure);
    int opp_pos = find_treasure(lab, lab->oppTreasure);
    if (my_pos != -1) score -= W_WALL * wall_cost_bfs(lab, lab->GoodGuy.x, lab->GoodGuy.y, my_pos);
    /* The opponent's walls count 1.4x ours: blocking is worth a bit more than advancing */
    if (opp_pos != -1) score += 1.4 * W_WALL * wall_cost_bfs(lab, lab->BadGuy.x,  lab->BadGuy.y,  opp_pos);
    return score;
}

// ------------------------------------------------------- //

static void move_pawn(t_labyrinth* lab, int who) {
    t_player* pawn   = (who == 0) ? &lab->GoodGuy : &lab->BadGuy;
    int       player = (who == 0) ? lab->me : 1 - lab->me;
    int*      target = (who == 0) ? &lab->myTreasure : &lab->oppTreasure;

    int n = lab->sizeX * lab->sizeY;
    int seen[n], dist[n];
    compute_reachable(lab, pawn->x, pawn->y, seen, dist);
    int pos = find_treasure(lab, *target);

    if (pos != -1 && seen[pos]) { /* Treasure reachable: take it */
        pawn->x = pos % lab->sizeX;
        pawn->y = pos / lab->sizeX;
        *target = next_treasure(player, *target);
        return;
    }
    if (pos != -1) { /* Otherwise get closer to it */
        int tx = pos % lab->sizeX, ty = pos / lab->sizeX, best = -1, bestd = BIG;
        for (int i = 0; i < n; i++)
            if (seen[i]) {
                int md = abs_int(i % lab->sizeX - tx) + abs_int(i / lab->sizeX - ty);
                if (md < bestd) { bestd = md; best = i; }
            }
        if (best != -1) { pawn->x = best % lab->sizeX; pawn->y = best / lab->sizeX; }
    }
}

// ------------------------------------------------------- //

static int list_insertions(const t_labyrinth* lab, int* types, int* indices) {
    int banned_type = (lab->lastType >= 0) ? (lab->lastType ^ 1) : -1;
    int banned_index = lab->lastIndex, c = 0;

    for (int t = INSERT_LINE_LEFT; t <= INSERT_LINE_RIGHT; t++)
        for (int i = 1; i < lab->sizeY; i += 2)
            if (!(t == banned_type && i == banned_index)) { types[c] = t; indices[c] = i; c++; }
    for (int t = INSERT_COL_TOP; t <= INSERT_COL_BOTTOM; t++)
        for (int i = 1; i < lab->sizeX; i += 2)
            if (!(t == banned_type && i == banned_index)) { types[c] = t; indices[c] = i; c++; }
    return c;
}

// ------------------------------------------------------- //

static int alpha_beta(t_labyrinth* lab, int depth, int alpha, int beta, int maximizing) {
    int opp = 1 - lab->me;
    if (player_won(lab->me, lab->myTreasure)) return  BIG;
    if (player_won(opp,     lab->oppTreasure)) return -BIG;
    if (depth == 0 || clock() > deadline) return evaluate(lab);

    int types[256], indices[256];
    int c = list_insertions(lab, types, indices);
    int best = maximizing ? -BIG - 1 : BIG + 1;

    for (int i = 0; i < c; i++)
        for (int r = 0; r < 4; r++) {
            t_labyrinth cl;
            copy_board(&cl, lab);
            apply_insertion(&cl, types[i], indices[i], r);
            move_pawn(&cl, maximizing ? 0 : 1);
            int v = alpha_beta(&cl, depth - 1, alpha, beta, !maximizing);
            free(cl.labyrinth);

            if (maximizing) { if (v > best) best = v; if (best > alpha) alpha = best; }
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
    int seen[n], dist[n];
    compute_reachable(&cl, cl.GoodGuy.x, cl.GoodGuy.y, seen, dist);
    int len = sprintf(out, "%d %d %d", type, index, rot);
    int pos = find_treasure(&cl, cl.myTreasure);

    if (pos == -1 || !seen[pos]) { /* Nothing to take: get closer */
        int dx = cl.GoodGuy.x, dy = cl.GoodGuy.y;
        if (pos != -1) {
            int tx = pos % cl.sizeX, ty = pos / cl.sizeX, bd = BIG;
            for (int i = 0; i < n; i++)
                if (seen[i]) {
                    int md = abs_int(i % cl.sizeX - tx) + abs_int(i / cl.sizeX - ty);
                    if (md < bd) { bd = md; dx = i % cl.sizeX; dy = i / cl.sizeX; }
                }
        }
        sprintf(out + len, " %d %d", dx, dy);
        free(cl.labyrinth);
        return;
    }

    int x = pos % cl.sizeX, y = pos / cl.sizeX; /* Take the first treasure */
    len += sprintf(out + len, " %d %d", x, y);
    int target = next_treasure(cl.me, cl.myTreasure);

    int budget = MAX_EXTRA_STEPS; /* Then chain the next ones within MAX_EXTRA_STEPS */
    while (!player_won(cl.me, target)) {
        compute_reachable(&cl, x, y, seen, dist);
        int next = find_treasure(&cl, target);
        if (next == -1 || !seen[next] || dist[next] > budget) break;
        budget -= dist[next];
        x = next % cl.sizeX; y = next / cl.sizeX;
        len += sprintf(out + len, " %d %d", x, y);
        target = next_treasure(cl.me, target);
    }

    if (!player_won(cl.me, target)) { /* Get closer to the next treasure if it is out of reach */
        compute_reachable(&cl, x, y, seen, dist);
        int next = find_treasure(&cl, target);
        if (next != -1) {
            int tx = next % cl.sizeX, ty = next / cl.sizeX;
            int bx = x, by = y, bd = abs_int(x - tx) + abs_int(y - ty);
            for (int i = 0; i < n; i++)
                if (seen[i] && dist[i] <= budget) {
                    int md = abs_int(i % cl.sizeX - tx) + abs_int(i / cl.sizeX - ty);
                    if (md < bd) { bd = md; bx = i % cl.sizeX; by = i / cl.sizeX; }
                }
            if (bx != x || by != y) sprintf(out + len, " %d %d", bx, by);
        }
    }
    free(cl.labyrinth);
}

// ------------------------------------------------------- //

void find_best_move(t_labyrinth* lab, char* moveText) {
    deadline = clock() + (clock_t)(TIME_LIMIT * CLOCKS_PER_SEC);

    int types[256], indices[256];
    int c = list_insertions(lab, types, indices);
    int bType = types[0], bIdx = indices[0], bRot = 0;

    for (int depth = 1; ; depth++) {
        /* Search the previous depth's best move first */
        for (int i = 0; i < c; i++)
            if (types[i] == bType && indices[i] == bIdx) {
                int t0 = types[0]; types[0] = types[i]; types[i] = t0;
                int i0 = indices[0]; indices[0] = indices[i]; indices[i] = i0;
                break;
            }

        int best_value = -BIG - 1, t = bType, idx = bIdx, rot = bRot;
        for (int i = 0; i < c; i++)
            for (int r = 0; r < 4; r++) {
                t_labyrinth cl;
                copy_board(&cl, lab);
                apply_insertion(&cl, types[i], indices[i], r);
                move_pawn(&cl, 0);
                int v = alpha_beta(&cl, depth - 1, best_value, BIG + 1, 0);
                free(cl.labyrinth);
                if (v > best_value) { best_value = v; t = types[i]; idx = indices[i]; rot = r; }
            }

        if (clock() > deadline) break; /* Depth cut short: keep the previous one */
        bType = t; bIdx = idx; bRot = rot;
        if (best_value >= BIG) break;
    }

    build_move(lab, bType, bIdx, bRot, moveText);
}
