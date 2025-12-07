#ifndef DATA_GENERATION_H
#define DATA_GENERATION_H

#include "game.h"
#include "mcts.h"

// Structure to hold training data for one position
typedef struct training_sample_s {
    unsigned int board[19][19];          // Board state
    float policy[19][19];                 // MCTS visit count distribution (normalized)
    int current_player;                   // 0 or 1
    unsigned int captured_black;
    unsigned int captured_white;
    int move_number;
    int x;                                // Move played from this position
    int y;
    struct training_sample_s *next;
} training_sample_t;

// Structure to hold complete game data
typedef struct game_data_s {
    training_sample_t *samples;           // Linked list of all positions
    int num_samples;
    int winner;                           // 0 = player 1, 1 = player 2, 2 = draw
    unsigned int final_board[19][19];
    int total_moves;
    unsigned int final_captured_black;
    unsigned int final_captured_white;
} game_data_t;

// Initialize game data structure
game_data_t* init_game_data(void);

// Add a training sample to game data
void add_training_sample(
    game_data_t *data,
    unsigned int board[19][19],
    node_t *root,                         // MCTS root to extract policy from
    int current_player,
    unsigned int captured_black,
    unsigned int captured_white,
    int move_number,
    int x,
    int y
);

// Save game data to files
int save_game_data(game_data_t *data, const char *output_dir, int game_id);

// Free game data structure
void free_game_data(game_data_t *data);

// Generate N self-play games
int generate_training_games(int num_games, const char *output_dir, int num_threads);

#endif
