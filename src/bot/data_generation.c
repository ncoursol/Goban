#include "data_generation.h"
#include "bot_config.h"
#include "mcts.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

// Forward declarations for functions not in headers
extern void free_tree(node_t *node);

game_data_t* init_game_data(void)
{
    game_data_t *data = (game_data_t *)malloc(sizeof(game_data_t));
    if (!data)
        return NULL;
    
    data->samples = NULL;
    data->num_samples = 0;
    data->winner = -1;
    data->total_moves = 0;
    data->final_captured_black = 0;
    data->final_captured_white = 0;
    memset(data->final_board, 0, sizeof(data->final_board));
    
    return data;
}

void add_training_sample(
    game_data_t *data,
    unsigned int board[19][19],
    node_t *root,
    int current_player,
    unsigned int captured_black,
    unsigned int captured_white,
    int move_number,
    int x,
    int y)
{
    training_sample_t *sample = (training_sample_t *)malloc(sizeof(training_sample_t));
    if (!sample)
        return;
    
    // Copy board state
    memcpy(sample->board, board, sizeof(unsigned int) * 19 * 19);
    
    // Initialize policy to zeros
    memset(sample->policy, 0, sizeof(float) * 19 * 19);
    
    // Extract policy from MCTS visit counts
    if (root && root->childs) {
        int total_visits = 0;
        int num_children = atomic_load(&root->nb_childs);
        
        // Calculate total visits across all children
        for (int i = 0; i < num_children; i++) {
            if (root->childs[i]) {
                total_visits += atomic_load(&root->childs[i]->visit_count);
            }
        }
        
        // Normalize visit counts to probabilities
        if (total_visits > 0) {
            for (int i = 0; i < num_children; i++) {
                if (root->childs[i]) {
                    int child_x = root->childs[i]->x;
                    int child_y = root->childs[i]->y;
                    int visits = atomic_load(&root->childs[i]->visit_count);
                    sample->policy[child_x][child_y] = (float)visits / (float)total_visits;
                }
            }
        }
    }
    
    sample->current_player = current_player;
    sample->captured_black = captured_black;
    sample->captured_white = captured_white;
    sample->move_number = move_number;
    sample->x = x;
    sample->y = y;
    sample->next = NULL;
    
    // Add to linked list
    if (data->samples == NULL) {
        data->samples = sample;
    } else {
        training_sample_t *current = data->samples;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = sample;
    }
    
    data->num_samples++;
}

static int ensure_directory_exists(const char *path)
{
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        if (mkdir(path, 0755) == -1) {
            if (errno != EEXIST) {
                perror("mkdir");
                return -1;
            }
        }
    }
    return 0;
}

int save_game_data(game_data_t *data, const char *output_dir, int game_id)
{
    if (!data || !output_dir)
        return -1;
    
    char json_dir[512];
    char training_dir[512];
    snprintf(json_dir, sizeof(json_dir), "%s/games", output_dir);
    snprintf(training_dir, sizeof(training_dir), "%s/training", output_dir);
    
    // Create directories
    if (ensure_directory_exists(output_dir) == -1)
        return -1;
    if (ensure_directory_exists(json_dir) == -1)
        return -1;
    if (ensure_directory_exists(training_dir) == -1)
        return -1;
    
    // Save game info as JSON
    char json_path[512];
    snprintf(json_path, sizeof(json_path), "%s/game_%05d.json", json_dir, game_id);
    
    FILE *json_file = fopen(json_path, "w");
    if (!json_file) {
        perror("fopen json");
        return -1;
    }
    
    fprintf(json_file, "{\n");
    fprintf(json_file, "  \"game_id\": %d,\n", game_id);
    fprintf(json_file, "  \"winner\": %d,\n", data->winner);
    fprintf(json_file, "  \"total_moves\": %d,\n", data->total_moves);
    fprintf(json_file, "  \"captured_black\": %d,\n", data->final_captured_black);
    fprintf(json_file, "  \"captured_white\": %d,\n", data->final_captured_white);
    fprintf(json_file, "  \"num_training_samples\": %d,\n", data->num_samples);
    
    // Save move sequence
    fprintf(json_file, "  \"moves\": [\n");
    training_sample_t *sample = data->samples;
    int first = 1;
    while (sample) {
        if (!first)
            fprintf(json_file, ",\n");
        fprintf(json_file, "    {\"move\": %d, \"x\": %d, \"y\": %d, \"player\": %d}",
                sample->move_number, sample->x, sample->y, sample->current_player);
        first = 0;
        sample = sample->next;
    }
    fprintf(json_file, "\n  ],\n");
    
    // Save final board state
    fprintf(json_file, "  \"final_board\": [\n");
    for (int i = 0; i < 19; i++) {
        fprintf(json_file, "    [");
        for (int j = 0; j < 19; j++) {
            fprintf(json_file, "%d", data->final_board[i][j]);
            if (j < 18)
                fprintf(json_file, ", ");
        }
        fprintf(json_file, "]");
        if (i < 18)
            fprintf(json_file, ",");
        fprintf(json_file, "\n");
    }
    fprintf(json_file, "  ]\n");
    fprintf(json_file, "}\n");
    
    fclose(json_file);
    
    // Save training data as binary NPZ-like format (simplified binary)
    char training_path[512];
    snprintf(training_path, sizeof(training_path), "%s/game_%05d.dat", training_dir, game_id);
    
    FILE *training_file = fopen(training_path, "wb");
    if (!training_file) {
        perror("fopen training");
        return -1;
    }
    
    // Write header
    int num_samples = data->num_samples;
    fwrite(&num_samples, sizeof(int), 1, training_file);
    fwrite(&data->winner, sizeof(int), 1, training_file);
    
    // Write each training sample
    sample = data->samples;
    while (sample) {
        // Determine value target based on winner and current player
        float value;
        if (data->winner == 2) {
            value = 0.0f;  // Draw
        } else if (data->winner == sample->current_player) {
            value = 1.0f;  // Win
        } else {
            value = -1.0f; // Loss
        }
        
        fwrite(sample->board, sizeof(unsigned int), 19 * 19, training_file);
        fwrite(sample->policy, sizeof(float), 19 * 19, training_file);
        fwrite(&value, sizeof(float), 1, training_file);
        fwrite(&sample->current_player, sizeof(int), 1, training_file);
        fwrite(&sample->captured_black, sizeof(unsigned int), 1, training_file);
        fwrite(&sample->captured_white, sizeof(unsigned int), 1, training_file);
        fwrite(&sample->move_number, sizeof(int), 1, training_file);
        
        sample = sample->next;
    }
    
    fclose(training_file);
    
    printf("Saved game %d: %d samples, winner=%d\n", game_id, data->num_samples, data->winner);
    
    return 0;
}

void free_game_data(game_data_t *data)
{
    if (!data)
        return;
    
    training_sample_t *sample = data->samples;
    while (sample) {
        training_sample_t *next = sample->next;
        free(sample);
        sample = next;
    }
    
    free(data);
}

static void play_selfplay_game(game_data_t *game_data, int game_id)
{
    game_t game;
    init_game(&game, 2);  // EvE mode
    
    // Disable swap2 opening for training data generation
    // Set swap2_step to 4 (finished state) to skip all swap2 logic
    game.swap2_step = 4;
    
    // Use default bot config (we just use the global MCTS without custom config)
    // The MCTS will use its default settings
    
    int max_moves = 361;  // Maximum possible moves on 19x19 board
    int move_count = 0;
    
    while (!game.game_over && move_count < max_moves) {
        int x, y;
        
        // Run MCTS and get the root tree for policy extraction
        node_t *root = run_mcts_return_tree(&game, &x, &y);
        
        if (x == -1 || y == -1 || !root) {
            if (root)
                free_tree(root);
            fprintf(stderr, "Game %d: No valid move found at move %d\n", game_id, move_count);
            break;
        }
        
        // Add training sample before making the move
        add_training_sample(
            game_data,
            game.board,
            root,  // Pass MCTS root for policy extraction
            game.current_player,
            game.captured_black,
            game.captured_white,
            move_count,
            x,
            y
        );
        
        // Free the MCTS tree
        free_tree(root);
        
        // Make the move
        int result = place_stone(&game, x, y);
        if (result != 0) {
            // place_stone returns non-zero if someone won (1 = player 1, 2 = player 2)
            game.game_over = result;
            move_count++;
            break;
        }
        
        move_count++;
        
        // Check for game over conditions
        if (game.game_over) {
            break;
        }
    }
    
    // Store final game state
    memcpy(game_data->final_board, game.board, sizeof(game.board));
    game_data->final_captured_black = game.captured_black;
    game_data->final_captured_white = game.captured_white;
    game_data->total_moves = move_count;
    
    // Determine winner
    if (game.game_over == 1) {
        game_data->winner = 0;  // Player 1 wins
    } else if (game.game_over == 2) {
        game_data->winner = 1;  // Player 2 wins
    } else {
        game_data->winner = 2;  // Draw/incomplete
    }
    
    cleanup_game(&game);
}

int generate_training_games(int num_games, const char *output_dir, int num_threads)
{
    (void)num_threads;  // TODO: Implement parallel game generation
    
    printf("Generating %d self-play games...\n", num_games);
    
    for (int i = 0; i < num_games; i++) {
        game_data_t *game_data = init_game_data();
        if (!game_data) {
            fprintf(stderr, "Failed to allocate game data for game %d\n", i);
            continue;
        }
        
        play_selfplay_game(game_data, i);
        
        if (save_game_data(game_data, output_dir, i) != 0) {
            fprintf(stderr, "Failed to save game %d\n", i);
        }
        
        free_game_data(game_data);
        
        if ((i + 1) % 10 == 0) {
            printf("Progress: %d/%d games completed\n", i + 1, num_games);
        }
    }
    
    printf("Game generation complete!\n");
    return 0;
}
