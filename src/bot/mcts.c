#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 199309L
#include "mcts.h"
#include "bot_config.h"
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int get_empty_positions(unsigned int board[19][19], int x, int y, int valid_moves[361])
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

node_t *create_node(int x, int y, node_t *parent, mcts_t *mcts, int parent_player, bot_config_t *config)
{
    node_t *node = (node_t *)malloc(sizeof(node_t));
    if (!node)
        return NULL;

    int max_childs = 0;
    if (parent == NULL)
        max_childs = get_empty_positions(mcts->board, x, y, node->valid_moves);
    else {
        mcts->board[x][y] = !parent_player + 1;

        unsigned int *captured = check_captures(mcts->board, x, y, !parent_player, &mcts->captured_black, &mcts->captured_white);
        if (captured) {
            remove_captured_stones(mcts->board, captured);
            free(captured);
        }
        max_childs = get_empty_positions(mcts->board, x, y, node->valid_moves);
    }

    atomic_store(&node->value, 0);
    atomic_store(&node->visit_count, 0);
    atomic_store(&node->uct, 0);
    node->x = x;
    node->y = y;
    atomic_store(&node->nb_childs, 0);
    node->num_valid_moves = max_childs; // store actual count for sorting
    // Limit branching factor for deeper search (use config if available)
    int max_children_limit = config ? config->max_children : MAX_CHILDREN_PER_NODE;
    node->max_childs = (max_childs > max_children_limit) ? max_children_limit : max_childs;
    node->player = !parent_player;
    pthread_mutex_init(&node->mutex, NULL);
    node->parent = parent;
    node->childs = (node_t **)malloc(sizeof(node_t *) * node->max_childs);
    if (!node->childs) {
        pthread_mutex_destroy(&node->mutex);
        free(node);
        return NULL;
    }

    return node;
}

mcts_t init_mcts(game_t *game)
{
    mcts_t mcts;
    memcpy(mcts.board, game->board, sizeof(unsigned int) * 19 * 19);
    mcts.captured_black = game->captured_black;
    mcts.captured_white = game->captured_white;
    mcts.winning_state = game->moves ? game->moves->winning_state : 0;
    return mcts;
}

void free_tree(node_t *node)
{
    if (!node)
        return;
    
    int num_childs = atomic_load(&node->nb_childs);
    for (int i = 0; i < num_childs; i++) {
        free_tree(node->childs[i]);
    }
    
    pthread_mutex_destroy(&node->mutex);
    if (node->childs)
        free(node->childs);
    free(node);
}

void find_next_valid_move(node_t *node, mcts_t *mcts, int *res_x, int *res_y)
{
    int nb = atomic_load(&node->nb_childs);
    if (nb >= node->max_childs) {
        *res_x = -1;
        *res_y = -1;
        return;
    }
    int counter = 0;
    while (nb + counter < node->max_childs) {
        if (mcts->board[node->valid_moves[nb + counter] / 19][node->valid_moves[nb + counter] % 19] == 0 && !check_double_free_three(mcts->board, node->valid_moves[nb + counter] / 19, node->valid_moves[nb + counter] % 19, node->player)) {
            *res_x = node->valid_moves[nb + counter] / 19;
            *res_y = node->valid_moves[nb + counter] % 19;
            return;
        }
        counter++;
    }
    *res_x = -1;
    *res_y = -1;
}

void get_random_move(node_t *node, mcts_t *mcts, int *res_x, int *res_y, unsigned int *seed)
{
    int choice = rand_r(seed) % node->max_childs;
    for (int i = 0; i < node->max_childs; i++) {
        int idx = (choice + i) % node->max_childs;
        int move = node->valid_moves[idx];
        int x = move / 19;
        int y = move - x * 19;
        if (mcts->board[x][y] == 0) {
            *res_x = x;
            *res_y = y;
            return;
        }
    }
    *res_x = -1;
    *res_y = -1;
}

int check_win_mcts(mcts_t *mcts, int x, int y, int prev_x, int prev_y, unsigned int player) {
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

float calculate_uct(node_t *node)
{
    if (node->visit_count == 0)
        return INFINITY;
    
    int value = atomic_load(&node->value);
    int visit = atomic_load(&node->visit_count);

    float exploitation = (float)value / (float)visit;
    float exploration = sqrtf(2.0f) * sqrtf(logf((float)node->parent->visit_count) / (float)visit);

    return exploitation + exploration;
}

float calculate_uct_with_log(node_t *node, float log_parent_visits, bot_config_t *config)
{
    if (node->visit_count == 0)
        return INFINITY;
    
    int value = atomic_load(&node->value);
    int visit = atomic_load(&node->visit_count);

    float exploration_constant = config ? config->exploration_constant : 1.414f;
    float exploitation = (float)value / (float)visit;
    float exploration = exploration_constant * sqrtf(log_parent_visits / (float)visit);

    return exploitation + exploration;
}

node_t *selection(node_t *node, mcts_t *mcts, int *depth, bot_config_t *config)
{
    int nb = atomic_load(&node->nb_childs);
    if (nb == 0 || nb < node->max_childs)
        return node;

    (*depth)++;

    float log_parent = logf((float)atomic_load(&node->visit_count));
    node_t *best = node->childs[0];

    float best_uct = calculate_uct_with_log(best, log_parent, config);
    for (int i = 1; i < nb; i++) {
        float uct = calculate_uct_with_log(node->childs[i], log_parent, config);
        if (uct > best_uct) {
            best_uct = uct;
            best = node->childs[i];
        }
    }

    mcts->board[best->x][best->y] = best->player + 1;
    unsigned int *captured = check_captures(mcts->board, best->x, best->y, best->player, &mcts->captured_black, &mcts->captured_white);
    if (captured) {
        remove_captured_stones(mcts->board, captured);
        free(captured);
    }

    int ret = 0;
    if ((ret = check_win_mcts(mcts, best->x, best->y, node->x, node->y, best->player)) != -1) {
        atomic_store(&best->nb_childs, -(ret + 1)); // Mark as terminal -1 for player 1 win, -2 for player 2 win
        return best;
    }

    return selection(best, mcts, depth, config);
}

// Partial sort: find top N best moves instead of sorting all
static void partial_sort_moves(int *valid_moves, float *weights, int max_size, int top_n) {
    if (top_n > max_size) top_n = max_size;
    
    for (int i = 0; i < top_n; i++) {
        int best_idx = i;
        float best_w = weights[i];
        for (int j = i + 1; j < max_size; j++) {
            if (weights[j] > best_w) {
                best_w = weights[j];
                best_idx = j;
            }
        }
        if (best_idx != i) {
            // Swap weights
            float tmp_w = weights[i];
            weights[i] = weights[best_idx];
            weights[best_idx] = tmp_w;
            
            // Swap moves
            int tmp_m = valid_moves[i];
            valid_moves[i] = valid_moves[best_idx];
            valid_moves[best_idx] = tmp_m;
        }
    }
}

node_t *expansion(node_t *node, mcts_t *mcts, bot_config_t *config)
{
    if (!node)
        return NULL;
    
    // Reserve a slot atomically before taking the lock
    int nb = atomic_load(&node->nb_childs);
    if (nb >= node->max_childs)
        return NULL;
    
    pthread_mutex_lock(&node->mutex);

    // Re-check after acquiring lock (double-check pattern)
    nb = atomic_load(&node->nb_childs);
    
    // If first expansion, sort valid_moves by heuristic score once
    if (nb == 0 && node->max_childs > 1) {
        // Use stack-allocated array instead of malloc
        float weights[361];
        // Sort from ALL valid moves to get the best ones
        weightmap_inplace(mcts->board, node->valid_moves, node->num_valid_moves, node->player, weights);
        
        // Sort top moves (at least max_childs, but cap at 50 for efficiency)
        int sort_count = node->max_childs > 50 ? 50 : node->max_childs;
        partial_sort_moves(node->valid_moves, weights, node->num_valid_moves, sort_count);
        // No free() needed - stack allocated!
    }
    
    // Check if we can still add children
    if (nb >= node->max_childs) {
        pthread_mutex_unlock(&node->mutex);
        return NULL;
    }
    
    int x = -1, y = -1;
    find_next_valid_move(node, mcts, &x, &y);
    if (x == -1 || y == -1) {
        pthread_mutex_unlock(&node->mutex);
        return NULL;
    }

    node_t *child = create_node(x, y, node, mcts, node->player, config);
    if (!child) {
        pthread_mutex_unlock(&node->mutex);
        return NULL;
    }
    
    // Atomically claim the slot and write the pointer
    node->childs[nb] = child;
    // Memory fence to ensure child pointer is visible before incrementing counter
    atomic_thread_fence(memory_order_release);
    atomic_store(&node->nb_childs, nb + 1);
    
    pthread_mutex_unlock(&node->mutex);

    return child;
}

int simulation(node_t *node, mcts_t sim_mcts, unsigned int *seed, int current_depth)
{
    int ret = 0;
    // Adaptive simulation depth: shallower sims deeper in tree
    int limit = 200 - (current_depth / 2);
    if (limit < 50) limit = 50;  // Minimum 50 moves
    
    int prev_x = -1, prev_y = -1;
    int sim_x = node->x;
    int sim_y = node->y;
    int sim_player = node->player;
    int sim_moves = 0;
    
    // Copy only valid_moves we'll actually use
    int local_moves[361];
    memcpy(local_moves, node->valid_moves, sizeof(int) * node->max_childs);
    int max_childs = node->max_childs;

    if (node->parent != NULL) {
        prev_x = node->parent->x;
        prev_y = node->parent->y;
    }

    while ((ret = check_win_mcts(&sim_mcts, sim_x, sim_y, prev_x, prev_y, sim_player)) == -1 && limit-- > 0)
    {
        sim_player = !sim_player;
        prev_x = sim_x;
        prev_y = sim_y;
        sim_moves++;

        // Inline random move selection to avoid function call overhead
        int choice = rand_r(seed) % max_childs;
        int found = 0;
        for (int i = 0; i < max_childs; i++) {
            int idx = (choice + i) % max_childs;
            int move = local_moves[idx];
            int x = move / 19;
            int y = move % 19;
            if (sim_mcts.board[x][y] == 0) {
                sim_x = x;
                sim_y = y;
                found = 1;
                break;
            }
        }
        
        if (!found)
            break;

        sim_mcts.board[sim_x][sim_y] = sim_player + 1;
        unsigned int *captured = check_captures(sim_mcts.board, sim_x, sim_y, sim_player, &sim_mcts.captured_black, &sim_mcts.captured_white);
        if (captured) {
            remove_captured_stones(sim_mcts.board, captured);
            free(captured);
        }
    }
    return ret;
}

void backpropagation(node_t *node, int result)
{
    while (node != NULL) {
        atomic_fetch_add(&node->visit_count, 1);
        atomic_fetch_add(&node->value, (node->player == result) ? 1 : -1);
        node = node->parent;
    }
}

void print_mcts_results(node_t *root, int *res_x, int *res_y)
{
    int nb = atomic_load(&root->nb_childs);
    printf("MCTS Results:\n");
    printf("Total simulations: %d\n", atomic_load(&root->visit_count));
    printf("Root value: %d\n", atomic_load(&root->value));
    printf("Child Nodes:\n");
    for (int i = 0; i < nb; i++) {
        node_t *child = root->childs[i];
        printf("Move (%d, %d): Value = %d, Visits = %d, UCT = %.4f\n", child->x, child->y, atomic_load(&child->value), atomic_load(&child->visit_count), calculate_uct(child));
    }
    printf("Best Move: (%d, %d)\n", *res_x, *res_y);
}

void get_best_move(node_t *root, int *res_x, int *res_y, game_t *game)
{
    node_t *best = NULL;
    int best_visits = -1;

    int nb = atomic_load(&root->nb_childs);
    for (int i = 0; i < nb; i++) {
        node_t *child = root->childs[i];
        int visits = atomic_load(&child->visit_count);
        
        // Only consider moves that are actually valid on the real game board
        if (visits > best_visits && check_valid_move(game, child->x, child->y)) {
            best_visits = visits;
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

void    init_load_balancer(load_balancer_t *balancer, double time_limit, int num_threads) {
    atomic_store(&balancer->global_sim_counter, 0);
    atomic_store(&balancer->max_depth, 0);
    for (int i = 0; i < num_threads; i++) {
        atomic_store(&balancer->thread_sim_counts[i], 0);
        atomic_store(&balancer->thread_times_ns[i], 0);
        for (int phase = 0; phase < 4; phase++) {
            atomic_store(&balancer->thread_phase_times_ns[i][phase], 0);
        }
    }
    pthread_mutex_init(&balancer->balance_mutex, NULL);
    balancer->active_threads = num_threads;
    balancer->time_limit_seconds = time_limit;
    clock_gettime(CLOCK_MONOTONIC, &balancer->start_time);
}

void    *mcts_thread_worker(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    node_t *root = data->node;
    load_balancer_t *balancer = data->balancer;
    int thread_id = data->thread_id;
    bot_config_t *config = data->config;
    
    // Thread-local random seed for rand_r
    unsigned int seed = data->rand_seed;
    
    int sim_result = 0;
    int local_sim_count = 0;
    struct timespec thread_start, thread_end, phase_start, phase_end;
    
    clock_gettime(CLOCK_MONOTONIC, &thread_start);

    // Each thread runs until time limit is reached
    while (1) {
        // Check if we've exceeded the time limit
        struct timespec current_time;
        clock_gettime(CLOCK_MONOTONIC, &current_time);
        double elapsed = (current_time.tv_sec - balancer->start_time.tv_sec) + 
                        (current_time.tv_nsec - balancer->start_time.tv_nsec) / 1e9;
        
        if (elapsed >= balancer->time_limit_seconds) {
            break;
        }
        
        atomic_fetch_add(&balancer->global_sim_counter, 1);

        mcts_t sim_mcts;
        memcpy(sim_mcts.board, data->board_sim, sizeof(unsigned int) * 19 * 19);
        sim_mcts.captured_black = data->captured_black;
        sim_mcts.captured_white = data->captured_white;
        sim_mcts.winning_state = data->winning_state;

        node_t *node = root;

        // Selection phase
        clock_gettime(CLOCK_MONOTONIC, &phase_start);
        int current_depth = 0;
        node = selection(node, &sim_mcts, &current_depth, config);
        clock_gettime(CLOCK_MONOTONIC, &phase_end);
        
        // Update max depth atomically
        int old_max = atomic_load(&balancer->max_depth);
        while (current_depth > old_max) {
            if (atomic_compare_exchange_weak(&balancer->max_depth, &old_max, current_depth))
                break;
        }
        
        long long selection_ns = (phase_end.tv_sec - phase_start.tv_sec) * 1000000000LL + 
                                 (phase_end.tv_nsec - phase_start.tv_nsec);
        atomic_fetch_add(&balancer->thread_phase_times_ns[thread_id][0], selection_ns);

        int nb = atomic_load(&node->nb_childs);
        if (nb >= 0) {  // nb_childs is negative for terminal nodes
            // Expansion phase
            clock_gettime(CLOCK_MONOTONIC, &phase_start);
            node = expansion(node, &sim_mcts, config);
            clock_gettime(CLOCK_MONOTONIC, &phase_end);
            long long expansion_ns = (phase_end.tv_sec - phase_start.tv_sec) * 1000000000LL + 
                                     (phase_end.tv_nsec - phase_start.tv_nsec);
            atomic_fetch_add(&balancer->thread_phase_times_ns[thread_id][1], expansion_ns);
            
            if (!node) {
                local_sim_count++;
                atomic_fetch_add(&balancer->thread_sim_counts[thread_id], 1);
                continue;
            }
            
            // Simulation phase
            clock_gettime(CLOCK_MONOTONIC, &phase_start);
            sim_result = simulation(node, sim_mcts, &seed, current_depth);
            clock_gettime(CLOCK_MONOTONIC, &phase_end);
            long long simulation_ns = (phase_end.tv_sec - phase_start.tv_sec) * 1000000000LL + 
                                      (phase_end.tv_nsec - phase_start.tv_nsec);
            atomic_fetch_add(&balancer->thread_phase_times_ns[thread_id][2], simulation_ns);
        } else {
            sim_result = -nb - 1;  // Use the local snapshot
        }
        
        // Backpropagation phase
        clock_gettime(CLOCK_MONOTONIC, &phase_start);
        backpropagation(node, sim_result);
        clock_gettime(CLOCK_MONOTONIC, &phase_end);
        long long backprop_ns = (phase_end.tv_sec - phase_start.tv_sec) * 1000000000LL + 
                                (phase_end.tv_nsec - phase_start.tv_nsec);
        atomic_fetch_add(&balancer->thread_phase_times_ns[thread_id][3], backprop_ns);
        
        local_sim_count++;
        atomic_fetch_add(&balancer->thread_sim_counts[thread_id], 1);
    }

    clock_gettime(CLOCK_MONOTONIC, &thread_end);
    long long elapsed_ns = (thread_end.tv_sec - thread_start.tv_sec) * 1000000000LL + 
                           (thread_end.tv_nsec - thread_start.tv_nsec);
    atomic_store(&balancer->thread_times_ns[thread_id], elapsed_ns);

    pthread_exit(NULL);
}

void run_mcts(game_t *game, int *res_x, int *res_y) {
        // Get number of CPU cores available
    int num_threads = (int)sysconf(_SC_NPROCESSORS_ONLN);
    if (num_threads <= 0 || num_threads > MAX_THREADS) {
        num_threads = 8; // Fallback to 8 threads if detection fails
    }

    printf("\n\e[1;33mDetected %d CPU cores, using %d threads for MCTS (%.1fs time limit)\e[0m\n", num_threads, num_threads, TIME_LIMIT_SECONDS);

    mcts_t mcts = init_mcts(game);
    node_t *root = create_node(-1, -1, NULL, &mcts, game->current_player, NULL);
    
    if (!root) {
        *res_x = -1;
        *res_y = -1;
        return;
    }
    
    pthread_t threads[MAX_THREADS];
    thread_data_t thread_data[MAX_THREADS];
    load_balancer_t balancer;
    
    init_load_balancer(&balancer, TIME_LIMIT_SECONDS, num_threads);

    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    int threads_created = 0;
    for (int i = 0; i < num_threads; i++) {
        memcpy(thread_data[i].board_sim, mcts.board, sizeof(unsigned int) * 19 * 19);
        thread_data[i].captured_black = mcts.captured_black;
        thread_data[i].captured_white = mcts.captured_white;
        thread_data[i].winning_state = mcts.winning_state;
        thread_data[i].thread_id = i;
        thread_data[i].node = root;
        thread_data[i].rand_seed = (unsigned int)time(NULL) + i * 1000;
        thread_data[i].balancer = &balancer;
        thread_data[i].config = NULL;

        if (pthread_create(&threads[i], NULL, mcts_thread_worker, &thread_data[i]) != 0) {
            fprintf(stderr, "Error creating thread %d\n", i);
            // Join already created threads
            for (int j = 0; j < threads_created; j++) {
                pthread_join(threads[j], NULL);
            }
            free_tree(root);
            pthread_mutex_destroy(&balancer.balance_mutex);
            *res_x = -1;
            *res_y = -1;
            return;
        }
        threads_created++;
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);
    
    double elapsed_seconds = (end_time.tv_sec - start_time.tv_sec) + 
                            (end_time.tv_nsec - start_time.tv_nsec) / 1e9;
    
    // Calculate total simulations completed
    int total_sims = atomic_load(&balancer.global_sim_counter);
    int max_depth = atomic_load(&balancer.max_depth);
    
    printf("\e[1;34m=== MCTS Performance Statistics ===\e[0m\n");
    printf("Total simulations: %d\n", total_sims);
    printf("Number of threads: %d\n", num_threads);
    printf("Max tree depth reached: %d\n", max_depth);
    printf("Total time: %.3f seconds\n", elapsed_seconds);
    printf("Simulations per second: %.2f\n", total_sims / elapsed_seconds);
    printf("Average time per simulation: %.6f seconds\n", elapsed_seconds / total_sims);
    
    printf("\n\e[1;33mPer-thread statistics:\e[0m\n");
    for (int i = 0; i < num_threads; i++) {
        int thread_sims = atomic_load(&balancer.thread_sim_counts[i]);
        long long thread_time_ns = atomic_load(&balancer.thread_times_ns[i]);
        double thread_time_s = thread_time_ns / 1e9;
        double thread_sims_per_sec = thread_sims > 0 ? thread_sims / thread_time_s : 0.0;
        
        printf("  Thread %d: %d simulations (%.1f%%) in %.3f seconds (%.2f sims/sec)\n",
               i, thread_sims, 
               100.0 * thread_sims / total_sims,
               thread_time_s,
               thread_sims_per_sec);
    }
    
    printf("\n\e[1;33mAverage phase times per thread:\e[0m\n");
    const char *phase_names[4] = {"Selection", "Expansion", "Simulation", "Backpropagation"};
    for (int phase = 0; phase < 4; phase++) {
        double total_phase_time_ns = 0.0;
        for (int i = 0; i < num_threads; i++) {
            total_phase_time_ns += atomic_load(&balancer.thread_phase_times_ns[i][phase]);
        }
        double avg_phase_time_s = (total_phase_time_ns / num_threads) / 1e9;
        printf("  %s: %.4f seconds (avg per thread)\n", phase_names[phase], avg_phase_time_s);
    }
    
    printf("\e[1;34m===================================\e[0m\n\n");

    get_best_move(root, res_x, res_y, game);
    pthread_mutex_destroy(&balancer.balance_mutex);
    free_tree(root);
}

// Run MCTS and return the root tree (for training data extraction)
// WARNING: Caller must free the returned tree with free_tree()
node_t* run_mcts_return_tree(game_t *game, int *res_x, int *res_y) {
    // Get number of CPU cores available
    int num_threads = (int)sysconf(_SC_NPROCESSORS_ONLN);
    if (num_threads <= 0 || num_threads > MAX_THREADS) {
        num_threads = 8;
    }

    mcts_t mcts = init_mcts(game);
    node_t *root = create_node(-1, -1, NULL, &mcts, game->current_player, NULL);
    
    if (!root) {
        *res_x = -1;
        *res_y = -1;
        return NULL;
    }
    
    pthread_t threads[MAX_THREADS];
    thread_data_t thread_data[MAX_THREADS];
    load_balancer_t balancer;
    
    init_load_balancer(&balancer, TIME_LIMIT_SECONDS, num_threads);

    int threads_created = 0;
    for (int i = 0; i < num_threads; i++) {
        memcpy(thread_data[i].board_sim, mcts.board, sizeof(unsigned int) * 19 * 19);
        thread_data[i].captured_black = mcts.captured_black;
        thread_data[i].captured_white = mcts.captured_white;
        thread_data[i].winning_state = mcts.winning_state;
        thread_data[i].thread_id = i;
        thread_data[i].node = root;
        thread_data[i].rand_seed = (unsigned int)time(NULL) + i * 1000;
        thread_data[i].balancer = &balancer;
        thread_data[i].config = NULL;

        if (pthread_create(&threads[i], NULL, mcts_thread_worker, &thread_data[i]) != 0) {
            for (int j = 0; j < threads_created; j++) {
                pthread_join(threads[j], NULL);
            }
            free_tree(root);
            pthread_mutex_destroy(&balancer.balance_mutex);
            *res_x = -1;
            *res_y = -1;
            return NULL;
        }
        threads_created++;
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    get_best_move(root, res_x, res_y, game);
    pthread_mutex_destroy(&balancer.balance_mutex);
    
    // Return root WITHOUT freeing it - caller is responsible
    return root;
}

// Run MCTS with a specific bot configuration
void run_mcts_with_config(game_t *game, int *res_x, int *res_y, bot_config_t *config)
{
    // Get number of CPU cores available
    int num_threads = config->num_threads;
    if (num_threads <= 0) {
        num_threads = (int)sysconf(_SC_NPROCESSORS_ONLN);
    }
    if (num_threads <= 0 || num_threads > MAX_THREADS) {
        num_threads = 8;
    }

    mcts_t mcts = init_mcts(game);
    node_t *root = create_node(-1, -1, NULL, &mcts, game->current_player, config);
    
    if (!root) {
        *res_x = -1;
        *res_y = -1;
        return;
    }
    
    pthread_t threads[MAX_THREADS];
    thread_data_t thread_data[MAX_THREADS];
    load_balancer_t balancer;
    
    // Use config time limit if available, otherwise use default
    double time_limit = config->time_limit > 0 ? config->time_limit : TIME_LIMIT_SECONDS;
    init_load_balancer(&balancer, time_limit, num_threads);

    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    int threads_created = 0;
    for (int i = 0; i < num_threads; i++) {
        memcpy(thread_data[i].board_sim, mcts.board, sizeof(unsigned int) * 19 * 19);
        thread_data[i].captured_black = mcts.captured_black;
        thread_data[i].captured_white = mcts.captured_white;
        thread_data[i].winning_state = mcts.winning_state;
        thread_data[i].thread_id = i;
        thread_data[i].node = root;
        thread_data[i].rand_seed = (unsigned int)time(NULL) + i * 1000;
        thread_data[i].balancer = &balancer;
        thread_data[i].config = config;  // Pass config to thread

        if (pthread_create(&threads[i], NULL, mcts_thread_worker, &thread_data[i]) != 0) {
            // Join already created threads
            for (int j = 0; j < threads_created; j++) {
                pthread_join(threads[j], NULL);
            }
            free_tree(root);
            pthread_mutex_destroy(&balancer.balance_mutex);
            *res_x = -1;
            *res_y = -1;
            return;
        }
        threads_created++;
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);
    
    get_best_move(root, res_x, res_y, game);
    pthread_mutex_destroy(&balancer.balance_mutex);
    free_tree(root);
}