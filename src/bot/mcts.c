#define _POSIX_C_SOURCE 199309L
#include "mcts.h"
#include <time.h>

node_t  *init_root(int player_color) {
    node_t *root = (node_t*)malloc(sizeof(node_t));
    
    if (!root)
        return (NULL);

    memset(root, 0, sizeof(node_t));
	root->value = 0;
	root->visit_count = 0;
    root->uct = -1;
    root->x = -1;
    root->y = -1;
    root->color = player_color;
    root->nb_childs = 19 * 19;
    root->childs = NULL;
    root->parent = NULL;
    pthread_mutex_init(&root->mutex, NULL);

    if (!(root->childs = (node_t**)malloc(sizeof(node_t*) * root->nb_childs))) {
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
	child->value = 0;
	child->visit_count = 0;
    child->uct = -1;
    child->x = x;
    child->y = y;
    child->color = parent->color;
    child->nb_childs = parent->nb_childs - 1;
    child->childs = NULL;
	child->parent = parent;
    pthread_mutex_init(&child->mutex, NULL);

    if (!(child->childs = (node_t**)malloc(sizeof(node_t*) * child->nb_childs))) {
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
    if (ret)
        *winning_state += current_player + 1;
    return 0;
}

void     random_choice(unsigned int board_sim[19][19], int *x, int *y)
{
    int rx, ry;
    rx = rand() % 19;
    ry = rand() % 19;
    for (int i = 0; i < 19 * 19; i++) {
        if (board_sim[rx + i / 19][ry + i % 19] == 0) {
            *x = rx + i / 19;
            *y = ry + i % 19;
            break;
        }
    }
    rx = -1;
    ry = -1;
}

node_t    *selection(node_t *root, unsigned int board_sim[19][19]) {
    int i;
    int best;
    float wi;
    while (root->childs[root->nb_childs - 1] && root->childs[0] != NULL) {
        best = 0;
        i = 0;
        while (root->childs[i] && i < root->nb_childs) {
            wi = ((float)root->childs[i]->visit_count / 2) + root->childs[i]->value;
            root->childs[i]->uct = (wi / root->childs[i]->visit_count) + sqrt(2) * sqrt(log(root->visit_count) / root->childs[i]->visit_count);
            if (root->childs[i]->uct > root->childs[best]->uct)
                best = i;
            i++;
        }
        root = root->childs[best];
        board_sim[root->x][root->y] = root->color == 1 ? 1 : 2;
    }
    return (root);
}

node_t    *expansion(node_t *cursor, unsigned int board_sim[19][19]) {
    int child_count = 0;
    int x = -1;
    int y = -1;

    while (cursor->childs && cursor->childs[child_count] != NULL) {
        child_count++;
    }
    
    find_next_empty(board_sim, &x, &y, child_count);

    if (x == -1 || y == -1)
        return (cursor);
    
    board_sim[x][y] = cursor->color == 1 ? 1 : 2;
    return (add_new_child(cursor, child_count, x, y));
}

int     simulation(node_t *cursor, thread_data_t *thread) {
    int x = cursor->x;
    int y = cursor->y;
    int player = cursor->color;
    int winner = 0;
    while (!winner) {
        random_choice(thread->board_sim, &x, &y);
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
    while (cursor->parent != NULL) {
        pthread_mutex_lock(&cursor->mutex);
        cursor->visit_count++;
        cursor->value += cursor->color == 1 ? -sim_result : sim_result;
        pthread_mutex_unlock(&cursor->mutex);
        cursor = cursor->parent;
    }
    pthread_mutex_lock(&cursor->mutex);
    cursor->visit_count++;
    pthread_mutex_unlock(&cursor->mutex);
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

void    *mcts_thread_worker(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    
    for (int i = 0; i < data->num_simulations; i++) {
        // Create a fresh copy of the board for each simulation
        //printf("Test\n");
        unsigned int local_board[19][19];
        for (int m = 0; m < 19; m++) {
            for (int n = 0; n < 19; n++) {
                local_board[m][n] = data->board_sim[m][n];
            }
        }
        
        //printf("Thread %d: Starting simulation %d/%d\n", data->thread_id, i + 1, data->num_simulations);
        node_t *selected = selection(data->node, local_board);
        //printf("Thread %d: Selected node at (%d, %d)\n", data->thread_id, selected->x, selected->y);
        node_t *expanded = expansion(selected, local_board);
        //printf("Thread %d: Expanded node at (%d, %d)\n", data->thread_id, expanded->x, expanded->y);
        
        // Create a local copy of thread data for simulation
        thread_data_t sim_data = *data;
        for (int m = 0; m < 19; m++) {
            for (int n = 0; n < 19; n++) {
                sim_data.board_sim[m][n] = local_board[m][n];
            }
        }
        
        int winner = simulation(expanded, &sim_data);
        //printf("Thread %d: Simulation result: %d\n", data->thread_id, winner);
        backPropagation(expanded, winner);
        //printf("Thread %d: Backpropagation complete for simulation %d/%d\n", data->thread_id, i + 1, data->num_simulations);
    }
    pthread_exit(NULL);
}

void    run_mcts(game_t *game) {
    node_t *root = init_root(game->swap2_player);
    if (!root)
        return;
    
    pthread_t threads[NUM_THREAD];
    thread_data_t thread_data[NUM_THREAD];
    int total_simulations = 100000;
    int sims_per_thread = total_simulations / NUM_THREAD;
    
    // Start timing
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    
    printf("Running MCTS with %d simulations using %d threads...\n", total_simulations, NUM_THREAD);
    for (int i = 0; i < NUM_THREAD; i++) {
        for (int m = 0; m < 19; m++) {
            for (int n = 0; n < 19; n++) {
                thread_data[i].board_sim[m][n] = game->board[m][n];
            }
        }
        
        thread_data[i].captured_black = game->captured_black;
        thread_data[i].captured_white = game->captured_white;
        thread_data[i].winning_state = 0;
        thread_data[i].thread_id = i;
        thread_data[i].num_simulations = sims_per_thread;
        thread_data[i].node = root;
        thread_data[i].results = NULL;

        printf("Thread %d initialized for %d simulations.\n", i, sims_per_thread);
        if (pthread_create(&threads[i], NULL, mcts_thread_worker, &thread_data[i]) != 0) {
            fprintf(stderr, "Error creating thread %d\n", i);
            free_node(root);
            return;
        }
    }
    
    for (int i = 0; i < NUM_THREAD; i++) {
        printf("Waiting for thread %d to finish...\n", i);
        pthread_join(threads[i], NULL);
    }

    // End timing
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    
    // Calculate elapsed time
    double elapsed_seconds = (end_time.tv_sec - start_time.tv_sec) + 
                            (end_time.tv_nsec - start_time.tv_nsec) / 1e9;
    
    printf("\n=== MCTS Performance Statistics ===\n");
    printf("Total simulations: %d\n", total_simulations);
    printf("Number of threads: %d\n", NUM_THREAD);
    printf("Total time: %.3f seconds\n", elapsed_seconds);
    printf("Simulations per second: %.2f\n", total_simulations / elapsed_seconds);
    printf("Average time per simulation: %.6f seconds\n", elapsed_seconds / total_simulations);
    printf("===================================\n\n");

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
    } else {
        printf("No valid moves found.\n");
    }

    free_node(root);
}