#ifndef BOT_CONFIG_H
#define BOT_CONFIG_H

#include "mcts.h"

// Bot configuration parameters
typedef struct bot_config_s
{
    char name[64];              // Bot name for display
    
    // MCTS parameters
    double time_limit;          // Time limit in seconds for MCTS (0 = use default)
    int max_children;           // Max children per node (branching factor)
    float exploration_constant; // UCT exploration constant (sqrt(2) = 1.414 default)
    
    // Heuristic weights
    float weight_capture;       // Weight for capture moves
    float weight_block_capture; // Weight for blocking opponent captures
    float weight_open_four;     // Weight for creating open four
    float weight_block_four;    // Weight for blocking opponent open four
    float weight_open_three;    // Weight for creating open three
    float weight_block_three;   // Weight for blocking opponent open three
    float weight_center;        // Weight for center distance
    float weight_win;           // Weight for winning move
    
    // Simulation parameters
    int simulation_depth_limit; // Max depth for random simulation
    
    // Threading
    int num_threads;            // 0 = auto-detect
} bot_config_t;

// Predefined bot configurations
extern bot_config_t BOT_DEFAULT;
extern bot_config_t BOT_AGGRESSIVE;
extern bot_config_t BOT_DEFENSIVE;
extern bot_config_t BOT_DEEP_SEARCH;
extern bot_config_t BOT_WIDE_SEARCH;

// Arena result structure
typedef struct arena_result_s
{
    int wins_bot1;
    int wins_bot2;
    int draws;
    int total_games;
    double avg_game_length;
    double avg_time_bot1;
    double avg_time_bot2;
} arena_result_t;

// Arena match structure
typedef struct arena_match_s
{
    bot_config_t *bot1;
    bot_config_t *bot2;
    int num_games;           // Total games to play
    int swap_colors;         // If true, play half games with swapped colors
    int verbose;             // Print game progress
    int save_games;          // Save game records to file
} arena_match_t;

// Functions
void init_bot_configs(void);
bot_config_t create_bot_config(const char *name);
void print_bot_config(bot_config_t *config);

// Run MCTS with specific bot configuration
void run_mcts_with_config(game_t *game, int *res_x, int *res_y, bot_config_t *config);

// Arena functions
arena_result_t run_arena_match(arena_match_t *match);
void print_arena_result(arena_result_t *result, bot_config_t *bot1, bot_config_t *bot2);

#endif
