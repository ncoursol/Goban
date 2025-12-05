#ifndef MCTS_V1_H
#define MCTS_V1_H

#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>
#include <time.h>
#include "gomo.h"

#define MAX_THREADS 128
#define NUM_SIMULATIONS_V1 200000

#define MCTS_TOP_K_V1 32

typedef struct game_s game_t;

typedef struct node_v1_s
{
    atomic_int      value;
    atomic_int      visit_count;
    _Atomic float   uct;
    int             x;
    int             y;
    int             player;
    int             nb_childs;
    int             max_childs;
    int             valid_moves[361];
    pthread_mutex_t mutex;
    struct node_v1_s *parent;
    struct node_v1_s **childs;
} node_v1_t;

typedef struct load_balancer_v1_s
{
    atomic_int global_sim_counter;
    atomic_int thread_sim_counts[MAX_THREADS];
    atomic_llong thread_times_ns[MAX_THREADS];
    atomic_llong thread_phase_times_ns[MAX_THREADS][4];
    pthread_mutex_t balance_mutex;
    int active_threads;
    int total_simulations;
} load_balancer_v1_t;

typedef struct thread_data_v1_s
{
    unsigned int board_sim[19][19];
    int captured_black;
    int captured_white;
    int winning_state;
    int thread_id;
    unsigned int rand_seed;
    node_v1_t *node;
    load_balancer_v1_t *balancer;
} thread_data_v1_t;

typedef struct mcts_v1_s
{
    unsigned int board[19][19];
    unsigned int captured_black;
    unsigned int captured_white;
    unsigned int winning_state;
}   mcts_v1_t;

void run_mcts_v1(game_t *game, int *res_x, int *res_y, int num_simulations);

#endif
