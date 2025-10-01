#include "../include/gomo.h"

void clear_lines(gomo_t *gomo)
{
    if (gomo->lines_buffer && gomo->nb_lines > 0) {
        if (gomo->lines_buffer[(gomo->nb_lines - 1) * 12 + 3] == 0.12f) {
            gomo->nb_lines -= 4; // Remove debug lines if present
        }
    }
}

void add_lines_batch(gomo_t *gomo, line_t *lines, int count, int start_id)
{
    if (!lines || count <= 0 || start_id + count > MAX_LINES)
        return;
        
    // Copy all lines to buffer at once
    for (int line_idx = 0; line_idx < count; line_idx++) {
        int id = start_id + line_idx;
        int i = id * 12;
        
        gomo->lines_buffer[i]     = lines[line_idx].start.x;
        gomo->lines_buffer[i + 1] = lines[line_idx].start.y;
        gomo->lines_buffer[i + 2] = lines[line_idx].start.z;
        gomo->lines_buffer[i + 3] = lines[line_idx].color.x;
        gomo->lines_buffer[i + 4] = lines[line_idx].color.y;
        gomo->lines_buffer[i + 5] = lines[line_idx].color.z;

        gomo->lines_buffer[i + 6] = lines[line_idx].end.x;
        gomo->lines_buffer[i + 7] = lines[line_idx].end.y;
        gomo->lines_buffer[i + 8] = lines[line_idx].end.z;
        gomo->lines_buffer[i + 9] = lines[line_idx].color.x;
        gomo->lines_buffer[i + 10] = lines[line_idx].color.y;
        gomo->lines_buffer[i + 11] = lines[line_idx].color.z;
    }
    
    int max_id = start_id + count;
    if (max_id > gomo->nb_lines) {
        gomo->nb_lines = max_id;
    }
    
    if (gomo->nb_lines > MAX_LINES) {
        gomo->nb_lines = MAX_LINES;
    }
}

void add_line_to_render(gomo_t *gomo, vec3_t start, vec3_t end, vec3_t color, int id)
{
    if (id >= MAX_LINES)
        return;

    int i = id * 12;

    gomo->lines_buffer[i]     = start.x;
    gomo->lines_buffer[i + 1] = start.y;
    gomo->lines_buffer[i + 2] = start.z;
    gomo->lines_buffer[i + 3] = color.x;
    gomo->lines_buffer[i + 4] = color.y;
    gomo->lines_buffer[i + 5] = color.z;

    gomo->lines_buffer[i + 6] = end.x;
    gomo->lines_buffer[i + 7] = end.y;
    gomo->lines_buffer[i + 8] = end.z;
    gomo->lines_buffer[i + 9] = color.x;
    gomo->lines_buffer[i + 10] = color.y;
    gomo->lines_buffer[i + 11] = color.z;

    if (id >= gomo->nb_lines) {
        gomo->nb_lines = id + 1;
    }
    
    if (gomo->nb_lines > MAX_LINES) {
        gomo->nb_lines = MAX_LINES;
    }
}

void render_all_lines(gomo_t *gomo)
{
    if (gomo->nb_lines == 0 || !gomo->shader || !gomo->lines_buffer)
        return;

    glUseProgram(gomo->shader->shaderProgramLine);
    glUniformMatrix4fv(gomo->shaderID.mvpID, 1, GL_FALSE, &gomo->camera->mvp[0]);
    
    glBindVertexArray(gomo->lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gomo->lineVBO);
    
    size_t buffer_size = gomo->nb_lines * 12 * sizeof(float);
    glBufferData(GL_ARRAY_BUFFER, buffer_size, gomo->lines_buffer, GL_DYNAMIC_DRAW);
    
    glDrawArrays(GL_LINES, 0, gomo->nb_lines * 2);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void init_lines(gomo_t *gomo)
{
    gomo->nb_lines = 0;
    
    if (!(gomo->lines = (line_t *)malloc(sizeof(line_t) * MAX_LINES)))
        exit_callback(gomo, 9, "lines malloc failed");
    
    memset(gomo->lines, 0, sizeof(line_t) * MAX_LINES);
    
    if (!(gomo->lines_buffer = (float *)malloc(sizeof(float) * MAX_LINES * 12)))
        exit_callback(gomo, 9, "lines buffer malloc failed");
    
    memset(gomo->lines_buffer, 0, sizeof(float) * MAX_LINES * 12);

    line_t initial_lines[] = {
        {{0.0f, 0.0f, 0.0f}, {100.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}}, // X axis
        {{0.0f, 0.0f, 0.0f}, {0.0f, 100.0f, 0.0f}, {0.0f, 1.0f, 0.0f}}, // Y axis
        {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 100.0f}, {0.0f, 0.0f, 1.0f}}, // Z axis
        {{1.0f, 0.0f, 0.04f}, {1.0f, 0.0f, -0.04f}, {1.0f, 0.0f, 0.0f}}, // 1m X axis
        {{0.0f, 1.0f, 0.04f}, {0.0f, 1.0f, -0.04f}, {0.0f, 1.0f, 0.0f}}, // 1m Y axis
        {{0.04f, 0.0f, 1.0f}, {-0.04f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}}, // 1m Z axis
        {{2.0f, 0.0f, 0.04f}, {2.0f, 0.0f, -0.04f}, {1.0f, 0.0f, 0.0f}}, // 2m X axis
        {{0.0f, 2.0f, 0.04f}, {0.0f, 2.0f, -0.04f}, {0.0f, 1.0f, 0.0f}}, // 2m Y axis
        {{0.04f, 0.0f, 2.0f}, {-0.04f, 0.0f, 2.0f}, {0.0f, 0.0f, 1.0f}}  // 2m Z axis
    };
    
    add_lines_batch(gomo, initial_lines, 9, 0);

    int line_id = 9;
    for (int i = -19; i < 19; i += 2) {
        for (int j = -19; j < 19; j += 2) {
            if (line_id < MAX_LINES) {
                line_t grid_line = {
                    {(i + 1) * 0.0245f, 0.4f, (j + 1) * 0.0245f},
                    {(i + 1) * 0.0245f, 0.47f, (j + 1) * 0.0245f},
                    {1.0f, 0.0f, 1.0f}
                };
                add_line_to_render(gomo, grid_line.start, grid_line.end, grid_line.color, line_id);
                line_id++;
            }
        }
    }
}

void free_lines(gomo_t *gomo)
{
    if (gomo->lines) {
        free(gomo->lines);
        gomo->lines = NULL;
    }
    if (gomo->lines_buffer) {
        free(gomo->lines_buffer);
        gomo->lines_buffer = NULL;
    }
    gomo->nb_lines = 0;
}