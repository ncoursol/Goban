#ifndef MCTS_H
#define MCTS_H

#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>
#include "gomo.h"

#define MAX_THREADS 128
#define NUM_SIMULATIONS 500000
#define LOAD_BALANCE_CHECK_INTERVAL 1000

typedef struct game_s game_t;

typedef struct      node_s
{
    atomic_int      value;
    atomic_int      visit_count;
    _Atomic float   uct;
    int             x;
    int             y;
    int             color;
    int             nb_childs;
    pthread_mutex_t mutex;
    struct node_s   *parent;
    struct node_s   **childs;
}   node_t;

typedef struct load_balancer_s
{
    atomic_int global_sim_counter;
    atomic_int thread_sim_counts[MAX_THREADS];
    atomic_llong thread_times_ns[MAX_THREADS];
    pthread_mutex_t balance_mutex;
    int active_threads;
    int total_simulations;
} load_balancer_t;

typedef struct thread_data_s
{
    unsigned int board_sim[19][19];
    int captured_black;
    int captured_white;
    int winning_state;
    int thread_id;
    unsigned int rand_seed;
    node_t *node;
    load_balancer_t *balancer;
} thread_data_t;

void    run_mcts(game_t *game, int *res_x, int *res_y);

#endif