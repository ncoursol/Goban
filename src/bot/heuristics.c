#include "mcts.h"
//#include <stdio.h>
//#include <string.h>
//#include <stdlib.h>

void create_captures(unsigned int board[19][19], unsigned int x, unsigned int y, int player, int *p_cap, int *o_cap)
{
    unsigned int player_val = (player == 0) ? 1 : 2;
    unsigned int opponent_val = (player == 0) ? 2 : 1;

    int dx[8] = {-1, -1, 0, 1, 1, 1, 0, -1};
    int dy[8] = {0, 1, 1, 1, 0, -1, -1, -1};

    for (int d = 0; d < 8; d++) {
        if (x + 3 * dx[d] >= 0 && x + 3 * dx[d] < 19 && y + 3 * dy[d] >= 0 && y + 3 * dy[d] < 19) {
            if (board[x + 3 * dx[d]][y + 3 * dy[d]] == player_val &&
                board[x + 2 * dx[d]][y + 2 * dy[d]] == opponent_val &&
                board[x + 1 * dx[d]][y + 1 * dy[d]] == opponent_val) {
                    *p_cap += 2;
            } else if (board[x + 3 * dx[d]][y + 3 * dy[d]] == opponent_val &&
                board[x + 2 * dx[d]][y + 2 * dy[d]] == player_val &&
                board[x + 1 * dx[d]][y + 1 * dy[d]] == player_val) {
                    *o_cap += 2;
            }
        }
    }
}

void create_opens(unsigned int board[19][19], unsigned int x, unsigned int y, int player, int *p_three, int *o_three, int *p_four, int *o_four) {
    unsigned int player_val = (player == 0) ? 1 : 2;
    unsigned int opponent_val = (player == 0) ? 2 : 1;

    int dx[4] = {-1, -1, 0, 1};
    int dy[4] = {0, 1, 1, 1};

    int p_res = 0;
    int o_res = 0;

    for (int d = 0; d < 4; d++) {
        p_res = 1;
        o_res = 1;
        for (int i = -4; i < 4; i++) {
            if (x + i * dx[d] >= 0 && x + i * dx[d] < 19 && y + i * dy[d] >= 0 && y + i * dy[d] < 19) {
                if (x + i * dx[d] == x && y + i * dy[d] == y) {
                    p_res = p_res > 0 ? p_res + 1 : 0;
                    o_res = o_res > 0 ? o_res + 1 : 0;
                } else if (board[x + i * dx[d]][y + i * dy[d]] == 0) {
                    if (p_res == 5)
                        (*p_four)++;
                    if (o_res == 5)
                        (*o_four)++;
                    if (p_res == 4)
                        (*p_three)++;
                    if (o_res == 4)
                        (*o_three)++;
                    p_res = 1;
                    o_res = 1;
                } else if (board[x + i * dx[d]][y + i * dy[d]] == player_val) {
                    if (p_res > 0)
                        p_res++;
                    o_res = 0;
                } else if (board[x + i * dx[d]][y + i * dy[d]] == opponent_val) {
                    if (o_res > 0)
                        o_res++;
                    p_res = 0;
                }
            }
        }
    }
}

int get_move_score(unsigned int board[19][19], int x, int y, int player)
{
    int score = 0;
    int player_val = 0, opponent_val = 0;
    int p_three = 0, o_three = 0, p_four = 0, o_four = 0;

    if (check_five_in_a_row_at(board, x, y, player, 0))
        return 10000;

    create_opens(board, x, y, player, &p_three, &o_three, &p_four, &o_four);

    // if create open four
    if (p_four > 0)
        score += 40 * p_four;
    // if block opponent open four
    if (o_four > 0)
        score += 30 * o_four;

    // if create open three
    if (p_three > 0)
        score += 6;
    // if block opponent open three
    if (o_three > 0)
        score += 4;
    
    create_captures(board, x, y, player, &player_val, &opponent_val);

    // if allow a capture
    if (player_val > 0)
        score += 10 * (player_val / 2);
    // if prevent opponent capture
    if (opponent_val > 0)
        score += 8 * (opponent_val / 2);

    return score;
}

int get_distance_to_center(int x, int y) {
    int center_x = 9;
    int center_y = 9;
    return (abs(center_x - x) + abs(center_y - y));
}

float *weightmap(unsigned int board[19][19], int *valid_moves, int max_size, int player) {
    float *weightmap = (float *)malloc(sizeof(float) * max_size);
    if (!weightmap) {
        perror("Failed to allocate memory for weightmap");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < max_size; i++)
        weightmap[i] = 0.0f;

    for (int i = 0; i < max_size; i++) {
        int x = valid_moves[i] / 19;
        int y = valid_moves[i] % 19;
        if (board[x][y] == 0) {
            float dist_term = 1.0f / (1.0f + get_distance_to_center(x, y) * 0.1f);
            float score_term = (float)get_move_score(board, x, y, player) / 10.0f;

            weightmap[i] = dist_term + score_term;

            if (weightmap[i] < 0.0f)
                weightmap[i] = 0.0f;
        } else {
            weightmap[i] = 0.0f;
        }
    }

    return weightmap;
}

int select_weighted_move(unsigned int board[19][19], int *valid_moves, int max_size, int player) {
    float *w = weightmap(board, valid_moves, max_size, player);
    float total = 0.0f;
    for (int i = 0; i < max_size; i++)
        total += w[i];

    int chosen_move = -1;
    if (total <= 0.0f) {
        /* fallback: uniform random among legal moves */
        int legal_count = 0;
        for (int i = 0; i < max_size; i++) {
            int x = valid_moves[i] / 19;
            int y = valid_moves[i] % 19;
            if (board[x][y] == 0) legal_count++;
        }

        if (legal_count == 0) {
            free(w);
            return -1;
        }
        
        int r = rand() % legal_count;
        for (int i = 0; i < max_size; i++) {
            int x = valid_moves[i] / 19;
            int y = valid_moves[i] % 19;
            if (board[x][y] == 0) {
                if (r == 0) { chosen_move = valid_moves[i]; break; }
                r--;
            }
        }
        free(w);
        return chosen_move;
    }

    float r = ((float)rand() / (float)RAND_MAX) * total;
    float accum = 0.0f;
    for (int i = 0; i < max_size; i++) {
        accum += w[i];
        if (r <= accum) {
            chosen_move = valid_moves[i];
            break;
        }
    }

    /* safety fallback (pick last positive weight) */
    if (chosen_move == -1) {
        for (int i = max_size - 1; i >= 0; i--) {
            if (w[i] > 0.0f) { chosen_move = valid_moves[i]; break; }
        }
    }

    free(w);
    return chosen_move;
}
/*
int get_nb_empty_positions(unsigned int board[19][19], int x, int y, int valid_moves[361])
{
    int count = 0;
    for (int i = 0; i < 19; i++) {
        for (int j = 0; j < 19; j++) {
            if (i == x && j == y) {
                continue;
            }
            if (board[i][j] == 0) {
                valid_moves[count] = i * 19 + j;
                count++;
            }
        }
    }
    return count;
}

void print_board(unsigned int board[19][19])
{
    for (int i = 0; i < 19; i++) {
        for (int j = 0; j < 19; j++) {
            printf("%c ", board[i][j] == 0 ? '.' : board[i][j] == 1 ? '1' : '2');
        }
        printf("\n");
    }
}

int main(void)
{
    unsigned int board[19][19] = {0};
    memset(board, 0, sizeof(board));
    int player = 0; // 0 for player 1, 1 for player 2
    board[9][10] = 2;
    board[8][10] = 2;
    board[7][10] = 1;

    board[9][9] = 2;
    board[8][8] = 2;

    board[10][9] = 1;
    board[10][8] = 1;
    board[10][7] = 1;

    board[9][11] = 1;
    board[8][12] = 1;
    board[7][13] = 2;


    print_board(board);
    // print weightmap
    int valid_moves[361];
    int max_size = get_nb_empty_positions(board, -1, -1, valid_moves);
    float *weightmap_values = weightmap(board, valid_moves, max_size, player);
    printf("Weightmap values:\n");
    for (int i = 0; i < 19; i++) {
        for (int j = 0; j < 19; j++) {
            int found = 0;
            for (int k = 0; k < max_size; k++) {
                if (valid_moves[k] == i * 19 + j) {
                    printf("%6.2f ", weightmap_values[k]);
                    found = 1;
                    break;
                }
            }
            if (!found) {
                printf("    *  ");
            }
        }
        printf("\n");
    }
    free(weightmap_values);

    int move = select_weighted_move(board, valid_moves, max_size, player);
    printf("get new random pos: (%d, %d) with a chance of: %.4f\n", move / 19, move % 19, weightmap_values[move]);

    return 0;
}
*/