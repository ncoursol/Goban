#ifndef MCTS_H
#define MCTS_H

#include <pthread.h>
#include "gomo.h"

#define NUM_THREAD 1

typedef struct game_s game_t;

typedef struct      node_s
{
    int             value;
    int             visit_count;
    float           uct;
    int             x;
    int             y;
    int             color;
    int             nb_childs;
    pthread_mutex_t mutex;
    struct node_s   *parent;
    struct node_s   **childs;
}   node_t;

typedef struct thread_data_s
{
    unsigned int board_sim[19][19];
    int captured_black;
    int captured_white;
    int winning_state;
    int thread_id;
    int num_simulations;
    int *results;
    node_t *node;
} thread_data_t;

void    run_mcts(game_t *game);

#endif