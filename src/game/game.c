#include "../include/game.h"

void sync_game_state(gomo_t *gomo, game_t *game)
{
    for (int i = 0; i < 19; i++) {
        for (int j = 0; j < 19; j++) {
            gomo->board[i * 19 + j].state = game->board[i][j] ? 1 : 0;
            gomo->board[i * 19 + j].color = (game->board[i][j] == 1) ? (vec3_t){0.0f, 0.0f, 0.0f} : (game->board[i][j] == 2) ? (vec3_t){1.0f, 1.0f, 1.0f} : (vec3_t){1.0f, 0.0f, 1.0f};
        }
    }
}

int check_double_free_three(game_t *game, unsigned int x, unsigned int y)
{
    int free_three_count = 0;

    /* per-direction state: 0=horizontal,1=vertical,2=diag 1,3=diag 2 */
    int empty[4] = {0,0,0,0};
    int count[4] = {0,0,0,0};
    int last_case[4] = {-1,-1,-1,-1};
    int dx[4] = {1, 0, 1, 1};
    int dy[4] = {0, 1, 1, -1};

    int player_val = (game->current_player == 0) ? 1 : 2;

    for (int i = -4; i <= 4; i++) {
        if (free_three_count > 1)
            return 1;

        for (int d = 0; d < 4; d++) {
            int xi = (int)x + i * dx[d];
            int yi = (int)y + i * dy[d];

            if (xi < 0 || xi >= 19 || yi < 0 || yi >= 19)
                continue;

            int cell = game->board[xi][yi];

            if (cell == 0) {
                empty[d]++;
                last_case[d] = 0;
            } else if (cell == player_val || i == 0) {
                count[d]++;
                last_case[d] = 1;
            } else {
                empty[d] = 0;
                count[d] = 0;
                last_case[d] = -1;
            }

            if (empty[d] > 3 || count[d] > 3) {
                empty[d] = 0;
                count[d] = 0;
            } else if (count[d] == 1 && empty[d] == 0) {
                count[d] = 0;
            } else if (empty[d] == 2 && last_case[d] == 0) {
                empty[d] = 1;
            } else if (count[d] == 3 && empty[d] >= 2 && last_case[d] == 0) {
                if (++free_three_count > 1)
                    return 1;
            }
        }
    }

    return 0;
}


int check_valid_move(game_t *game, unsigned int x, unsigned int y)
{
    if (x >= 19 || y >= 19 || game->board[x][y] != 0)
        return 0;

    /*
    if (check_double_free_three(game, x, y))
        return 0;
    */
    return 1;
}

unsigned int *check_captures(game_t *game, unsigned int x, unsigned int y)
{
    unsigned int player_val = (game->current_player == 0) ? 1 : 2;
    unsigned int opponent_val = (game->current_player == 0) ? 2 : 1;
    unsigned int *captured_stones = NULL;
    unsigned int capture_count = 0;

    if (!(captured_stones = (unsigned int *)malloc(sizeof(unsigned int) * 16)))
        return NULL;

    int dx[8] = {-1, -1, 0, 1, 1, 1, 0, -1};
    int dy[8] = {0, 1, 1, 1, 0, -1, -1, -1};

    for (int d = 0; d < 8; d++) {
        if (x + 3 * dx[d] >= 0 && x + 3 * dx[d] < 19 && y + 3 * dy[d] >= 0 && y + 3 * dy[d] < 19) {
            if (game->board[x + 3 * dx[d]][y + 3 * dy[d]] == player_val &&
                game->board[x + 2 * dx[d]][y + 2 * dy[d]] == opponent_val &&
                game->board[x + 1 * dx[d]][y + 1 * dy[d]] == opponent_val) {
                    captured_stones[capture_count++] = (x + 1 * dx[d]) * 19 + (y + 1 * dy[d]);
                    captured_stones[capture_count++] = (x + 2 * dx[d]) * 19 + (y + 2 * dy[d]);
            }
        }
    }

    if (capture_count == 0) {
        free(captured_stones);
        return NULL;
    }

    if (!realloc(captured_stones, sizeof(unsigned int) * (capture_count + 1))) {
        free(captured_stones);
        return NULL;
    }

    if (game->current_player)
        game->captured_white += capture_count;
    else
        game->captured_black += capture_count;

    return captured_stones;
}

void remove_captured_stones(game_t *game, unsigned int *captured_stones) {
    int i = 0;
    int index = 0;

    while (captured_stones && captured_stones[i]) {
        index = captured_stones[i] / 19;
        if (game->board[index][captured_stones[i] - 19 * index])
            game->board[index][captured_stones[i] - 19 * index] = 0;
        i++;
    }
}

int check_five_in_a_row_at(game_t *game, unsigned int x, unsigned int y, int preview_play)
{
    unsigned int player_val = (game->current_player == 0) ? 1 : 2;
    unsigned int count = 0;

    int directions[4][2] = {
        {1, 0},  // horizontal
        {0, 1},  // vertical
        {1, 1},  // diagonal1
        {1, -1}  // diagonal2
    };

    for (int d = 0; d < 4; d++) {
        count = game->board[x][y] == player_val || preview_play ? 1 : preview_play ? 1 : 0;

        for (int step = 1; step < 5; step++) {
            int nx = (int)x + step * directions[d][0];
            int ny = (int)y + step * directions[d][1];
            if (nx >= 0 && nx < 19 && ny >= 0 && ny < 19 && game->board[nx][ny] == player_val) {
                count++;
            } else {
                break;
            }
        }

        for (int step = 1; step < 5; step++) {
            int nx = (int)x - step * directions[d][0];
            int ny = (int)y - step * directions[d][1];
            if (nx >= 0 && nx < 19 && ny >= 0 && ny < 19 && game->board[nx][ny] == player_val) {
                count++;
            } else {
                break;
            }
        }

        if (count >= 5) {
            return 1;
        }
    }

    return 0;
}

// code made by me - 6465 ns/call (avg. 1000000 calls)
int check_five_in_a_row(game_t *game)
{
    unsigned int player_val = (game->current_player == 0) ? 1 : 2;
    unsigned int tmp[3][15]; // 15 bulks of 5 consecutives rows * 3 directions
    unsigned int rows[19];
    int index = 0, index2 = 0;
    int check_h = 0;

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 15; j++) {
            tmp[i][j] = 0;
        }
    }
    
    for (int i = 0; i < 19; i++) {
        rows[i] = 0;
        for (int j = 0; j < 19; j++) {
            if (game->board[i][j] == player_val)
                check_h++;
            else
                check_h = 0;

            if (check_h == 5)
                return 1;
            
            rows[i] |= (game->board[i][j] == player_val) << (18 - j);
        }
        check_h = 0;
        for (int x = 0; x < 3; x++) {
            index = (i < 4) ? i + 1 : i < 15 ? i + 1 : 15;
            for (int k = 0; k < index; k++) {
                index2 = (i - k) + 1 < 5 ? (i - k) + 1 : 5;
                if (!tmp[x][k] && index2 == 1)
                    tmp[x][k] = rows[i];
                else if (tmp[x][k]) {
                    if (x == 0) {
                        tmp[x][k] &= rows[i]; // vertical
                    } else if (x == 1) {
                        tmp[x][k] = (tmp[x][k] >> 1) & rows[i]; // diagonal 1
                    } else if (x == 2) {
                        tmp[x][k] = (tmp[x][k] << 1) & rows[i]; // diagonal 2
                    }
                }
                if (tmp[x][k] && index2 == 5) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

/*
// Code made with AI - 12157 ns/call (avg. 1000000 calls)
#include <stdbool.h>

#define BOARD_SIZE 19
#define WIN_COUNT 5

int check_five_in_a_row(game_t *game) {
    unsigned int player = game->current_player + 1; // Map 0→1, 1→2

    // Direction vectors: {dx, dy}
    const int directions[4][2] = {
        {0, 1},  // Horizontal →
        {1, 0},  // Vertical ↓
        {1, 1},  // Diagonal ↘
        {-1, 1}  // Diagonal ↗
    };

    for (int row = 0; row < BOARD_SIZE; ++row) {
        for (int col = 0; col < BOARD_SIZE; ++col) {
            // Only check cells with current player's stone
            if (game->board[row][col] != player)
                continue;

            // Check in all 4 directions
            for (int d = 0; d < 4; ++d) {
                int count = 1;
                int dx = directions[d][0];
                int dy = directions[d][1];

                // Step forward
                int r = row + dx;
                int c = col + dy;
                while (r >= 0 && r < BOARD_SIZE && c >= 0 && c < BOARD_SIZE &&
                       game->board[r][c] == player) {
                    count++;
                    if (count == WIN_COUNT)
                        return 1;
                    r += dx;
                    c += dy;
                }
            }
        }
    }

    return 0;
}*/

int check_win(game_t *game) {
    if (game->captured_black >= 10 || game->captured_white >= 10)
        return 2;

    int ret = 0;
    if (!game->current_player)
        ret = check_five_in_a_row(game);

    return ret;
}

void print_board(game_t *game) {
    for (int i = 0; i < 19; i++) {
        for (int j = 0; j < 19; j++) {
            printf("%c ", game->board[i][j] == 0 ? '.' : game->board[i][j] == 1 ? '1' : '2');
        }
        printf("\n");
    }
}

int place_stone(game_t *game, unsigned int x, unsigned int y)
{
    if (!check_valid_move(game, x, y))
        return 0;

    unsigned int *captured_stones = check_captures(game, x, y);
    if (captured_stones != NULL)
        printf("cs: [%d, %d]\n", captured_stones[0], captured_stones[1]);

    game->board[x][y] = game->current_player == 0 ? 1 : 2;

    game->moves[game->move_count].x = x;
    game->moves[game->move_count].y = y;
    game->moves[game->move_count].player = game->current_player;
    game->moves[game->move_count].captured_stones = captured_stones;
    game->move_count++;
    
    if (captured_stones != NULL)
        remove_captured_stones(game, captured_stones);

    if (check_win(game))
        return 2;

    if (game->swap2_step == 0 && game->move_count == 3) {
        game->swap2_step = 1;
        game->current_player = 1 - game->current_player;
    } else if (game->swap2_step == 2 && game->move_count == 5) {
        game->swap2_step = 3;
        game->current_player = 1 - game->current_player;
    } else {
        game->current_player = 1 - game->current_player;
    }
    return 1;

    print_board(game);
}


int init_game(game_t *game)
{
    memset(game, 0, sizeof(game_t));

    for (int i = 0; i < 19; i++) {
        for (int j = 0; j < 19; j++) {
            game->board[i][j] = 0;
        }
    }
    game->current_player = 0;
    game->captured_black = 0;
    game->captured_white = 0;
    game->move_count = 0;
    game->swap2_step = 0;

    if (!(game->moves = (moves_t *)malloc(sizeof(moves_t) * 19 * 19)))
        return 0;
    memset(game->moves, 0, sizeof(moves_t) * 19 * 19);

    for (int i = 0; i < 19 * 19; i++) {
        game->moves[i].x = 0;
        game->moves[i].y = 0;
        game->moves[i].player = 0;
        game->moves[i].captured_stones = NULL;
    }

    for (int i = 0; i < 2; i++) {
        snprintf(game->players[i].name, sizeof(game->players[i].name), "Player %d", i + 1);
        game->players[i].is_human = 1;
    }

    /*
    for (int i = 0; i < 19; i++) {
        for (int j = 0; j < 19; j++) {
            if (j % 5 && i % 5)
                game->board[i][j] = 1; // star points
        }
    }*/

    return 1;
}