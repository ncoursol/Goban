#include "data_generation.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
    int num_games = 100;  // Default
    const char *output_dir = "generated_games";  // Default
    int num_threads = 1;  // Default (not yet implemented)
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
            num_games = atoi(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output_dir = argv[i + 1];
            i++;
        } else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
            num_threads = atoi(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("Usage: %s [options]\n", argv[0]);
            printf("Options:\n");
            printf("  -n <num>     Number of games to generate (default: 100)\n");
            printf("  -o <dir>     Output directory (default: generated_games)\n");
            printf("  -t <num>     Number of threads (default: 1, not yet implemented)\n");
            printf("  -h, --help   Show this help message\n");
            return 0;
        }
    }
    
    if (num_games <= 0) {
        fprintf(stderr, "Error: Number of games must be positive\n");
        return 1;
    }
    
    printf("Training Data Generator\n");
    printf("=======================\n");
    printf("Games to generate: %d\n", num_games);
    printf("Output directory: %s\n", output_dir);
    printf("Threads: %d\n\n", num_threads);
    
    int result = generate_training_games(num_games, output_dir, num_threads);
    
    if (result == 0) {
        printf("\nSuccess! Generated data saved to:\n");
        printf("  - Game records: %s/games/\n", output_dir);
        printf("  - Training data: %s/training/\n", output_dir);
        return 0;
    } else {
        fprintf(stderr, "\nError: Game generation failed\n");
        return 1;
    }
}
