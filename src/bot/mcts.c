#define _POSIX_C_SOURCE 200112L
#include "mcts.h"
#include <time.h>

node_t  *init_root(int player_color) {
    node_t *root = (node_t*)malloc(sizeof(node_t));
    
    if (!root)
        return (NULL);

    memset(root, 0, sizeof(node_t));
	atomic_store(&root->value, 0);
	atomic_store(&root->visit_count, 0);
    atomic_store(&root->uct, -1);
    root->x = -1;
    root->y = -1;
    root->color = player_color;
    root->nb_childs = 19 * 19;
    root->childs = NULL;
    root->parent = NULL;
    pthread_mutex_init(&root->mutex, NULL);

    if (!(root->childs = (node_t**)malloc(sizeof(node_t*) * root->nb_childs))) {
        pthread_mutex_destroy(&root->mutex);
        free(root);
        return (NULL);
    }
    memset(root->childs, 0, sizeof(node_t*) * root->nb_childs);
	return (root);
}

node_t  *add_new_child(node_t *parent, int index, int x, int y) {
	node_t	    *child;

	if (!(child = (node_t*)malloc(sizeof(node_t))))
		return (NULL);
	atomic_store(&child->value, 0);
	atomic_store(&child->visit_count, 0);
    atomic_store(&child->uct, -1);
    child->x = x;
    child->y = y;
    child->color = !parent->color;
    child->nb_childs = parent->nb_childs - 1;
    child->childs = NULL;
	child->parent = parent;
    pthread_mutex_init(&child->mutex, NULL);

    if (!(child->childs = (node_t**)malloc(sizeof(node_t*) * child->nb_childs))) {
        pthread_mutex_destroy(&child->mutex);
        free(child);
        return (NULL);
    }
    memset(child->childs, 0, sizeof(node_t*) * child->nb_childs);
    parent->childs[index] = child;
    return (child);
}

void    find_next_empty(unsigned int board_sim[19][19], int *x, int *y, int child_count) {
    for (int i = 0; i < 19; i++) {
        for (int j = 0; j < 19; j++) {
            if (board_sim[i][j] == 0 && child_count-- == 0) {
                *x = i;
                *y = j;
                return;
            }
        }
    }
    *x = -1;
    *y = -1; 
}

int check_win_mcts(unsigned int board_sim[19][19], unsigned int x, unsigned int y, int captured_black, int captured_white, int current_player, int *winning_state) {
    int ret = 0;
    if (captured_black >= 10 || captured_white >= 10)
        return current_player + 1;

    if (*winning_state > 2 || *winning_state == !current_player + 1) {
        ret = check_five_in_a_row(board_sim, !current_player);
        if (ret)
            return !current_player + 1;
        else
            *winning_state -= !current_player + 1;
    }
    ret = check_five_in_a_row_at(board_sim, x, y, current_player, 0);
    if (ret) {
        *winning_state += current_player + 1;
        return current_player + 1;  // Return winner immediately!
    }
    
    return 0;
}

void     random_choice(unsigned int board_sim[19][19], int *x, int *y, unsigned int *seed)
{
    int rx, ry;
    rx = rand_r(seed) % 19;
    ry = rand_r(seed) % 19;
    for (int i = 0; i < 19 * 19; i++) {
        int check_x = (rx + i / 19) % 19;
        int check_y = (ry + i % 19) % 19;
        if (board_sim[check_x][check_y] == 0) {
            *x = check_x;
            *y = check_y;
            return;
        }
    }
    *x = -1;
    *y = -1;
}

void update_uct(node_t *node) {
    if (node->visit_count > 0) {
        float exploit = (float)node->value / node->visit_count;
        float explore = sqrt(2 * log(node->parent->visit_count) / node->visit_count);
        atomic_store(&node->uct, exploit + explore);
    }
}


node_t    *selection(node_t *root, unsigned int board_sim[19][19]) {
    int i;
    int best;
    while (root->childs[root->nb_childs - 1] && root->childs[0] != NULL) {
        best = 0;
        i = 0;
        while (root->childs[i] && i < root->nb_childs) {
            update_uct(root->childs[i]);
            if (atomic_load(&root->childs[i]->uct) > atomic_load(&root->childs[best]->uct))
                best = i;
            i++;
        }
        root = root->childs[best];
        board_sim[root->x][root->y] = root->color == 0 ? 1 : 2;
    }
    return (root);
}

node_t    *expansion(node_t *cursor, unsigned int board_sim[19][19]) {
    int child_count = 0;
    int x = -1;
    int y = -1;
    node_t *new_child;

    pthread_mutex_lock(&cursor->mutex);
    
    while (cursor->childs && cursor->childs[child_count] != NULL) {
        child_count++;
    }
    
    find_next_empty(board_sim, &x, &y, child_count);

    if (x == -1 || y == -1) {
        pthread_mutex_unlock(&cursor->mutex);
        return (cursor);
    }

    new_child = add_new_child(cursor, child_count, x, y);
    pthread_mutex_unlock(&cursor->mutex);
    if (!new_child)
        return (cursor);
    
    board_sim[x][y] = cursor->color == 0 ? 2 : 1;
    return (new_child);
}

int     simulation(node_t *cursor, thread_data_t *thread) {
    int x = cursor->x;
    int y = cursor->y;
    int player = cursor->color;
    int winner = 0;
    while (!winner) {
        random_choice(thread->board_sim, &x, &y, &thread->rand_seed);
        if (x == -1 || y == -1)
        break;
        thread->board_sim[x][y] = player == 1 ? 1 : 2;
        winner = check_win_mcts(thread->board_sim, x, y, thread->captured_black, thread->captured_white, player, &thread->winning_state);
        if (winner != 0)
            return (winner - 1 == cursor->color ? 1 : -1);
        player = player == 1 ? 0 : 1;
    }
    return (winner);
}

node_t    *backPropagation(node_t *cursor, int sim_result) {
    int value_to_propagate = sim_result;
    
    while (cursor->parent != NULL) {
        atomic_fetch_add(&cursor->visit_count, 1);
        atomic_fetch_add(&cursor->value, value_to_propagate);
        value_to_propagate = -value_to_propagate;
        cursor = cursor->parent;
    }
    atomic_fetch_add(&cursor->visit_count, 1);
    return (cursor);
}

void    free_node(node_t *node) {
    if (!node)
        return;
    
    if (node->childs) {
        for (int i = 0; node->childs[i] != NULL && i < node->nb_childs; i++) {
            free_node(node->childs[i]);
        }
        free(node->childs);
    }
    
    pthread_mutex_destroy(&node->mutex);
    free(node);
}

void    init_load_balancer(load_balancer_t *balancer, int total_sims, int num_threads) {
    atomic_store(&balancer->global_sim_counter, 0);
    for (int i = 0; i < num_threads; i++) {
        atomic_store(&balancer->thread_sim_counts[i], 0);
        atomic_store(&balancer->thread_times_ns[i], 0);
    }
    pthread_mutex_init(&balancer->balance_mutex, NULL);
    balancer->active_threads = num_threads;
    balancer->total_simulations = total_sims;
}

void    print_load_balance_stats(load_balancer_t *balancer) {
    printf("\n=== Dynamic Load Balance Statistics ===\n");
    for (int i = 0; i < balancer->active_threads; i++) {
        int sims = atomic_load(&balancer->thread_sim_counts[i]);
        long long time_ns = atomic_load(&balancer->thread_times_ns[i]);
        double time_sec = time_ns / 1e9;
        double sims_per_sec = (time_sec > 0) ? sims / time_sec : 0;
        printf("Thread %d: %d sims (%.2f%%) in %.3fs (%.2f sims/s)\n", 
               i, sims, (sims * 100.0) / balancer->total_simulations, 
               time_sec, sims_per_sec);
    }
    printf("=======================================\n\n");
}

void    *mcts_thread_worker(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    load_balancer_t *balancer = data->balancer;
    struct timespec start_time, sim_start, sim_end;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    
    int local_sim_count = 0;
    
    while (1) {
        int global_count = atomic_fetch_add(&balancer->global_sim_counter, 1);
        
        if (global_count >= balancer->total_simulations)
            break;
        
        clock_gettime(CLOCK_MONOTONIC, &sim_start);
        
        unsigned int local_board[19][19];
        for (int m = 0; m < 19; m++) {
            for (int n = 0; n < 19; n++) {
                local_board[m][n] = data->board_sim[m][n];
            }
        }
        
        node_t *selected = selection(data->node, local_board);
        node_t *expanded = expansion(selected, local_board);
        
        thread_data_t sim_data = *data;
        for (int m = 0; m < 19; m++) {
            for (int n = 0; n < 19; n++) {
                sim_data.board_sim[m][n] = local_board[m][n];
            }
        }
        
        int winner = simulation(expanded, &sim_data);
        backPropagation(expanded, winner);
        
        clock_gettime(CLOCK_MONOTONIC, &sim_end);
        long long sim_time_ns = (sim_end.tv_sec - sim_start.tv_sec) * 1000000000LL + 
                                (sim_end.tv_nsec - sim_start.tv_nsec);
        atomic_fetch_add(&balancer->thread_times_ns[data->thread_id], sim_time_ns);
        
        local_sim_count++;
        atomic_fetch_add(&balancer->thread_sim_counts[data->thread_id], 1);
        /*
        if (local_sim_count % LOAD_BALANCE_CHECK_INTERVAL == 0) {
            int total_done = atomic_load(&balancer->global_sim_counter);
            printf("Thread %d: %d local sims, %d/%d global (%.1f%% complete)\n",
                   data->thread_id, local_sim_count, total_done, 
                   balancer->total_simulations,
                   (total_done * 100.0) / balancer->total_simulations);
        }
        */
    }
    
    struct timespec end_time;
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    /*
    double thread_elapsed = (end_time.tv_sec - start_time.tv_sec) + 
                           (end_time.tv_nsec - start_time.tv_nsec) / 1e9;
    printf("Thread %d finished: %d simulations in %.3f seconds (%.2f sims/s)\n",
           data->thread_id, local_sim_count, thread_elapsed, 
           local_sim_count / thread_elapsed);
    */
    
    pthread_exit(NULL);
}

void    run_mcts(game_t *game, int *res_x, int *res_y) {
    // Get number of CPU cores available
    int num_threads = (int)sysconf(_SC_NPROCESSORS_ONLN);
    if (num_threads <= 0 || num_threads > MAX_THREADS) {
        num_threads = 8; // Fallback to 8 threads if detection fails
    }
    
    printf("\n\e[1;33mDetected %d CPU cores, using %d threads for MCTS\e[0m\n", num_threads, num_threads);
    
    // Count empty cells on the board
    int empty_cells = 0;
    for (int i = 0; i < 19; i++) {
        for (int j = 0; j < 19; j++) {
            if (game->board[i][j] == 0) empty_cells++;
        }
    }
    
    node_t *root = init_root(game->current_player);
    if (!root)
        return;
    
    // Update root's nb_childs to match actual empty cells
    root->nb_childs = empty_cells;
    free(root->childs);
    if (!(root->childs = (node_t**)malloc(sizeof(node_t*) * root->nb_childs))) {
        free_node(root);
        return;
    }
    memset(root->childs, 0, sizeof(node_t*) * root->nb_childs);
    
    pthread_t threads[MAX_THREADS];
    thread_data_t thread_data[MAX_THREADS];
    load_balancer_t balancer;
    
    init_load_balancer(&balancer, NUM_SIMULATIONS, num_threads);
    
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    
    //printf("Running MCTS with %d simulations using %d threads...\n", NUM_SIMULATIONS, num_threads);
    for (int i = 0; i < num_threads; i++) {
        for (int m = 0; m < 19; m++) {
            for (int n = 0; n < 19; n++) {
                thread_data[i].board_sim[m][n] = game->board[m][n];
            }
        }
        
        thread_data[i].captured_black = game->captured_black;
        thread_data[i].captured_white = game->captured_white;
        thread_data[i].winning_state = 0;
        thread_data[i].thread_id = i;
        thread_data[i].node = root;
        thread_data[i].rand_seed = (unsigned int)time(NULL) + i * 1000;
        thread_data[i].balancer = &balancer;

        //printf("Thread %d initialized with dynamic load balancing.\n", i);
        if (pthread_create(&threads[i], NULL, mcts_thread_worker, &thread_data[i]) != 0) {
            fprintf(stderr, "Error creating thread %d\n", i);
            free_node(root);
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
    printf("\e[1;34m=== MCTS Performance Statistics ===\e[0m\n");
    printf("Total simulations: %d\n", NUM_SIMULATIONS);
    printf("Number of threads: %d\n", num_threads);
    printf("Total time: %.3f seconds\n", elapsed_seconds);
    printf("Simulations per second: %.2f\n", NUM_SIMULATIONS / elapsed_seconds);
    printf("Average time per simulation: %.6f seconds\n", elapsed_seconds / NUM_SIMULATIONS);
    printf("\e[1;34m===================================\e[0m\n\n");
    
    //print_load_balance_stats(&balancer);
    
    pthread_mutex_destroy(&balancer.balance_mutex);

    int best = 0;
    int i = 0;
    while (root->childs && root->childs[i] && i < root->nb_childs) {
        if (root->childs[i]->visit_count > root->childs[best]->visit_count)
            best = i;
        else if (root->childs[i]->visit_count == root->childs[best]->visit_count &&
                 root->childs[i]->value > root->childs[best]->value)
            best = i;
        i++;
    }
    
    if (root->childs && root->childs[best]) {
        printf("Best Move: (%d, %d) with %d visits and value %d\n", 
               root->childs[best]->x, root->childs[best]->y, 
               root->childs[best]->visit_count, root->childs[best]->value);
        *res_x = root->childs[best]->x;
        *res_y = root->childs[best]->y;
    } else {
        printf("No valid moves found.\n");
        *res_x = -1;
        *res_y = -1;
    }

    free_node(root);
}