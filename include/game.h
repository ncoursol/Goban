#ifndef GAME_H
#define GAME_H

#include "gomo.h"

typedef struct gomo_s gomo_t;

typedef struct moves_s {
    unsigned int x;
    unsigned int y;
    int player; // 0 for player 1, 1 for player 2
    unsigned int *captured_stones; // array of captured stones in this move
    unsigned int winning_state; // 0 = no win, 1 = player 1, 2 = player 2, 3 = both players
    struct moves_s *next;
    struct moves_s *prev;
    struct moves_s *first;
} moves_t;

typedef struct player_s {
    char name[50];
    unsigned int color; // 0 for black, 1 for white
    unsigned int is_human; // 1 if human, 0 if AI

} player_t;

typedef struct game_s {
    unsigned int board[19][19];     // current state of the game
    unsigned int current_player;    // 0 for player 1, 1 for player 2
    unsigned int swap2_player;      // -1 if no swap2, 0 if player 1, 1 if player 2
    unsigned int captured_black;
    unsigned int captured_white;
    unsigned int move_count;
    unsigned int swap2_step;        // 0: p1 place 3 stones, 1: p2 choose color, 2 : p2 place 2 stones, 3: p1 choose color
    moves_t     *moves;             // list of moves played
    player_t     players[2];        // player information
} game_t;

int init_game(game_t *game);
int place_stone(game_t *game, unsigned int x, unsigned int y);
void sync_game_state(gomo_t *gomo, game_t *game);
int check_double_free_three(game_t *game, unsigned int x, unsigned int y);
void pick_color(game_t *game, int color);

#endif