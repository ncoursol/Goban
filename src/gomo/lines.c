#include "../include/gomo.h"

void render_lines(gomo_t *gomo)
{
    glUseProgram(gomo->shader->shaderProgramLine);
    glUniformMatrix4fv(gomo->shaderID.mvpID, 1, GL_FALSE, &gomo->camera->mvp[0]);
    glBindVertexArray(gomo->lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gomo->lineVBO);
    glDrawArrays(GL_LINES, 0, gomo->nb_lines * 2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

}

void update_lines_buffer(gomo_t *gomo)
{
    if (gomo->shader != NULL && gomo->shader->shaderProgramLine != 0) {
        glUseProgram(gomo->shader->shaderProgramLine);
        glUniformMatrix4fv(gomo->shaderID.mvpID, 1, GL_FALSE, &gomo->camera->mvp[0]);
        glBindVertexArray(gomo->lineVAO);
        glBindBuffer(GL_ARRAY_BUFFER, gomo->lineVBO);
        
        glBufferData(GL_ARRAY_BUFFER, gomo->nb_lines * 12 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
        float *data = (float *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

	    memcpy(data, &gomo->lines_buffer[0], gomo->nb_lines * 12 * sizeof(float));

	    glUnmapBuffer(GL_ARRAY_BUFFER);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
}

void draw_line(gomo_t *gomo, vec3_t start, vec3_t end, vec3_t color) {
    static int i = 0;
    if (i >= V_BUFF_SIZE) {
        i = 0;
    }
    gomo->lines[i].start = start;
    gomo->lines[i].end = end;
    gomo->lines[i].color = color;
    // Update the line buffer
    int offset = i * 12;
    gomo->lines_buffer[offset]     = start.x;
    gomo->lines_buffer[offset + 1] = start.y;
    gomo->lines_buffer[offset + 2] = start.z;
    gomo->lines_buffer[offset + 3] = color.x;
    gomo->lines_buffer[offset + 4] = color.y;
    gomo->lines_buffer[offset + 5] = color.z;
    gomo->lines_buffer[offset + 6] = end.x;
    gomo->lines_buffer[offset + 7] = end.y;
    gomo->lines_buffer[offset + 8] = end.z;
    gomo->lines_buffer[offset + 9] = color.x;
    gomo->lines_buffer[offset + 10] = color.y;
    gomo->lines_buffer[offset + 11] = color.z;

    gomo->nb_lines++;
    if (gomo->nb_lines >= V_BUFF_SIZE) {
        gomo->nb_lines = V_BUFF_SIZE;
    }
    i++;
    update_lines_buffer(gomo);
}

void init_lines(gomo_t *gomo)
{
    gomo->nb_lines = 0;
    if (!(gomo->lines = (line_t *)malloc(sizeof(line_t) * V_BUFF_SIZE)))
        exit_callback(gomo, 9, "lines malloc failed");
    for (int i = 0; i < V_BUFF_SIZE; i++)
    {
        gomo->lines[i].start = (vec3_t){0.0f, 0.0f, 0.0f};
        gomo->lines[i].end = (vec3_t){0.0f, 0.0f, 0.0f};
        gomo->lines[i].color = (vec3_t){0.0f, 0.0f, 0.0f};
    }
    if (!(gomo->lines_buffer = (float *)malloc(sizeof(float) * V_BUFF_SIZE * 12)))
        exit_callback(gomo, 9, "lines buffer malloc failed");
    for (int i = 0; i < V_BUFF_SIZE * 12; i++)
    {
        gomo->lines_buffer[i] = 0.0f;
    }
    // draw gizmo lines
    draw_line(gomo, (vec3_t){0.0f, 0.0f, 0.0f}, (vec3_t){100.0f, 0.0f, 0.0f}, (vec3_t){1.0f, 0.0f, 0.0f}); // X axis
    draw_line(gomo, (vec3_t){0.0f, 0.0f, 0.0f}, (vec3_t){0.0f, 100.0f, 0.0f}, (vec3_t){0.0f, 1.0f, 0.0f}); // Y axis
    draw_line(gomo, (vec3_t){0.0f, 0.0f, 0.0f}, (vec3_t){0.0f, 0.0f, 100.0f}, (vec3_t){0.0f, 0.0f, 1.0f}); // Z axis

    for (int i = -19; i < 19; i += 2) {
        for (int j = -19; j < 19; j += 2)
        {
            draw_line(gomo, (vec3_t){(i + 1) * 0.0245f, 0.4f, (j + 1) * 0.0245f}, (vec3_t){(i + 1) * 0.0245f, 0.47f, (j + 1) * 0.0245f}, (vec3_t){1.0f, 0.0f, 1.0f});
        }
        
    }
}

void free_lines(gomo_t *gomo)
{
    if (gomo->lines) {
        free(gomo->lines);
        gomo->lines = NULL;
    }
    gomo->nb_lines = 0;
}