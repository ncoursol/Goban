#ifndef MCTS_H
#define MCTS_H

#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>
#include <time.h>
#include "gomo.h"

#define MAX_THREADS 128
#define NUM_SIMULATIONS 400000
#define MAX_CHILDREN_PER_NODE 17  // Limit branching factor for deeper search

#define LOAD_BALANCE_CHECK_INTERVAL 1000

typedef struct game_s game_t;

typedef struct      node_s
{
    atomic_int      value;
    atomic_int      visit_count;
    _Atomic float   uct;
    int             x;
    int             y;
    int             player; // 0 for player 1, 1 for player 2
    atomic_int      nb_childs;  // Must be atomic for thread-safe access
    int             max_childs;      // capped to MAX_CHILDREN_PER_NODE
    int             num_valid_moves; // actual number of valid moves (for sorting)
    int             valid_moves[361];
    pthread_mutex_t mutex;
    struct node_s   *parent;
    struct node_s   **childs;
}   node_t;

typedef struct load_balancer_s
{
    atomic_int global_sim_counter;
    atomic_int max_depth;
    atomic_int thread_sim_counts[MAX_THREADS];
    atomic_llong thread_times_ns[MAX_THREADS];
    atomic_llong thread_phase_times_ns[MAX_THREADS][4]; // per-thread phase times (selection, expansion, simulation, backpropagation)
    pthread_mutex_t balance_mutex;
    int active_threads;
    int total_simulations;
} load_balancer_t;


typedef struct bot_config_s bot_config_t;

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
    bot_config_t *config;  // Bot configuration for this thread
} thread_data_t;

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
void    weightmap_inplace(unsigned int board[19][19], int *valid_moves, int max_size, int player, float *weights);

#endif