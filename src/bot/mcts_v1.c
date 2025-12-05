#define _POSIX_C_SOURCE 199309L
#include "mcts_v1.h"
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// V1: Original implementation without optimizations

void partial_select_top_k_v1(int *moves, float *weights, int n, int k)
{
    if (k <= 0 || n <= 0)
        return;
    if (k > n)
        k = n;

    for (int t = 0; t < k; t++) {
        int best = t;
        for (int i = t + 1; i < n; i++) {
            if (weights[i] > weights[best])
                best = i;
        }
        if (best != t) {
            int mv = moves[t];
            moves[t] = moves[best];
            moves[best] = mv;
            float w = weights[t];
            weights[t] = weights[best];
            weights[best] = w;
        }
    }
}

int get_empty_positions_v1(unsigned int board[19][19], int x, int y, int valid_moves[361])
{
    int count = 0;
    for (int i = 0; i < 19; i++) {
        for (int j = 0; j < 19; j++) {
            if (i == x && j == y)
                continue;
            if (board[i][j] == 0) {
                valid_moves[count] = i * 19 + j;
                count++;
            }
        }
    }
    return count;
}

node_v1_t *create_node_v1(int x, int y, node_v1_t *parent, mcts_v1_t *mcts, int parent_player)
{
    node_v1_t *node = (node_v1_t *)malloc(sizeof(node_v1_t));
    if (!node)
        return NULL;

    int max_childs = 0;
    if (parent == NULL)
        max_childs = get_empty_positions_v1(mcts->board, x, y, node->valid_moves);
    else {
        mcts->board[x][y] = !parent_player + 1;

        unsigned int *captured = check_captures(mcts->board, x, y, !parent_player, &mcts->captured_black, &mcts->captured_white);
        if (captured) {
            remove_captured_stones(mcts->board, captured);
        }
        max_childs = get_empty_positions_v1(mcts->board, x, y, node->valid_moves);
    }

    atomic_store(&node->value, 0);
    atomic_store(&node->visit_count, 0);
    atomic_store(&node->uct, 0);
    node->x = x;
    node->y = y;
    node->nb_childs = 0;
    node->max_childs = max_childs;
    node->player = !parent_player;
    pthread_mutex_init(&node->mutex, NULL);
    node->parent = parent;
    node->childs = (node_v1_t **)malloc(sizeof(node_v1_t *) * max_childs);
    if (!node->childs) {
        pthread_mutex_destroy(&node->mutex);
        free(node);
        return NULL;
    }

    return node;
}

mcts_v1_t init_mcts_v1(game_t *game)
{
    mcts_v1_t mcts;
    memcpy(mcts.board, game->board, sizeof(unsigned int) * 19 * 19);
    mcts.captured_black = game->captured_black;
    mcts.captured_white = game->captured_white;
    mcts.winning_state = game->moves ? game->moves->winning_state : 0;
    return mcts;
}

void free_tree_v1(node_v1_t *node)
{
    if (!node)
        return;
    
    for (int i = 0; i < node->nb_childs; i++) {
        free_tree_v1(node->childs[i]);
    }

    pthread_mutex_destroy(&node->mutex);
    if (node->childs)
        free(node->childs);
    free(node);
}

void find_next_valid_move_v1(node_v1_t *node, mcts_v1_t *mcts, int *res_x, int *res_y)
{
    if (node->nb_childs >= node->max_childs) {
        *res_x = -1;
        *res_y = -1;
        return;
    }
    int counter = 0;
    while (node->nb_childs + counter < node->max_childs) {
        if (mcts->board[node->valid_moves[node->nb_childs + counter] / 19][node->valid_moves[node->nb_childs + counter] % 19] == 0 && !check_double_free_three(mcts->board, node->valid_moves[node->nb_childs + counter] / 19, node->valid_moves[node->nb_childs + counter] % 19, node->player)) {
            *res_x = node->valid_moves[node->nb_childs + counter] / 19;
            *res_y = node->valid_moves[node->nb_childs + counter] % 19;
            return;
        }
        counter++;
    }
}

void get_random_move_v1(node_v1_t *node, mcts_v1_t *mcts, int *res_x, int *res_y)
{
    int choice = rand() % node->max_childs;
    for (int i = 0; i < node->max_childs; i++) {
        int idx = (choice + i) % node->max_childs;
        int x = node->valid_moves[idx] / 19;
        int y = node->valid_moves[idx] % 19;
        if (mcts->board[x][y] == 0) {
            *res_x = x;
            *res_y = y;
            return;
        }
    }
    *res_x = -1;
    *res_y = -1;
}

int check_win_mcts_v1(mcts_v1_t *mcts, int x, int y, int prev_x, int prev_y, unsigned int player) {
    int ret = 0;
    if (mcts->captured_black >= 10 || mcts->captured_white >= 10)
        return player;

    if (prev_x != -1 && prev_y != -1 && (mcts->winning_state > 2 || mcts->winning_state == !player + 1)) {
        ret = check_five_in_a_row_at(mcts->board, prev_x, prev_y, !player, 0);
        if (ret)
            return !player;
        else
            mcts->winning_state -= !player + 1;
    }
    ret = check_five_in_a_row_at(mcts->board, x, y, player, 0);
    if (ret)
        mcts->winning_state += player + 1;
    return -1;
}

// V1: Original UCT without caching or virtual losses
float calculate_uct_v1(node_v1_t *node)
{
    if (atomic_load(&node->visit_count) == 0)
        return INFINITY;
    
    int value = atomic_load(&node->value);
    int visit = atomic_load(&node->visit_count);

    float exploitation = (float)value / (float)visit;
    float exploration = sqrtf(2.0f) * sqrtf(logf((float)atomic_load(&node->parent->visit_count)) / (float)visit);

    return exploitation + exploration;
}

// V1: Original selection without virtual losses
node_v1_t *selection_v1(node_v1_t *node, mcts_v1_t *mcts)
{
    if (node->nb_childs == 0 || node->nb_childs < node->max_childs)
        return node;

    node_v1_t *best = node->childs[0];

    float best_uct = calculate_uct_v1(best);
    for (int i = 1; i < node->nb_childs; i++) {
        float uct = calculate_uct_v1(node->childs[i]);
        if (uct > best_uct) {
            best_uct = uct;
            best = node->childs[i];
        }
    }

    mcts->board[best->x][best->y] = best->player + 1;
    unsigned int *captured = check_captures(mcts->board, best->x, best->y, best->player, &mcts->captured_black, &mcts->captured_white);
    remove_captured_stones(mcts->board, captured);
    if (captured) free(captured);

    int ret = 0;
    if ((ret = check_win_mcts_v1(mcts, best->x, best->y, node->x, node->y, best->player)) != -1) {
        best->nb_childs = -(ret + 1);
        return best;
    }

    return selection_v1(best, mcts);
}

// V1: Original expansion - sorts EVERY time
node_v1_t *expansion_v1(node_v1_t *node, mcts_v1_t *mcts)
{
    if (!node)
        return NULL;
    
    pthread_mutex_lock(&node->mutex);
     
    // V1: Sort every time (no sorted_once flag)
    if (node->nb_childs == 0 && node->max_childs > 1) {
        float *weights = weightmap(mcts->board, node->valid_moves, node->max_childs, node->player);

        int k = MCTS_TOP_K_V1;
        if (k > node->max_childs)
            k = node->max_childs;
        partial_select_top_k_v1(node->valid_moves, weights, node->max_childs, k);
        free(weights);
    }
    
    int x = -1, y = -1;
    find_next_valid_move_v1(node, mcts, &x, &y);
    if (x == -1 || y == -1) {
        pthread_mutex_unlock(&node->mutex);
        return NULL;
    }

    node_v1_t *child = create_node_v1(x, y, node, mcts, node->player);
    if (!child) {
        pthread_mutex_unlock(&node->mutex);
        return NULL;
    }
    node->childs[node->nb_childs] = child;
    node->nb_childs++;
    pthread_mutex_unlock(&node->mutex);

    return child;
}

// V1: Original simulation with 1000 move limit
int simulation_v1(node_v1_t *node, mcts_v1_t sim_mcts)
{
    int ret = 0;
    int limit = 1000;  // V1: Original long playouts
    int prev_x = -1, prev_y = -1;

    node_v1_t sim_node = *node;
    memcpy(sim_node.valid_moves, node->valid_moves, sizeof(int) * 361);

    if (node->parent != NULL) {
        prev_x = node->parent->x;
        prev_y = node->parent->y;
    }

    while ((ret = check_win_mcts_v1(&sim_mcts, sim_node.x, sim_node.y, prev_x, prev_y, sim_node.player)) == -1 && limit-- > 0)
    {
        sim_node.player = !sim_node.player;
        prev_x = sim_node.x;
        prev_y = sim_node.y;
        sim_node.nb_childs++;

        get_random_move_v1(&sim_node, &sim_mcts, &sim_node.x, &sim_node.y);

        if (sim_node.x == -1 || sim_node.y == -1)
            break;

        sim_mcts.board[sim_node.x][sim_node.y] = sim_node.player + 1;
        unsigned int *sim_captured = check_captures(sim_mcts.board, sim_node.x, sim_node.y, sim_node.player, &sim_mcts.captured_black, &sim_mcts.captured_white);
        remove_captured_stones(sim_mcts.board, sim_captured);
        if (sim_captured) free(sim_captured);
    }
    return ret;
}

// V1: Original backpropagation without virtual losses
void backpropagation_v1(node_v1_t *node, int result)
{
    while (node != NULL) {
        atomic_fetch_add(&node->visit_count, 1);
        atomic_fetch_add(&node->value, (node->player == result) ? 1 : -1);
        node = node->parent;
    }
}

void get_best_move_v1(node_v1_t *root, int *res_x, int *res_y)
{
    node_v1_t *best = NULL;
    int best_visits = -1;

    for (int i = 0; i < root->nb_childs; i++) {
        node_v1_t *child = root->childs[i];
        if (atomic_load(&child->visit_count) > best_visits) {
            best_visits = atomic_load(&child->visit_count);
            best = child;
        }
    }

    if (best) {
        *res_x = best->x;
        *res_y = best->y;
    } else {
        *res_x = -1;
        *res_y = -1;
    }
}

void init_load_balancer_v1(load_balancer_v1_t *balancer, int total_sims, int num_threads) {
    atomic_store(&balancer->global_sim_counter, 0);
    for (int i = 0; i < num_threads; i++) {
        atomic_store(&balancer->thread_sim_counts[i], 0);
        atomic_store(&balancer->thread_times_ns[i], 0);
        for (int phase = 0; phase < 4; phase++) {
            atomic_store(&balancer->thread_phase_times_ns[i][phase], 0);
        }
    }
    pthread_mutex_init(&balancer->balance_mutex, NULL);
    balancer->active_threads = num_threads;
    balancer->total_simulations = total_sims;
}

void *mcts_thread_worker_v1(void *arg) {
    thread_data_v1_t *data = (thread_data_v1_t *)arg;
    node_v1_t *root = data->node;
    load_balancer_v1_t *balancer = data->balancer;
    int thread_id = data->thread_id;
    
    srand(data->rand_seed);
    
    int sim_result = 0;
    struct timespec thread_start, thread_end, phase_start, phase_end;
    
    clock_gettime(CLOCK_MONOTONIC, &thread_start);

    while (1) {
        int current_global_count = atomic_fetch_add(&balancer->global_sim_counter, 1);
        if (current_global_count >= balancer->total_simulations) {
            atomic_fetch_sub(&balancer->global_sim_counter, 1);
            break;
        }

        mcts_v1_t sim_mcts;
        memcpy(sim_mcts.board, data->board_sim, sizeof(unsigned int) * 19 * 19);
        sim_mcts.captured_black = data->captured_black;
        sim_mcts.captured_white = data->captured_white;
        sim_mcts.winning_state = data->winning_state;

        node_v1_t *node = root;

        clock_gettime(CLOCK_MONOTONIC, &phase_start);
        node = selection_v1(node, &sim_mcts);
        clock_gettime(CLOCK_MONOTONIC, &phase_end);
        long long selection_ns = (phase_end.tv_sec - phase_start.tv_sec) * 1000000000LL + 
                                 (phase_end.tv_nsec - phase_start.tv_nsec);
        atomic_fetch_add(&balancer->thread_phase_times_ns[thread_id][0], selection_ns);

        if (node->nb_childs != -1) {
            clock_gettime(CLOCK_MONOTONIC, &phase_start);
            node = expansion_v1(node, &sim_mcts);
            clock_gettime(CLOCK_MONOTONIC, &phase_end);
            long long expansion_ns = (phase_end.tv_sec - phase_start.tv_sec) * 1000000000LL + 
                                     (phase_end.tv_nsec - phase_start.tv_nsec);
            atomic_fetch_add(&balancer->thread_phase_times_ns[thread_id][1], expansion_ns);
            
            if (!node) {
                atomic_fetch_add(&balancer->thread_sim_counts[thread_id], 1);
                continue;
            }
            
            clock_gettime(CLOCK_MONOTONIC, &phase_start);
            sim_result = simulation_v1(node, sim_mcts);
            clock_gettime(CLOCK_MONOTONIC, &phase_end);
            long long simulation_ns = (phase_end.tv_sec - phase_start.tv_sec) * 1000000000LL + 
                                      (phase_end.tv_nsec - phase_start.tv_nsec);
            atomic_fetch_add(&balancer->thread_phase_times_ns[thread_id][2], simulation_ns);
        } else {
            sim_result = -node->nb_childs - 1;
        }
        
        clock_gettime(CLOCK_MONOTONIC, &phase_start);
        backpropagation_v1(node, sim_result);
        clock_gettime(CLOCK_MONOTONIC, &phase_end);
        long long backprop_ns = (phase_end.tv_sec - phase_start.tv_sec) * 1000000000LL + 
                                (phase_end.tv_nsec - phase_start.tv_nsec);
        atomic_fetch_add(&balancer->thread_phase_times_ns[thread_id][3], backprop_ns);
        
        atomic_fetch_add(&balancer->thread_sim_counts[thread_id], 1);
    }

    clock_gettime(CLOCK_MONOTONIC, &thread_end);
    long long elapsed_ns = (thread_end.tv_sec - thread_start.tv_sec) * 1000000000LL + 
                           (thread_end.tv_nsec - thread_start.tv_nsec);
    atomic_store(&balancer->thread_times_ns[thread_id], elapsed_ns);

    pthread_exit(NULL);
}

void run_mcts_v1(game_t *game, int *res_x, int *res_y, int num_simulations) {
    int num_threads = (int)sysconf(_SC_NPROCESSORS_ONLN);
    if (num_threads <= 0 || num_threads > MAX_THREADS) {
        num_threads = 8;
    }

    mcts_v1_t mcts = init_mcts_v1(game);
    node_v1_t *root = create_node_v1(-1, -1, NULL, &mcts, game->current_player);
    
    if (!root) {
        *res_x = -1;
        *res_y = -1;
        return;
    }
    
    pthread_t threads[MAX_THREADS];
    thread_data_v1_t thread_data[MAX_THREADS];
    load_balancer_v1_t balancer;
    
    init_load_balancer_v1(&balancer, num_simulations, num_threads);

    for (int i = 0; i < num_threads; i++) {
        memcpy(thread_data[i].board_sim, mcts.board, sizeof(unsigned int) * 19 * 19);
        thread_data[i].captured_black = mcts.captured_black;
        thread_data[i].captured_white = mcts.captured_white;
        thread_data[i].winning_state = mcts.winning_state;
        thread_data[i].thread_id = i;
        thread_data[i].node = root;
        thread_data[i].rand_seed = (unsigned int)time(NULL) + i * 1000;
        thread_data[i].balancer = &balancer;

        if (pthread_create(&threads[i], NULL, mcts_thread_worker_v1, &thread_data[i]) != 0) {
            fprintf(stderr, "Error creating thread %d\n", i);
            free_tree_v1(root);
            pthread_mutex_destroy(&balancer.balance_mutex);
            return;
        }
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    get_best_move_v1(root, res_x, res_y);
    pthread_mutex_destroy(&balancer.balance_mutex);
    free_tree_v1(root);
}
