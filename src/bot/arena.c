#include "bot_config.h"
#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Play a single game between two bots
// Returns: 1 = bot1 wins, 2 = bot2 wins, 0 = draw
static int play_single_game(bot_config_t *bot1, bot_config_t *bot2, 
                            int *game_length, double *time_bot1, double *time_bot2,
                            int verbose)
{
    game_t game;
    init_game(&game, 2);  // Mode 2 = EvE (bot vs bot)
    
    // Disable swap2 rules for arena - bots play standard Gomoku
    game.swap2_step = 5;  // Skip all swap2 logic
    
    *game_length = 0;
    *time_bot1 = 0.0;
    *time_bot2 = 0.0;
    
    int max_moves = 361;  // Max possible moves on 19x19
    int move_count = 0;
    
    while (move_count < max_moves) {
        int x, y;
        struct timespec start, end;
        
        bot_config_t *current_bot = (game.current_player == 0) ? bot1 : bot2;
        double *current_time = (game.current_player == 0) ? time_bot1 : time_bot2;
        
        clock_gettime(CLOCK_MONOTONIC, &start);
        run_mcts_with_config(&game, &x, &y, current_bot);
        clock_gettime(CLOCK_MONOTONIC, &end);
        
        double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
        *current_time += elapsed;
        
        if (x == -1 || y == -1) {
            // No valid move - draw
            printf("  No valid move found!\n");
            *game_length = move_count;
            return 0;
        }
        
        if (verbose) {
            printf("  Move %d: %s plays (%d, %d) [%.2fs]\n", 
                   move_count + 1, current_bot->name, x, y, elapsed);
            fflush(stdout);
        }
        
        int winner = place_stone(&game, x, y);
        
        // Check if move was actually placed (0 = failed, non-zero = success or win)
        if (winner == 0 && game.board[x][y] == 0) {
            // Move failed - shouldn't happen if MCTS is working correctly
            printf("  ERROR: Move (%d, %d) was invalid!\n", x, y);
            *game_length = move_count;
            return 0;
        }
        
        move_count++;
        
        if (winner != 0) {
            *game_length = move_count;
            if (verbose) {
                printf("  Winner: %s (%d moves)\n\n", 
                       (winner == 1) ? bot1->name : bot2->name, move_count);
            }
            return winner;
        }
    }
    
    *game_length = move_count;
    return 0;  // Draw
}

arena_result_t run_arena_match(arena_match_t *match)
{
    arena_result_t result = {0};
    result.total_games = match->num_games;
    
    double total_game_length = 0;
    double total_time_bot1 = 0;
    double total_time_bot2 = 0;
    
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                    ARENA MATCH START                       ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║  %-20s  vs  %-20s         ║\n", match->bot1->name, match->bot2->name);
    printf("║  Games: %-3d          Swap colors: %-3s                    ║\n", 
           match->num_games, match->swap_colors ? "Yes" : "No");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    for (int g = 0; g < match->num_games; g++) {
        int game_length;
        double time_bot1, time_bot2;
        
        // Determine which bot plays first
        bot_config_t *first_bot = match->bot1;
        bot_config_t *second_bot = match->bot2;
        
        // Swap colors for half the games if enabled
        int swapped = 0;
        if (match->swap_colors && g >= match->num_games / 2) {
            first_bot = match->bot2;
            second_bot = match->bot1;
            swapped = 1;
        }
        
        printf("Game %d/%d: %s (Black) vs %s (White)%s\n", 
               g + 1, match->num_games,
               first_bot->name, second_bot->name,
               swapped ? " [swapped]" : "");
        
        int winner = play_single_game(first_bot, second_bot, 
                                       &game_length, &time_bot1, &time_bot2,
                                       match->verbose);
        
        total_game_length += game_length;
        
        // Map winner back to bot1/bot2
        if (winner == 0) {
            result.draws++;
            printf("  Result: Draw (%d moves)\n", game_length);
        } else {
            int actual_winner;
            if (swapped) {
                actual_winner = (winner == 1) ? 2 : 1;
                total_time_bot1 += time_bot2;  // Bot1 was second
                total_time_bot2 += time_bot1;  // Bot2 was first
            } else {
                actual_winner = winner;
                total_time_bot1 += time_bot1;
                total_time_bot2 += time_bot2;
            }
            
            if (actual_winner == 1) {
                result.wins_bot1++;
                printf("  Result: %s wins (%d moves)\n", match->bot1->name, game_length);
            } else {
                result.wins_bot2++;
                printf("  Result: %s wins (%d moves)\n", match->bot2->name, game_length);
            }
        }
        printf("\n");
    }
    
    result.avg_game_length = total_game_length / match->num_games;
    result.avg_time_bot1 = total_time_bot1 / match->num_games;
    result.avg_time_bot2 = total_time_bot2 / match->num_games;
    
    return result;
}

void print_arena_result(arena_result_t *result, bot_config_t *bot1, bot_config_t *bot2)
{
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                    ARENA RESULTS                           ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║  %-20s: %3d wins (%5.1f%%)                   ║\n", 
           bot1->name, result->wins_bot1, 
           100.0 * result->wins_bot1 / result->total_games);
    printf("║  %-20s: %3d wins (%5.1f%%)                   ║\n", 
           bot2->name, result->wins_bot2,
           100.0 * result->wins_bot2 / result->total_games);
    printf("║  Draws              : %3d      (%5.1f%%)                   ║\n", 
           result->draws,
           100.0 * result->draws / result->total_games);
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║  Avg game length: %.1f moves                              ║\n", 
           result->avg_game_length);
    printf("║  Avg time %s: %.2fs/game                          ║\n", 
           bot1->name, result->avg_time_bot1);
    printf("║  Avg time %s: %.2fs/game                          ║\n", 
           bot2->name, result->avg_time_bot2);
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    // Determine winner
    if (result->wins_bot1 > result->wins_bot2) {
        printf("🏆 Winner: %s\n\n", bot1->name);
    } else if (result->wins_bot2 > result->wins_bot1) {
        printf("🏆 Winner: %s\n\n", bot2->name);
    } else {
        printf("🤝 Match ended in a tie!\n\n");
    }
}

// Standalone arena main function
#ifdef ARENA_STANDALONE
int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    
    printf("\n🎮 Gomoku Bot Arena 🎮\n");
    printf("======================\n\n");
    
    // Print available bots
    printf("Available bot configurations:\n");
    print_bot_config(&BOT_DEFAULT);
    print_bot_config(&BOT_AGGRESSIVE);
    print_bot_config(&BOT_DEFENSIVE);
    print_bot_config(&BOT_DEEP_SEARCH);
    print_bot_config(&BOT_WIDE_SEARCH);
    
    // Create custom bot for testing
    bot_config_t my_bot = create_bot_config("MyTestBot");
    my_bot.num_simulations = 100000;
    my_bot.max_children = 12;
    my_bot.weight_capture = 12.0f;
    
    // Setup arena match
    arena_match_t match = {
        .bot1 = &BOT_DEFENSIVE,
        .bot2 = &BOT_WIDE_SEARCH,
        .num_games = 20,
        .swap_colors = 1,    // Fair: each bot plays both colors
        .verbose = 1,        // Set to 1 for move-by-move output
        .save_games = 0
    };
    
    // Run the match
    arena_result_t result = run_arena_match(&match);
    
    // Print results
    print_arena_result(&result, match.bot1, match.bot2);
    
    return 0;
}
#endif
