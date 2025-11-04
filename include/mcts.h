#ifndef MCTS_H
#define MCTS_H

#include <unistd.h>
#include <time.h>
#include "gomo.h"

#define NUM_SIMULATIONS 10000

typedef struct game_s game_t;

typedef struct      node_s
{
    int             value;
    int             visit_count;
    float           uct;
    int             x;
    int             y;
    int             player; // 0 for player 1, 1 for player 2
    int             nb_childs;
    int             max_childs;
    int             valid_moves[361];
    struct node_s   *parent;
    struct node_s   **childs;
}   node_t;

typedef struct      mcts_s
{
    unsigned int board[19][19];
    unsigned int captured_black;
    unsigned int captured_white;
    unsigned int winning_state;
}   mcts_t;

void    run_mcts(game_t *game, int *res_x, int *res_y);
int     select_weighted_move(unsigned int board[19][19], int *valid_moves, int max_size, int player);
float   *weightmap(unsigned int board[19][19], int *valid_moves, int max_size, int player);

#endif