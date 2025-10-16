#include "mcts.h"

node_t  *init_root() {
    node_t *root = (node_t*)malloc(sizeof(node_t));
    
    if (!root)
        return (NULL);

    memset(root, 0, sizeof(node_t*));
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

node_t  *add_new_child(node_t *parent, int index, int x, int y, int **board_sim) {
	node_t	    *child;

	if (!(child = (node_t*)malloc(sizeof(node_t))))
		return (NULL);
	child->value = 0;
	child->visit_count = 0;
    child->uct = -1;
	child->parent = parent;
    child->x = x;
    child->y = y;
    child->color = parent->color == 1 ? 0 : 1;
    pthread_mutex_init(&child->mutex, NULL);  // Initialize mutex
    
    if (!(child->childs = (node_t**)malloc(sizeof(node_t*) * 19 * 19))) {
        free(child);
        return (NULL);
    }
    memset(child->childs, 0, 19 * 19);
    parent->childs[index] = child;
    return (child);
}

node_t    *selection(node_t *root, int **board_sim) {
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
        update_board(board_sim, root->x, root->y, root->color, 0);
    }
    return (root);
}

node_t    *expansion(node_t *cursor, int **board_sim) {
    int i = 0;
    int x = -1;
    int y = -1;

    while (cursor->childs[i] != NULL) {
        i++;
    }
    find_next_empty(board_sim, &x, &y, i);
    if (x == -1 || y == -1)
        return (cursor);
    update_board(board_sim, x, y, cursor->color == 1 ? 0 : 1, 0);
    return (add_new_child(cursor, i, x, y, board_sim));
}

void     simulation(node_t *cursor, int **board_sim) {
    int x = cursor->x;
    int y = cursor->y;
    int player = cursor->color;
    while (!game_is_over(board_sim)) {
        random_choice(board_sim, &x, &y, player);
        update_board(board_sim, x, y, player, 0);
        player = player == 1 ? 0 : 1;
    }
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