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
    t_tile* labyrinth;
    t_tile extra;
    t_player GoodGuy;
    t_player BadGuy;
} t_labyrinth;