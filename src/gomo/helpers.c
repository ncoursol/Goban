#include "../include/gomo.h"

void add_helper(gomo_t *gomo, int index, vec3_t color, int type, int id)
{
    float size[3] = {0.2f, 0.1f, 0.02f};
    vec3_t center = {gomo->board[index].pos.x, 0.438f, gomo->board[index].pos.z};
    line_t initial_lines[] = {
        {center, {center.x, center.y + size[type], center.z}, color}, // | line
        {{center.x + 0.025f, center.y, center.z - 0.025f}, {center.x - 0.025f, center.y, center.z + 0.025f}, color}, // / line
        {{center.x + 0.025f, center.y, center.z + 0.025f}, {center.x - 0.025f, center.y, center.z - 0.025f}, color}, // \ line
    };

    add_lines_batch(gomo, initial_lines, 3, 500 + id * 3);
}

void render_helpers(gomo_t *gomo)
{
    if (gomo->game->swap2_step != 4)
        return;
    int p1 = -1, p2 = -1;
    int number_of_helpers = 2;
    vec3_t helper_color1, helper_color2;

    if (gomo->game->moves->player != -1) {
        p1 = gomo->game->moves->x * 19 + gomo->game->moves->y;
        helper_color1 = gomo->game->moves->player == 0 ? (vec3_t){0.0f, 0.0f, 0.0f} : (vec3_t){1.0f, 1.0f, 1.0f};
    }
    if (gomo->game->moves->prev && gomo->game->moves->prev->player != -1) {
        p2 = gomo->game->moves->prev->x * 19 + gomo->game->moves->prev->y;
        helper_color2 = gomo->game->moves->prev->player == 0 ? (vec3_t){0.0f, 0.0f, 0.0f} : (vec3_t){1.0f, 1.0f, 1.0f};
    }

    if (p1 != -1)
        add_helper(gomo, p1, helper_color1, 0, 0);
    if (p2 != -1)
        add_helper(gomo, p2, helper_color2, 0, 1);

    if (gomo->game->moves->captured_stones) {
        for (int i = 0; gomo->game->moves->captured_stones[i]; i++) {
            int index = gomo->game->moves->captured_stones[i] / 19 * 19 + gomo->game->moves->captured_stones[i] % 19;
            add_helper(gomo, index, gomo->game->moves->player == 0 ? (vec3_t){1.0f, 1.0f, 1.0f} : (vec3_t){0.0f, 0.0f, 0.0f}, 2, number_of_helpers++);
        }
    }

    for (int i = 0; i < 19; i++) {
        for (int j = 0; j < 19; j++) {
            if (check_double_free_three(gomo->game->board, i, j, !gomo->game->swap2_player) && gomo->game->board[i][j] == 0) {
                add_helper(gomo, i * 19 + j, (vec3_t){1.0f, 0.0f, 0.0f}, 1, number_of_helpers++);
            }
        }
    }
    clear_lines_batch_to_render(gomo, 500 + number_of_helpers * 3, MAX_LINES - (500 + number_of_helpers * 3));
}