#define _POSIX_C_SOURCE 199309L
#include "mcts.h"
#include <time.h>
#include <math.h>

/* helper to compute seconds between two timespecs (file-scope) */
static inline double timespec_diff_secs(const struct timespec *a, const struct timespec *b)
{
    return (double)(b->tv_sec - a->tv_sec) + (double)(b->tv_nsec - a->tv_nsec) / 1e9;
}

/* simple timer wrapper for cleaner code in run_mcts */
typedef struct {
    struct timespec start;
} mcts_timer_t;

static inline void mcts_timer_start(mcts_timer_t *t)
{
    clock_gettime(CLOCK_MONOTONIC, &t->start);
}

static inline double mcts_timer_elapsed(mcts_timer_t *t)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return timespec_diff_secs(&t->start, &now);
}

int get_nb_empty_positions(unsigned int board[19][19], int x, int y, int valid_moves[361])
{
    int count = 0;
    for (int i = 0; i < 19; i++) {
        for (int j = 0; j < 19; j++) {
            if (i == x && j == y) {
                continue;
            }
            if (board[i][j] == 0 && !check_double_free_three(board, i, j, 0)) {
                valid_moves[count] = i * 19 + j;
                count++;
            }
        }
    }
    return count;
}

node_t *create_node(int x, int y, node_t *parent, unsigned int board[19][19], int parent_player)
{
    node_t *node = (node_t *)malloc(sizeof(node_t));
    if (!node)
        return NULL;

    int max_childs = get_nb_empty_positions(board, x, y, node->valid_moves);

    node->value = 0;
    node->visit_count = 0;
    node->uct = 0.0f;
    node->x = x;
    node->y = y;
    node->nb_childs = 0;
    node->max_childs = max_childs;
    node->player = !parent_player;
    node->parent = parent;
    node->childs = (node_t **)malloc(sizeof(node_t *) * max_childs);
    if (!node->childs) {
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
    if (mcts->board[node->valid_moves[node->nb_childs] / 19][node->valid_moves[node->nb_childs] % 19] == 0) {
        *res_x = node->valid_moves[node->nb_childs] / 19;
        *res_y = node->valid_moves[node->nb_childs] % 19;
        return;
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
    
    float exploitation = (float)node->value / (float)node->visit_count;
    float exploration = sqrtf(2.0f) * sqrtf(logf((float)node->parent->visit_count) / (float)node->visit_count);
    
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
    if (x == -1 || y == -1)
        return NULL;

    node_t *child = create_node(x, y, node, mcts->board, node->player);
    if (!child)
        return NULL;

    node->childs[node->nb_childs] = child;
    node->nb_childs++;

    mcts->board[x][y] = child->player + 1;
    remove_captured_stones(mcts->board, check_captures(mcts->board, x, y, child->player, &mcts->captured_black, &mcts->captured_white));

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

        int move = -1;
        
        // Use heuristics only for first 3-5 moves of simulation, then pure random
        if (depth < 3 && (rand() % 100) < 70) { // 70% chance to use heuristics in early moves
            move = select_weighted_move(sim_mcts.board, sim_node.valid_moves, sim_node.max_childs, sim_node.player);
        }
        
        // Fallback to fast random
        if (move == -1) {
            int x, y;
            get_random_move(&sim_node, &sim_mcts, &x, &y);
            move = x * 19 + y;
        }
        
        sim_node.x = move / 19;
        sim_node.y = move % 19;

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
        node->visit_count++;
        node->value += (node->player == result) ? 1 : -1;
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

/* Display timing and selected move results for run_mcts */
static void display_mcts_results(int *res_x, int *res_y, double elapsed_time, double total[4], int num_simulations)
{
    double sel_avg = num_simulations ? total[0] / num_simulations : 0.0;
    double exp_avg = num_simulations ? total[1] / num_simulations : 0.0;
    double sim_avg = num_simulations ? total[2] / num_simulations : 0.0;
    double back_avg = num_simulations ? total[3] / num_simulations : 0.0;

    printf("MCTS selected move (%d, %d) after %d simulations.\n", *res_x, *res_y, num_simulations);
    printf("Total MCTS elapsed time: %.2f seconds\n", elapsed_time);
    printf("Time statistics:\n");
    printf("  Selection: %.2f seconds (avg: %.4f s/sim)\n", total[0], sel_avg);
    printf("  Expansion: %.2f seconds (avg: %.4f s/sim)\n", total[1], exp_avg);
    printf("  Simulation: %.2f seconds (avg: %.4f s/sim)\n", total[2], sim_avg);
    printf("  Backpropagation: %.2f seconds (avg: %.4f s/sim)\n", total[3], back_avg);
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

void run_mcts(game_t *game, int *res_x, int *res_y) {
    mcts_t mcts = init_mcts(game);
    node_t *root = create_node(-1, -1, NULL, game->board, game->current_player);
    
    if (!root) {
        *res_x = -1;
        *res_y = -1;
        return;
    }
    
    node_t *node = root;
    mcts_t sim_mcts;
    int sim_result = 0;

    mcts_timer_t total_timer, phase_timer;

    double total[4] = {0.0, 0.0, 0.0, 0.0};

    mcts_timer_start(&total_timer);
    for (int i = 0; i < NUM_SIMULATIONS; i++) {
        node = root;
        memcpy(&sim_mcts, &mcts, sizeof(mcts_t));

        mcts_timer_start(&phase_timer);
        node = selection(node, &sim_mcts);
        total[0] += mcts_timer_elapsed(&phase_timer);

        if (node->nb_childs != -1) {
            mcts_timer_start(&phase_timer);
            node = expansion(node, &sim_mcts);
            total[1] += mcts_timer_elapsed(&phase_timer);
            if (!node) {
                printf("No valid moves available for expansion. Skipping simulation.\n");
                continue;
            }
            mcts_timer_start(&phase_timer);
            sim_result = simulation(node, sim_mcts);
            total[2] += mcts_timer_elapsed(&phase_timer);
        } else {
            sim_result = -node->nb_childs - 1;
        }
        mcts_timer_start(&phase_timer);
        backpropagation(node, sim_result);
        total[3] += mcts_timer_elapsed(&phase_timer);
    }

    double elapsed_time = mcts_timer_elapsed(&total_timer);

    get_best_move(root, res_x, res_y);
    //print_mcts_results(root, res_x, res_y);
    display_mcts_results(res_x, res_y, elapsed_time, total, NUM_SIMULATIONS);
    free_tree(root);
}