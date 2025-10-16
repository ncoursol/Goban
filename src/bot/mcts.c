#include "mcts.h"

node_t  *init_root(void) {
    node_t *root = (node_t*)malloc(sizeof(node_t));
    
    if (!root)
        return (NULL);

    memset(root, 0, sizeof(node_t));
	root->value = 0;
	root->visit_count = 0;
    root->uct = -1;
    root->x = -1;
    root->y = -1;
    root->color = 1;
    root->childs = NULL;
    root->parent = NULL;
    pthread_mutex_init(&root->mutex, NULL);
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
    child->color = parent->color == 1 ? 0 : 1;
    child->childs = NULL;
	child->parent = parent;
    pthread_mutex_init(&child->mutex, NULL);
    
    if (!(child->childs = (node_t**)malloc(sizeof(node_t*) * 19 * 19))) {
        free(child);
        return (NULL);
    }
    memset(child->childs, 0, 19 * 19);
    parent->childs[index] = child;
    return (child);
}

void    find_next_empty(unsigned int board_sim[19][19], int *x, int *y) {
    for (int i = 0; i < 19; i++) {
        for (int j = 0; j < 19; j++) {
            if (board_sim[i][j] == 0) {
                *x = i;
                *y = j;
                return;
            }
        }
    }
    *x = -1;
    *y = -1; 
}

int check_win(unsigned int board_sim[19][19], unsigned int x, unsigned int y, int captured_black, int captured_white, int current_player, int winning_state) {
    int ret = 0;
    if (captured_black >= 10 || captured_white >= 10)
        return current_player + 1;

    if (winning_state > 2 || winning_state == !current_player + 1) {
        ret = check_five_in_a_row(board_sim, !current_player);
        if (ret)
            return !current_player + 1;
        else
            winning_state -= !current_player + 1;
    }
    ret = check_five_in_a_row_at(board_sim, x, y, current_player, 0);
    if (ret)
        winning_state += current_player + 1;
    return 0;
}

void     random_choice(unsigned int board_sim[19][19], int *x, int *y)
{
    int rx, ry;
    do {
        rx = rand() % 19;
        ry = rand() % 19;
    } while (board_sim[rx][ry] != 0);
    *x = rx;
    *y = ry;
}

node_t    *selection(node_t *root, unsigned int board_sim[19][19]) {
    int i;
    int best;
    float wi;
    while (root->childs[0] != NULL) {
        best = 0;
        i = 0;
        while (root->childs[i]) {
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
    int i = 0;
    int x = -1;
    int y = -1;

    while (cursor->childs[i] != NULL) {
        i++;
    }
    find_next_empty(board_sim, &x, &y);
    if (x == -1 || y == -1)
        return (cursor);
    board_sim[x][y] = cursor->color == 1 ? 1 : 2;
    return (add_new_child(cursor, i, x, y));
}

int     simulation(node_t *cursor, thread_data_t *thread) {
    int x = cursor->x;
    int y = cursor->y;
    int player = cursor->color;
    int winner = 0;
    while (!winner) {
        winner = check_win(thread->board_sim, x, y, thread->captured_black, thread->captured_white, player, thread->winning_state);
        random_choice(thread->board_sim, &x, &y);
        thread->board_sim[x][y] = player == 1 ? 1 : 2;
        player = player == 1 ? 0 : 1;
    }
    return (winner - 1);
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
        for (int i = 0; node->childs[i] != NULL; i++) {
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
        node_t *selected = selection(data->node, data->board_sim);
        node_t *expanded = expansion(selected, data->board_sim);

        int winner = simulation(expanded, data) - 1;
        backPropagation(expanded, winner);

        // Reset board for next simulation
        // Note: You might need to reinitialize board_sim here
        // depending on your init_board_sim implementation
    }
    
    pthread_exit(NULL);
}

void    

void    run_mcts(game_t *game) {
    node_t *root = init_root();
    if (!root)
        return;
    
    pthread_t threads[NUM_THREAD];
    thread_data_t thread_data[NUM_THREAD];
    int total_simulations = 1000;
    int sims_per_thread = total_simulations / NUM_THREAD;
    
    for (int i = 0; i < NUM_THREAD; i++) {
        thread_data[i].board_sim = init_board_sim(game->board);
        if (!thread_data[i].board_sim) {
            for (int j = 0; j < i; j++) {
                free_board_sim(thread_data[j].board_sim);
            }
            free_node(root);
            return;
        }
        
        thread_data[i].captured_black = game->captured_black;
        thread_data[i].captured_white = game->captured_white;
        thread_data[i].winning_state = 0;
        thread_data[i].thread_id = i;
        thread_data[i].num_simulations = sims_per_thread;
        thread_data[i].node = root;
        thread_data[i].results = NULL;

        if (pthread_create(&threads[i], NULL, mcts_thread_worker, &thread_data[i]) != 0) {
            fprintf(stderr, "Error creating thread %d\n", i);
            for (int j = 0; j <= i; j++) {
                free_board_sim(thread_data[j].board_sim);
            }
            free_node(root);
            return;
        }
    }
    
    for (int i = 0; i < NUM_THREAD; i++) {
        pthread_join(threads[i], NULL);
        free_board_sim(thread_data[i].board_sim);
    }

    int best = 0;
    for (int i = 1; root->childs && root->childs[i] != NULL; i++) {
        if (root->childs[i]->visit_count > root->childs[best]->visit_count)
            best = i;
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