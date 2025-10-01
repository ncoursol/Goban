#include "../include/gomo.h"

int read_sgf_game(gomo_t *gomo, char *filename) {
    printf("Reading SGF file: %s\n", filename);
    //char tmp[250];
    FILE *file = fopen(filename, "r");
    printf("File pointer: %p\n", file);
    if (!file) return -1;
    char line[1024];
    char *p;
    gomo->game_data->max_move = 0;
    
    while (fgets(line, sizeof(line), file)) {
        p = line;
        char *cursor;
        
        size_t len = strlen(line);
        if (len >= 2) line[len - 2] = '\0';
        p = line;
        
        // Metadata
        if ((cursor = strstr(p, "EV[")) != NULL) {
            sscanf(cursor, "EV[%49[^]]", gomo->game_data->event);
        }
        if ((cursor = strstr(p, "RO[")) != NULL) {
            sscanf(cursor, "RO[%49[^]]", gomo->game_data->round);
        }
        if ((cursor = strstr(p, "PB[")) != NULL) {
            sscanf(cursor, "PB[%49[^]]", gomo->game_data->black_player);
        }
        if ((cursor = strstr(p, "BR[")) != NULL) {
            sscanf(cursor, "BR[%49[^]]", gomo->game_data->black_rank);
        }
        if ((cursor = strstr(p, "PW[")) != NULL) {
            sscanf(cursor, "PW[%49[^]]", gomo->game_data->white_player);
        }
        if ((cursor = strstr(p, "WR[")) != NULL) {
            sscanf(cursor, "WR[%49[^]]", gomo->game_data->white_rank);
        }
        if ((cursor = strstr(p, "KM[")) != NULL) {
            sscanf(cursor, "KM[%19[^]]", gomo->game_data->komi);
        }
        if ((cursor = strstr(p, "RE[")) != NULL) {
            sscanf(cursor, "RE[%19[^]]", gomo->game_data->result);
        }
        if ((cursor = strstr(p, "DT[")) != NULL) {
            sscanf(cursor, "DT[%19[^]]", gomo->game_data->date);
        }
        if (p[0] == ';') {
            // Moves
            char *move = p;
            while ((move = strchr(move, ';'))) {
                move++;
                if ((*move == 'B' || *move == 'W') && move[1] == '[') {
                    if (gomo->game_data->max_move < MAX_MOVES) {
                        char color = *move;
                        char x = move[2];
                        char y = move[3];
                        if (x >= 'a' && x <= 's' && y >= 'a' && y <= 's') {
                            int col = x - 'a';
                            int row = y - 'a';
                            int idx = row * 19 + col;
                            gomo->game_data->moves[gomo->game_data->max_move].nb = gomo->game_data->max_move;
                            gomo->game_data->moves[gomo->game_data->max_move].id = idx;
                            gomo->game_data->moves[gomo->game_data->max_move].color = (color == 'B') ? (vec3_t){0.0f, 0.0f, 0.0f} : (vec3_t){1.0f, 1.0f, 1.0f};
                            gomo->board[idx].state = 1;
                            gomo->board[idx].color = gomo->game_data->moves[gomo->game_data->max_move].color;
                            gomo->nb_stones++;
                        }
                        gomo->game_data->max_move++;
                    }

                }
            }
        }
    }
    fclose(file);
    /*
    sprintf(tmp, "Event : %s, round - %s, date - %s", gomo->game_data->event, gomo->game_data->round, gomo->game_data->date);
    add_text_to_render(gomo, "font_text2", tmp, (vec3_t){5, HEIGHT - 60, 0.0f}, 0.3f, (vec3_t){0.9f, 0.9f, 0.9f}, 0, 6);
	sprintf(tmp, "Black_player : %s [%s]", gomo->game_data->black_player, gomo->game_data->black_rank);
    add_text_to_render(gomo, "font_text2", tmp, (vec3_t){5, HEIGHT - 90, 0.0f}, 0.3f, (vec3_t){0.9f, 0.9f, 0.9f}, 0, 7);
	sprintf(tmp, "White_player : %s [%s]", gomo->game_data->white_player, gomo->game_data->white_rank);
    add_text_to_render(gomo, "font_text2", tmp, (vec3_t){5, HEIGHT - 120, 0.0f}, 0.3f, (vec3_t){0.9f, 0.9f, 0.9f}, 0, 8);
	sprintf(tmp, "Komi : %s, Result - %s", gomo->game_data->komi,gomo->game_data->result);
    add_text_to_render(gomo, "font_text2", tmp, (vec3_t){5, HEIGHT - 150, 0.0f}, 0.3f, (vec3_t){0.9f, 0.9f, 0.9f}, 0, 9);
    */
    return 0;
}