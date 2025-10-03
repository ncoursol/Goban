#include "../../include/gomo.h"

void create_grid(gomo_t *gomo, vec3_t position, int nb_col, int nb_row, vec3_t *stones, int nb_stones, int index)
{
    line_t initial_lines[nb_row + nb_col + (nb_stones * 2)];
    float gap = 0.1f;
    float offset = 0.025f;
    for (int i = 0; i < nb_row; i++) {
        initial_lines[i] = (line_t){
            {position.x, position.y - (i * gap) - (gap / 2), 3.2f},
            {position.x - (nb_col * gap), position.y - (i * gap) - (gap / 2), 3.2f},
            {0.0f, 0.0f, 0.0f}
        };
    }
    for (int j = 0; j < nb_col; j++) {
        initial_lines[nb_row + j] = (line_t){
            {position.x - (j * gap) - (gap / 2), position.y, 3.2f},
            {position.x - (j * gap) - (gap / 2), position.y - (nb_row * gap), 3.2f},
            {0.0f, 0.0f, 0.0f}
        };
    }
    for (int k = 0; k < nb_stones; k++) {
        if (stones[k].z > 1) {
            offset = 0.01f;
            add_text_to_render(gomo, "font_text2", stones[k].z == 2 ? "b" : "r", (vec3_t){position.x - (stones[k].x * gap) - (gap / 2) - 0.02f, position.y - (stones[k].y * gap) - (gap / 2) + 0.02f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.001f, stones[k].z == 2 ? (vec3_t){0.0f, 0.0f, 1.0f} : (vec3_t){1.0f, 0.0f, 0.0f}, 1, 38 + index + k);
        } else {
            offset = 0.025f;
        }
        initial_lines[nb_row + nb_col + k] = (line_t){
            {position.x - (stones[k].x * gap) - (gap / 2) + offset, position.y - (stones[k].y * gap) - (gap / 2) + offset, 3.2f},
            {position.x - (stones[k].x * gap) - (gap / 2) - offset, position.y - (stones[k].y * gap) - (gap / 2) - offset, 3.2f},
            stones[k].z == 0 || stones[k].z == 2 ? (vec3_t){0.0f, 0.0f, 1.0f} : (vec3_t){1.0f, 0.0f, 0.0f}
        };
        initial_lines[nb_row + nb_col + nb_stones + k] = (line_t){
            {position.x - (stones[k].x * gap) - (gap / 2) - offset, position.y - (stones[k].y * gap) - (gap / 2) + offset, 3.2f},
            {position.x - (stones[k].x * gap) - (gap / 2) + offset, position.y - (stones[k].y * gap) - (gap / 2) - offset, 3.2f},
            stones[k].z == 0 || stones[k].z == 2 ? (vec3_t){0.0f, 0.0f, 1.0f} : (vec3_t){1.0f, 0.0f, 0.0f}
        };
    }
    add_lines_batch(gomo, initial_lines, nb_row + nb_col + (nb_stones * 2), 375 + index);
}

void change_tutorial(gomo_t *gomo)
{
    if (gomo->cursor < 12 || gomo->cursor > 16)
        gomo->cursor = 12;

    for (int i = 19; i < NB_TEXT; i++) {
        clear_text_to_render(gomo, i);
    }
    clear_lines_batch_to_render(gomo, 375, MAX_LINES - 375);
    switch (gomo->cursor)
    {
        case 12:
            add_text_to_render(gomo, "font_text2", "Overview", (vec3_t){1.0f, 3.1f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.004f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 19);

            add_text_to_render(gomo, "font_text2", "Objective:", (vec3_t){1.0f, 2.7f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.002f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 20);
            add_text_to_render(gomo, "font_text2", "Two players take turns placing stones of their color on an intersection of", (vec3_t){0.8f, 2.5f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 21);
            add_text_to_render(gomo, "font_text2", "the board.", (vec3_t){0.8f, 2.4f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 22);
            add_text_to_render(gomo, "font_text2", "The goal is to be the first to make an unbroken row of five stones", (vec3_t){0.8f, 2.2f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 23);
            add_text_to_render(gomo, "font_text2", "or more (horizontal, vertical or diagonal).", (vec3_t){0.8f, 2.1f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 24);
            create_grid(gomo, (vec3_t){-2.0f, 2.1f, 3.2f}, 5, 5, (vec3_t[5]){{0, 0, 0}, {1, 1, 0}, {2, 2, 0}, {3, 3, 2}, {4, 4, 0}}, 5, 0);
            add_text_to_render(gomo, "font_text2", "A player can also win by meeting a capture victory", (vec3_t){0.8f, 1.9f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 25);
            add_text_to_render(gomo, "font_text2", "condition : capture 5 pairs of opponents stones (see Captures).", (vec3_t){0.8f, 1.8f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 26);

            add_text_to_render(gomo, "font_text2", "Common rules:", (vec3_t){1.0f, 1.6f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.002f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 27);
            add_text_to_render(gomo, "font_text2", "- Black moves first unless the opening rule (Swap2) changes who takes Black.", (vec3_t){0.8f, 1.4f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 28);
            add_text_to_render(gomo, "font_text2", "- Gomoku is played on a 19x19 Goban, without limit to the number of stones.", (vec3_t){0.8f, 1.3f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 29);
            add_text_to_render(gomo, "font_text2", "- There is no limitation of time.", (vec3_t){0.8f, 1.2f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 30);
            add_text_to_render(gomo, "font_text2", "- Double free-three are FORBIDDEN (see Free-threes)", (vec3_t){0.8f, 1.1f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 31);
            add_text_to_render(gomo, "font_text2", "- You don't stand a chance against my AI.", (vec3_t){0.8f, 1.0f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 32);
            break;
        case 13:
            add_text_to_render(gomo, "font_text2", "Captures", (vec3_t){1.0f, 3.1f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.004f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 19);

            add_text_to_render(gomo, "font_text2", "Captures rules:", (vec3_t){1.0f, 2.7f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.002f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 20);
            add_text_to_render(gomo, "font_text2", "- If you flank EXACTLY two contiguous opponent stones in a straight line, those", (vec3_t){0.8f, 2.5f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 21);
            add_text_to_render(gomo, "font_text2", "two opponent stones are removed from the board.", (vec3_t){0.7f, 2.4f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 22);
            create_grid(gomo, (vec3_t){0.0f, 2.3f, 3.2f}, 4, 4, (vec3_t[4]){{0, 0, 0}, {1, 1, 1}, {2, 2, 1}, {3, 3, 2}}, 4, 0);
            create_grid(gomo, (vec3_t){0.6f, 2.2f, 3.2f}, 4, 1, (vec3_t[4]){{0, 0, 0}, {1, 0, 1}, {2, 0, 1}, {3, 0, 2}}, 4, 16);
            create_grid(gomo, (vec3_t){-0.6f, 2.3f, 3.2f}, 4, 4, (vec3_t[7]){{0, 0, 0}, {1, 0, 1}, {2, 0, 1}, {0, 3, 0}, {1, 2, 1}, {2, 1, 1}, {3, 0, 2}}, 7, 29);
            add_text_to_render(gomo, "font_text2", "- Captures are resolved immediately when the flanking stone is placed.", (vec3_t){0.8f, 1.8f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 23);
            add_text_to_render(gomo, "font_text2", "- If a player captures a total of 5 pairs of stones, THEY WIN THE GAME.", (vec3_t){0.8f, 1.7f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 24);
            add_text_to_render(gomo, "font_text2", "- Captures can change threats and block lines; they may also be used tactically", (vec3_t){0.8f, 1.6f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 25);
            add_text_to_render(gomo, "font_text2", "to prevent opponent fives.", (vec3_t){0.7f, 1.5f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 26);
            add_text_to_render(gomo, "font_text2", "- A player can place a stone even if it create a capture situation for the", (vec3_t){0.8f, 1.4f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 27);
            add_text_to_render(gomo, "font_text2", "opponent without being captured themselves.", (vec3_t){0.7f, 1.3f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 28);
            add_text_to_render(gomo, "font_text2", "Example :", (vec3_t){0.7f, 1.1f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 29);
            create_grid(gomo, (vec3_t){0.6f, 1.0f, 3.2f}, 4, 1, (vec3_t[4]){{0, 0, 0}, {1, 0, 1}, {3, 0, 0}, {2, 0, 3}}, 4, 51);
            add_text_to_render(gomo, "font_text2", "In this scenario, Red can play in 'r' without losing the pair. However, if later", (vec3_t){0.7f, 0.8f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 30);
            add_text_to_render(gomo, "font_text2", "Red takes one of the Blue stones, his position becomes vulnerable to capture...", (vec3_t){0.7f, 0.7f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 31);
            break;
        case 14:
            add_text_to_render(gomo, "font_text2", "Opening system", (vec3_t){1.0f, 3.1f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.004f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 19);

            add_text_to_render(gomo, "font_text2", "Swap2 opening system purpose:", (vec3_t){1.0f, 2.7f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.002f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 20);
            add_text_to_render(gomo, "font_text2", "Tournament opening rules are used in professional play to balance the game and", (vec3_t){0.8f, 2.5f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 21);
            add_text_to_render(gomo, "font_text2", "mitigate the first player advantage.", (vec3_t){0.8f, 2.4f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 22);
            add_text_to_render(gomo, "font_text2", "Regular Gomoku is proven to be unfair, a perfect first player wins 100% of the time.", (vec3_t){0.8f, 2.3f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 23);
            add_text_to_render(gomo, "font_text2", "The win ratio of the first player has been calculated to be around 52 percent using", (vec3_t){0.8f, 2.2f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 24);
            add_text_to_render(gomo, "font_text2", "the Swap2 opening protocol, greatly balancing the game and largely evening out", (vec3_t){0.8f, 2.1f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 25);
            add_text_to_render(gomo, "font_text2", "the first-player advantage.", (vec3_t){0.8f, 2.0f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 26);

            add_text_to_render(gomo, "font_text2", "Swap2 rules:", (vec3_t){1.0f, 1.7f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.002f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 27);
            add_text_to_render(gomo, "font_text2", "- Step 1: Black (first player) places three stones anywhere: two black and one white.", (vec3_t){0.8f, 1.5f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 28);
            add_text_to_render(gomo, "font_text2", "- Step 2: White (second player) chooses one of three options:", (vec3_t){0.8f, 1.4f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 29);
            add_text_to_render(gomo, "font_text2", "1. Play as Black.", (vec3_t){0.6f, 1.3f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 30);
            add_text_to_render(gomo, "font_text2", "2. Play as White.", (vec3_t){0.6f, 1.2f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 31);
            add_text_to_render(gomo, "font_text2", "3. Place an additional pair of stones (one black and one white) anywhere;", (vec3_t){0.6f, 1.1f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 32);
            add_text_to_render(gomo, "font_text2", "after that, the original first player chooses which color to play.", (vec3_t){0.5f, 1.0f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 33);
            add_text_to_render(gomo, "font_text2", "Example :", (vec3_t){0.7f, 0.8f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 34);
            add_text_to_render(gomo, "font_text2", "Step 1 :", (vec3_t){0.7f, 0.6f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 35);
            create_grid(gomo, (vec3_t){0.3f, 0.7f, 3.2f}, 4, 5, (vec3_t[3]){{0, 0, 2}, {2, 1, 3}, {0, 3, 2}}, 3, 0);
            add_text_to_render(gomo, "font_text2", "Step 2.3 (optional) :", (vec3_t){-0.3f, 0.6f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 36);
            create_grid(gomo, (vec3_t){-1.3f, 0.7f, 3.2f}, 4, 5, (vec3_t[5]){{0, 0, 0}, {2, 1, 1}, {0, 3, 0}, {3, 3, 2}, {2, 4, 3}}, 5, 15);
            break;
        case 15:
            add_text_to_render(gomo, "font_text2", "Free-threes", (vec3_t){1.0f, 3.1f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.004f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 19);

            add_text_to_render(gomo, "font_text2", "Free-threes and double free-threes:", (vec3_t){1.0f, 2.7f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.002f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 20);
            add_text_to_render(gomo, "font_text2", "- Free three (open three): a contiguous line of three own stones that has at", (vec3_t){0.8f, 2.5f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 21);
            add_text_to_render(gomo, "font_text2", "least one empty intersection on each end.", (vec3_t){0.7f, 2.4f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 22);
            create_grid(gomo, (vec3_t){0.6f, 2.2f, 3.2f}, 6, 1, (vec3_t[3]){{1, 0, 1}, {3, 0, 1}, {4, 0, 1}}, 3, 0);
            create_grid(gomo, (vec3_t){-0.2f, 2.3f, 3.2f}, 5, 5, (vec3_t[3]){{1, 1, 0}, {2, 2, 0}, {3, 3, 0}}, 3, 13);
            
            add_text_to_render(gomo, "font_text2", "- Double free-three: a single move that simultaneously creates two distinct", (vec3_t){0.8f, 1.7f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 23);
            add_text_to_render(gomo, "font_text2", "free-threes (two independent open-three threats).", (vec3_t){0.7f, 1.6f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 24);
            add_text_to_render(gomo, "font_text2", "Double free-threes are FORBIDDEN. Any move that creates two or more separate", (vec3_t){0.8f, 1.5f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 25);
            add_text_to_render(gomo, "font_text2", "open threes for the same player is illegal and must be rejected.", (vec3_t){0.8f, 1.4f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 26);
            add_text_to_render(gomo, "font_text2", "Example :", (vec3_t){0.7f, 1.2f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 27);
            create_grid(gomo, (vec3_t){0.6f, 1.1f, 3.2f}, 8, 5, (vec3_t[6]){{1, 1, 1}, {2, 2, 1}, {3, 4, 2}, {4, 4, 3}, {5, 4, 1}, {6, 4, 1}}, 6, 29);
            add_text_to_render(gomo, "font_text2", "In this scenario, by playing in 'r', Red would introduce a", (vec3_t){-0.3f, 1.2f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 28);
            add_text_to_render(gomo, "font_text2", "double-three, therefore this is a forbidden move.", (vec3_t){-0.3f, 1.1f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 29);
            add_text_to_render(gomo, "font_text2", "However, if there were a blue stone in 'b', one of the", (vec3_t){-0.3f, 0.9f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 30);
            add_text_to_render(gomo, "font_text2", "three-aligned would be obstructed, therefore the move", (vec3_t){-0.3f, 0.8f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 31);
            add_text_to_render(gomo, "font_text2", "in 'r' would be legal.", (vec3_t){-0.3f, 0.7f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 32);
            add_text_to_render(gomo, "font_text2", "It is important to note that it is NOT forbidden to introduce a double-three", (vec3_t){0.8f, 0.4f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 33);
            add_text_to_render(gomo, "font_text2", "by capturing a pair.", (vec3_t){0.8f, 0.3f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 34);

            break;
        case 16:
            add_text_to_render(gomo, "font_text2", "Tips", (vec3_t){1.0f, 3.1f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.004f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 19);

            add_text_to_render(gomo, "font_text2", "Short play tips:", (vec3_t){1.0f, 2.7f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.002f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 20);
            add_text_to_render(gomo, "font_text2", "- Watch for captures and capture-threats as they can quickly change the balance.", (vec3_t){0.8f, 2.5f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 21);
            add_text_to_render(gomo, "font_text2", "- Use Swap2 openings to avoid extremely aggressive first-player setups.", (vec3_t){0.8f, 2.4f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 22);
            add_text_to_render(gomo, "font_text2", "- When building toward five, be mindful of free-three rules: try to create single,", (vec3_t){0.8f, 2.3f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 23);
            add_text_to_render(gomo, "font_text2", "hard-to-block threats without violating the double free-three prohibition.", (vec3_t){0.7f, 2.2f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 24);
            add_text_to_render(gomo, "font_text2", "- Block your opponent’s open-threes immediately when possible; prevent them from", (vec3_t){0.8f, 2.1f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 25);
            add_text_to_render(gomo, "font_text2", "making two separate open-threes in one move.", (vec3_t){0.7f, 2.0f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 2, 0.0015f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 26);
            create_grid(gomo, (vec3_t){-0.1f, 1.8f, 3.2f}, 13, 13, (vec3_t[39]){
                {3, 3, 1}, {5, 3, 3}, {6, 3, 1}, {9, 3, 1}, {11, 3, 1},
                {4, 4, 0}, {5, 4, 1}, {6, 4, 0}, {7, 4, 0}, {8, 4, 0}, {9, 4, 1}, {10, 4, 0},
                {4, 5, 1}, {5, 5, 0}, {6, 5, 0}, {7, 5, 0}, {8, 5, 1}, {9, 5, 0},
                {4, 6, 1}, {5, 6, 1}, {6, 6, 0}, {7, 6, 1}, {8, 6, 0},
                {4, 7, 1}, {5, 7, 0}, {6, 7, 0}, {7, 7, 0}, {8, 7, 1}, {9, 7, 0},
                {4, 8, 1}, {5, 8, 0}, {6, 8, 1}, {8, 8, 1}, {10, 8, 2},
                {3, 9, 1}, {4, 9, 0}, {5, 9, 0}, {6, 9, 0},
                {5, 10, 1},
            }, 39, 0);
            break;

    }
}

void clear_tutorial(gomo_t *gomo)
{
    for (int i = 11; i < NB_TEXT; i++)
    {
        text_t *text = &gomo->text[i];
        if (text->id && text->text)
        {
            free(text->text);
            text->text = NULL;
        }
    }
    clear_lines_batch_to_render(gomo, 375, MAX_LINES - 375);
}

void display_menu(gomo_t *gomo)
{
    gomo->textHover = -1;
    add_text_to_render(gomo, "font_text2", "Main Menu", (vec3_t){0.0f, 1.8f, 0.0f}, (vec3_t){0.0f, 0.0f, 0.0f}, 1, 0.005f, (vec3_t){1.0f, 0.5f, 0.5f}, 1, 3);
	add_text_to_render(gomo, "font_text2", "Human VS Human", (vec3_t){0.0f, 1.00f, 0.0f}, (vec3_t){0.0f, 0.0f, 0.0f}, 1, 0.003f, (vec3_t){1.0f, 0.5f, 0.5f}, 1, 4);
	add_text_to_render(gomo, "font_text2", "Human VS IA", (vec3_t){0.0f, 0.80f, 0.0f}, (vec3_t){0.0f, 0.0f, 0.0f}, 1, 0.003f, (vec3_t){1.0f, 0.5f, 0.5f}, 1, 5);
    add_text_to_render(gomo, "font_text2", "IA VS IA", (vec3_t){1.0f, 1.0f, 1.0f}, (vec3_t){0.0f, 0.0f, 0.0f}, 1, 0.003f, (vec3_t){1.0f, 0.5f, 0.5f}, 1, 6);
    add_text_to_render(gomo, "font_text2", "Gomoku ?", (vec3_t){-1.0f, 1.0f, -1.0f}, (vec3_t){0.0f, 0.0f, 0.0f}, 1, 0.003f, (vec3_t){1.0f, 0.5f, 0.5f}, 1, 7);
}

void display_tutorial(gomo_t *gomo)
{
    clear_lines_batch_to_render(gomo, 371, 4);

    add_text_to_render(gomo, "font_text2", "How to play", (vec3_t){2.0f, 3.2f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 0, 0.004f, (vec3_t){0.9f, 0.9f, 0.9f}, 1, 11);

    // Summary
    add_text_to_render(gomo, "font_text2", "Overview", (vec3_t){2.0f, 2.6f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 0, 0.003f, (vec3_t){1.0f, 0.5f, 0.5f}, 1, 12);
    add_text_to_render(gomo, "font_text2", "Captures", (vec3_t){2.0f, 2.3f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 0, 0.003f, (vec3_t){1.0f, 0.5f, 0.5f}, 1, 13);
    add_text_to_render(gomo, "font_text2", "Opening system", (vec3_t){2.0f, 2.0f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 0, 0.003f, (vec3_t){1.0f, 0.5f, 0.5f}, 1, 14);
    add_text_to_render(gomo, "font_text2", "Free-threes", (vec3_t){2.0f, 1.7f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 0, 0.003f, (vec3_t){1.0f, 0.5f, 0.5f}, 1, 15);
    add_text_to_render(gomo, "font_text2", "Tips", (vec3_t){2.0f, 1.4f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 0, 0.003f, (vec3_t){1.0f, 0.5f, 0.5f}, 1, 16);

    // Back button
    if (gomo->textHover == 7)
        add_text_to_render(gomo, "font_text2", "Back", (vec3_t){2.4f, 0.5f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 0, 0.004f, (vec3_t){1.0f, 0.5f, 0.5f}, 1, 17);

    gomo->textHover = -1;
    
    change_tutorial(gomo);
}


void display_gameMode(gomo_t *gomo)
{
    clear_lines_batch_to_render(gomo, 371, 4);

    char *mode_text = NULL;
    switch (gomo->textHover)
    {
        case 4:
            mode_text = "Human VS Human";
            break;
        case 5:
            mode_text = "Human VS IA";
            break;
        case 6:
            mode_text = "IA VS IA";
            break;
        default:
            mode_text = "Unknown Mode";
            break;
    }

    for (int i = 3; i < 8; i++)
        clear_text_to_render(gomo, i);

    gomo->textHover = -1;

    add_text_to_render(gomo, "font_text2", mode_text, (vec3_t){0.7f, 0.0f, 1.0f}, (vec3_t){PI / 2.0f, PI, 0.0f}, 2, 0.004f, (vec3_t){1.0f, 0.5f, 0.5f}, 1, 37);
    //add_text_to_render(gomo, "font_text2", "Overview", (vec3_t){2.0f, 2.6f, 3.2f}, (vec3_t){0.0f, PI, 0.0f}, 0, 0.003f, (vec3_t){1.0f, 0.5f, 0.5f}, 1, 12);

}