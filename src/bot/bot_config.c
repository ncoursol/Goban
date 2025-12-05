#include "bot_config.h"
#include <string.h>
#include <stdio.h>

// Final optimized configuration (based on 3 rounds of testing)
bot_config_t BOT_DEFAULT = {
    .name = "Default",
    .num_simulations = 100000,
    .max_children = 17,
    .exploration_constant = 1.414f,
    .weight_capture = 13.0f,
    .weight_block_capture = 10.0f,
    .weight_open_four = 45.0f,
    .weight_block_four = 40.0f,
    .weight_open_three = 8.0f,
    .weight_block_three = 7.0f,
    .weight_center = 0.15f,
    .weight_win = 10000.0f,
    .simulation_depth_limit = 1000,
    .num_threads = 0
};

// Aggressive variant
bot_config_t BOT_AGGRESSIVE = {
    .name = "Aggressive",
    .num_simulations = 100000,
    .max_children = 14,
    .exploration_constant = 1.3f,
    .weight_capture = 16.0f,
    .weight_block_capture = 8.0f,
    .weight_open_four = 50.0f,
    .weight_block_four = 35.0f,
    .weight_open_three = 10.0f,
    .weight_block_three = 5.0f,
    .weight_center = 0.18f,
    .weight_win = 10000.0f,
    .simulation_depth_limit = 1000,
    .num_threads = 0
};

// Defensive variant
bot_config_t BOT_DEFENSIVE = {
    .name = "Defensive",
    .num_simulations = 100000,
    .max_children = 20,
    .exploration_constant = 1.6f,
    .weight_capture = 8.0f,
    .weight_block_capture = 12.0f,
    .weight_open_four = 35.0f,
    .weight_block_four = 45.0f,
    .weight_open_three = 4.0f,
    .weight_block_three = 8.0f,
    .weight_center = 0.1f,
    .weight_win = 10000.0f,
    .simulation_depth_limit = 1000,
    .num_threads = 0
};

// Deep search variant (narrower tree)
bot_config_t BOT_DEEP_SEARCH = {
    .name = "DeepSearch",
    .num_simulations = 100000,
    .max_children = 15,
    .exploration_constant = 1.414f,
    .weight_capture = 10.0f,
    .weight_block_capture = 8.0f,
    .weight_open_four = 40.0f,
    .weight_block_four = 30.0f,
    .weight_open_three = 6.0f,
    .weight_block_three = 4.0f,
    .weight_center = 0.1f,
    .weight_win = 10000.0f,
    .simulation_depth_limit = 1000,
    .num_threads = 0
};

// Wide search variant (broader tree)
bot_config_t BOT_WIDE_SEARCH = {
    .name = "WideSearch",
    .num_simulations = 100000,
    .max_children = 22,
    .exploration_constant = 1.35f,
    .weight_capture = 14.0f,
    .weight_block_capture = 9.0f,
    .weight_open_four = 48.0f,
    .weight_block_four = 38.0f,
    .weight_open_three = 9.0f,
    .weight_block_three = 6.0f,
    .weight_center = 0.16f,
    .weight_win = 10000.0f,
    .simulation_depth_limit = 1000,
    .num_threads = 0
};

bot_config_t create_bot_config(const char *name)
{
    bot_config_t config = BOT_DEFAULT;
    strncpy(config.name, name, 63);
    config.name[63] = '\0';
    return config;
}

void print_bot_config(bot_config_t *config)
{
    printf("\n=== Bot Configuration: %s ===\n", config->name);
    printf("  Simulations: %d\n", config->num_simulations);
    printf("  Max children: %d\n", config->max_children);
    printf("  Exploration: %.3f\n", config->exploration_constant);
    printf("  Weights:\n");
    printf("    Capture: %.1f / Block: %.1f\n", config->weight_capture, config->weight_block_capture);
    printf("    Open4: %.1f / Block4: %.1f\n", config->weight_open_four, config->weight_block_four);
    printf("    Open3: %.1f / Block3: %.1f\n", config->weight_open_three, config->weight_block_three);
    printf("    Center: %.2f / Win: %.0f\n", config->weight_center, config->weight_win);
    printf("================================\n\n");
}
