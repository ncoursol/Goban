#include "../include/game.h"

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

    return 1;
}

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

    if (check_double_free_three(game, x, y))
        return 0;

    return 1;
}

int *check_captures(game_t *game, unsigned int x, unsigned int y)
{
    int player_val = (game->current_player == 0) ? 1 : 2;
    int opponent_val = (game->current_player == 0) ? 2 : 1;
    int *captured_stones = NULL;
    int capture_count = 0;

    if (!(captured_stones = (int *)malloc(sizeof(int) * 16)))
        return NULL;

    int dx[8] = {-1, -1, 0, 1, 1, 1, 0, -1};
    int dy[8] = {0, 1, 1, 1, 0, -1, -1, -1};


    for (int d = 0; d < 8; d++) {
        if (game->board[x + 3 * dx[d]][y + 3 * dy[d]] == player_val &&
            game->board[x + 2 * dx[d]][y + 2 * dy[d]] == opponent_val &&
            game->board[x + 1 * dx[d]][y + 1 * dy[d]] == opponent_val) {
                captured_stones[capture_count++] = (x + 1 * dx[d]) * 19 + (y + 1 * dy[d]);
                captured_stones[capture_count++] = (x + 2 * dx[d]) * 19 + (y + 2 * dy[d]);
        }
    }

    if (capture_count == 0) {
        free(captured_stones);
        return NULL;
    }
    if (capture_count + 1 < 16 && !realloc(captured_stones, sizeof(int) * (capture_count + 1))) {
        free(captured_stones);
        return NULL;
    }

    return captured_stones;
}

int place_stone(game_t *game, unsigned int x, unsigned int y)
{
    if (!check_valid_move(game, x, y))
        return 0;

    int *captured_stones = check_captures(game, x, y);

    game->board[x][y] = game->current_player == 0 ? 1 : 2;

    game->moves[game->move_count].x = x;
    game->moves[game->move_count].y = y;
    game->moves[game->move_count].player = game->current_player;
    game->moves[game->move_count].captured_stones = captured_stones;
    game->move_count++;
    
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
}