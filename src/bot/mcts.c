#define _POSIX_C_SOURCE 199309L
#include "mcts.h"
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

node_t *create_node(int x, int y, node_t *parent, mcts_t *mcts, int parent_player)
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
        }
        max_childs = get_empty_positions(mcts->board, x, y, node->valid_moves);
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
    node->childs = (node_t **)malloc(sizeof(node_t *) * max_childs);
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
    
    for (int i = 0; i < node->nb_childs; i++) {
        free_tree(node->childs[i]);
    }
    
    if (node->childs)
        free(node->childs);
    free(node);
}

void find_next_valid_move(node_t *node, mcts_t *mcts, int *res_x, int *res_y)
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

void get_random_move(node_t *node, mcts_t *mcts, int *res_x, int *res_y)
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

node_t *selection(node_t *node, mcts_t *mcts)
{
    if (node->nb_childs == 0 || node->nb_childs < node->max_childs)
        return node;

    node_t *best = node->childs[0];

    float best_uct = calculate_uct(best);
    for (int i = 1; i < node->nb_childs; i++) {
        float uct = calculate_uct(node->childs[i]);
        if (uct > best_uct) {
            best_uct = uct;
            best = node->childs[i];
        }
    }

    mcts->board[best->x][best->y] = best->player + 1;
    remove_captured_stones(mcts->board, check_captures(mcts->board, best->x, best->y, best->player, &mcts->captured_black, &mcts->captured_white));

    int ret = 0;
    if ((ret = check_win_mcts(mcts, best->x, best->y, node->x, node->y, best->player)) != -1) {
        best->nb_childs = -(ret + 1); // Mark as terminal -1 for player 1 win, -2 for player 2 win
        return best;
    }

    return selection(best, mcts);
}

node_t *expansion(node_t *node, mcts_t *mcts)
{
    if (!node)
        return NULL;
    
    pthread_mutex_lock(&node->mutex);

    // If first expansion, sort valid_moves by heuristic score once
    
    if (node->nb_childs == 0 && node->max_childs > 1) {
        float *weights = weightmap(mcts->board, node->valid_moves, node->max_childs, node->player);
        // Simple insertion sort by weight (descending)
        for (int i = 1; i < node->max_childs; i++) {
            int key_move = node->valid_moves[i];
            float key_weight = weights[i];
            int j = i - 1;
            while (j >= 0 && weights[j] < key_weight) {
                node->valid_moves[j + 1] = node->valid_moves[j];
                weights[j + 1] = weights[j];
                j--;
            }
            node->valid_moves[j + 1] = key_move;
            weights[j + 1] = key_weight;
        }
        free(weights);
    }
    
    int x = -1, y = -1;
    find_next_valid_move(node, mcts, &x, &y);
    if (x == -1 || y == -1) {
        pthread_mutex_unlock(&node->mutex);
        return NULL;
    }

    node_t *child = create_node(x, y, node, mcts, node->player);
    if (!child) {
        pthread_mutex_unlock(&node->mutex);
        return NULL;
    }
    node->childs[node->nb_childs] = child;
    node->nb_childs++;
    pthread_mutex_unlock(&node->mutex);

    return child;
}

int simulation(node_t *node, mcts_t sim_mcts)
{
    int ret = 0;
    int limit = 1000;
    int prev_x = -1, prev_y = -1;
    int depth = 0; // Track simulation depth

    node_t sim_node = *node;
    memcpy(sim_node.valid_moves, node->valid_moves, sizeof(int) * 361);

    if (node->parent != NULL) {
        prev_x = node->parent->x;
        prev_y = node->parent->y;
    }

    while ((ret = check_win_mcts(&sim_mcts, sim_node.x, sim_node.y, prev_x, prev_y, sim_node.player)) == -1 && limit-- > 0)
    {
        sim_node.player = !sim_node.player;
        prev_x = sim_node.x;
        prev_y = sim_node.y;
        sim_node.nb_childs++;

        get_random_move(&sim_node, &sim_mcts, &sim_node.x, &sim_node.y);

        if (sim_node.x == -1 || sim_node.y == -1)
            break;

        sim_mcts.board[sim_node.x][sim_node.y] = sim_node.player + 1;
        remove_captured_stones(sim_mcts.board, check_captures(sim_mcts.board, sim_node.x, sim_node.y, sim_node.player, &sim_mcts.captured_black, &sim_mcts.captured_white));
        
        depth++;
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
    printf("MCTS Results:\n");
    printf("Total simulations: %d\n", root->visit_count);
    printf("Root value: %d\n", root->value);
    printf("Child Nodes:\n");
    for (int i = 0; i < root->nb_childs; i++) {
        node_t *child = root->childs[i];
        printf("Move (%d, %d): Value = %d, Visits = %d, UCT = %.4f\n", child->x, child->y, child->value, child->visit_count, calculate_uct(child));
    }
    printf("Best Move: (%d, %d)\n", *res_x, *res_y);
}

void get_best_move(node_t *root, int *res_x, int *res_y)
{
    node_t *best = NULL;
    int best_visits = -1;

    for (int i = 0; i < root->nb_childs; i++) {
        node_t *child = root->childs[i];
        if (child->visit_count > best_visits) {
            best_visits = child->visit_count;
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

void    init_load_balancer(load_balancer_t *balancer, int total_sims, int num_threads) {
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

void    *mcts_thread_worker(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    node_t *root = data->node;
    load_balancer_t *balancer = data->balancer;
    int thread_id = data->thread_id;
    
    // Initialize thread-local random seed
    srand(data->rand_seed);
    
    int sim_result = 0;
    int local_sim_count = 0;
    struct timespec thread_start, thread_end, phase_start, phase_end;
    
    clock_gettime(CLOCK_MONOTONIC, &thread_start);

    // Each thread runs until global simulation counter reaches total_simulations
    while (1) {
        // Check if we've reached the total simulation limit
        int current_global_count = atomic_fetch_add(&balancer->global_sim_counter, 1);
        if (current_global_count >= balancer->total_simulations) {
            atomic_fetch_sub(&balancer->global_sim_counter, 1);
            break;
        }

        mcts_t sim_mcts;
        memcpy(sim_mcts.board, data->board_sim, sizeof(unsigned int) * 19 * 19);
        sim_mcts.captured_black = data->captured_black;
        sim_mcts.captured_white = data->captured_white;
        sim_mcts.winning_state = data->winning_state;

        node_t *node = root;

        // Selection phase
        clock_gettime(CLOCK_MONOTONIC, &phase_start);
        node = selection(node, &sim_mcts);
        clock_gettime(CLOCK_MONOTONIC, &phase_end);
        long long selection_ns = (phase_end.tv_sec - phase_start.tv_sec) * 1000000000LL + 
                                 (phase_end.tv_nsec - phase_start.tv_nsec);
        atomic_fetch_add(&balancer->thread_phase_times_ns[thread_id][0], selection_ns);

        if (node->nb_childs != -1) {
            // Expansion phase
            clock_gettime(CLOCK_MONOTONIC, &phase_start);
            node = expansion(node, &sim_mcts);
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
            sim_result = simulation(node, sim_mcts);
            clock_gettime(CLOCK_MONOTONIC, &phase_end);
            long long simulation_ns = (phase_end.tv_sec - phase_start.tv_sec) * 1000000000LL + 
                                      (phase_end.tv_nsec - phase_start.tv_nsec);
            atomic_fetch_add(&balancer->thread_phase_times_ns[thread_id][2], simulation_ns);
        } else {
            sim_result = -node->nb_childs - 1;
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

    printf("\n\e[1;33mDetected %d CPU cores, using %d threads for MCTS\e[0m\n", num_threads, num_threads);

    mcts_t mcts = init_mcts(game);
    node_t *root = create_node(-1, -1, NULL, &mcts, game->current_player);
    
    if (!root) {
        *res_x = -1;
        *res_y = -1;
        return;
    }
    
    pthread_t threads[MAX_THREADS];
    thread_data_t thread_data[MAX_THREADS];
    load_balancer_t balancer;
    
    init_load_balancer(&balancer, NUM_SIMULATIONS, num_threads);

    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    for (int i = 0; i < num_threads; i++) {
        for (int m = 0; m < 19; m++) {
            for (int n = 0; n < 19; n++) {
                thread_data[i].board_sim[m][n] = mcts.board[m][n];
            }
        }
        thread_data[i].captured_black = mcts.captured_black;
        thread_data[i].captured_white = mcts.captured_white;
        thread_data[i].winning_state = mcts.winning_state;
        thread_data[i].thread_id = i;
        thread_data[i].node = root;
        thread_data[i].rand_seed = (unsigned int)time(NULL) + i * 1000;
        thread_data[i].balancer = &balancer;

        if (pthread_create(&threads[i], NULL, mcts_thread_worker, &thread_data[i]) != 0) {
            fprintf(stderr, "Error creating thread %d\n", i);
            free_tree(root);
            pthread_mutex_destroy(&balancer.balance_mutex);
            return;
        }
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);
    
    double elapsed_seconds = (end_time.tv_sec - start_time.tv_sec) + 
                            (end_time.tv_nsec - start_time.tv_nsec) / 1e9;
    
    // Calculate total simulations completed
    int total_sims = atomic_load(&balancer.global_sim_counter);
    
    printf("\e[1;34m=== MCTS Performance Statistics ===\e[0m\n");
    printf("Total simulations: %d / %d requested\n", total_sims, NUM_SIMULATIONS);
    printf("Number of threads: %d\n", num_threads);
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

    get_best_move(root, res_x, res_y);
    pthread_mutex_destroy(&balancer.balance_mutex);
    free_tree(root);
}