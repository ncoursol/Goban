#ifndef MCTS_H
#define MCTS_H

#include <pthread.h>
#include "game.h"

#define NUM_THREAD CPU_cores

typedef struct      node_s
{
    int             value;
    int             visit_count;
    float           uct;
    int             x;
    int             y;
    int             color;
    pthread_mutex_t mutex;
    struct node_s   *parent;
    struct node_s   **childs;
}   node_t;

typedef struct thread_data_s
{
    int **board_sim;
    int thread_id;
    int num_simulations;
    int *results;
    node_t *node;
} thread_data_t;

void init_mcts(void);

#endif