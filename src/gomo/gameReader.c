#include "../include/gomo.h"

int read_sgf_game(gomo_t *gomo, char *filename) {
    char tmp[250];
    FILE *file = fopen(filename, "r");
    if (!file) return -1;
    char line[1024];
    char *p;
    gomo->game_data->max_move = 0;
    
    while (fgets(line, sizeof(line), file)) {
        p = line;
        
        size_t len = strlen(line);
        if (len >= 2) line[len - 2] = '\0';
        p = line;
        
        // Metadata
        if (p[0] == 'E' && p[1] == 'V') strcpy(gomo->game_data->event, &p[3]);
        else if (p[0] == 'R' && p[1] == 'O') strcpy(gomo->game_data->round, &p[3]);
        else if (p[0] == 'P' && p[1] == 'B') strcpy(gomo->game_data->black_player, &p[3]);
        else if (p[0] == 'B' && p[1] == 'R') strcpy(gomo->game_data->black_rank, &p[3]);
        else if (p[0] == 'P' && p[1] == 'W') strcpy(gomo->game_data->white_player, &p[3]);
        else if (p[0] == 'W' && p[1] == 'R') strcpy(gomo->game_data->white_rank, &p[3]);
        else if (p[0] == 'K' && p[1] == 'M') strcpy(gomo->game_data->komi, &p[3]);
        else if (p[0] == 'R' && p[1] == 'E') strcpy(gomo->game_data->result, &p[3]);
        else if (p[0] == 'D' && p[1] == 'T') strcpy(gomo->game_data->date, &p[3]);
        else if (p[0] == ';') {
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
                        }
                        gomo->game_data->max_move++;
                    }

                }
            }
        }
    }
    fclose(file);
    sprintf(tmp, "Event : %s, round - %s, date - %s", gomo->game_data->event, gomo->game_data->round, gomo->game_data->date);
    add_text_to_render(gomo, "font_text2", tmp, (vec3_t){5, HEIGHT - 200, 0.0f}, 0.3f, (vec3_t){0.9f, 0.9f, 0.9f}, 10);
	sprintf(tmp, "Black_player : %s [%s]", gomo->game_data->black_player, gomo->game_data->black_rank);
    add_text_to_render(gomo, "font_text2", tmp, (vec3_t){5, HEIGHT - 230, 0.0f}, 0.3f, (vec3_t){0.9f, 0.9f, 0.9f}, 11);
	sprintf(tmp, "White_player : %s [%s]", gomo->game_data->white_player, gomo->game_data->white_rank);
    add_text_to_render(gomo, "font_text2", tmp, (vec3_t){5, HEIGHT - 260, 0.0f}, 0.3f, (vec3_t){0.9f, 0.9f, 0.9f}, 12);
	sprintf(tmp, "Komi : %s, Result - %s", gomo->game_data->komi,gomo->game_data->result);
    add_text_to_render(gomo, "font_text2", tmp, (vec3_t){5, HEIGHT - 290, 0.0f}, 0.3f, (vec3_t){0.9f, 0.9f, 0.9f}, 13);
    return 0;
}