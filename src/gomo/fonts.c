/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   fonts.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncoursol <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/04/04 16:59:13 by ncoursol          #+#    #+#             */
/*   Updated: 2022/05/01 19:24:40 by ncoursol         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/gomo.h"

void clear_text_to_render(gomo_t *gomo, int id)
{
    if (id < 0 || id >= NB_TEXT)
        return;
        
    if (gomo->text[id].text) {
        free(gomo->text[id].text);
        gomo->text[id].text = NULL;
    }
}

void add_text_to_render(gomo_t *gomo, char *font, char *text, vec3_t pos, vec3_t rotation, int face_camera, float scale, vec3_t color, int proj, int id)
{
    if (id < 0 || id >= NB_TEXT)
        return;
    
    // Only reallocate if text has changed or is NULL
    size_t new_len = strlen(text);
    if (!gomo->text[id].text || strcmp(gomo->text[id].text, text) != 0) {
        if (gomo->text[id].text) {
            free(gomo->text[id].text);
        }
        
        gomo->text[id].text = (char *)malloc(sizeof(char) * (new_len + 1));
        if (!gomo->text[id].text) {
            exit_callback(gomo, 8, "text malloc failed");
            return;
        }
        strcpy(gomo->text[id].text, text);
    }

    gomo->text[id].font = font;
    gomo->text[id].pos = pos;
    gomo->text[id].scale = scale;
    gomo->text[id].color = color;
    gomo->text[id].proj = (proj == 1) ? 1 : 0;
    gomo->text[id].rotation = rotation;
    gomo->text[id].face_camera = face_camera;
}

void render_text_2D_batched(gomo_t *gomo, font_t *font, char *text, vec3_t pos, float scale, vec3_t color)
{
    if (!font || !text) return;
    
    size_t text_length = strlen(text);
    if (text_length == 0) return;
    
    // Group characters by texture ID for batched rendering
    typedef struct {
        GLuint texture_id;
        size_t start_vertex;
        size_t vertex_count;
    } texture_batch_t;
    
    texture_batch_t batches[128]; // Max 128 different textures
    size_t batch_count = 0;
    
    // Pre-allocate vertex buffer for entire text
    size_t max_vertices = text_length * 6 * 5;
    float *vertices_buffer = malloc(sizeof(float) * max_vertices);
    if (!vertices_buffer) return;
    
    size_t total_vertices = 0;
    vec3_t cur = pos;
    GLuint last_texture = 0;
    
    // Build vertices and batch info
    for (unsigned int i = 0; i < text_length; i++)
    {
        if ((int)text[i] >= 0 && (int)text[i] < 128)
        {
            charact_t ch = font->characters[(int)text[i]];
            GLuint tex_id = ch.id;
            
            // Check if we need a new batch
            if (tex_id != last_texture) {
                if (batch_count < 128) {
                    batches[batch_count].texture_id = tex_id;
                    batches[batch_count].start_vertex = total_vertices;
                    batches[batch_count].vertex_count = 0;
                    batch_count++;
                }
                last_texture = tex_id;
            }
            
            float xpos = cur.x + ch.bearing_x * scale;
            float ypos = cur.y - (ch.height - ch.bearing_y) * scale;
            float w = ch.width * scale;
            float h = ch.height * scale;

            float char_vertices[6][5] = {
                {xpos,     ypos + h, 0.0f, 0.0f, 0.0f},
                {xpos,     ypos,     0.0f, 0.0f, 1.0f},
                {xpos + w, ypos,     0.0f, 1.0f, 1.0f},
                {xpos,     ypos + h, 0.0f, 0.0f, 0.0f},
                {xpos + w, ypos,     0.0f, 1.0f, 1.0f},
                {xpos + w, ypos + h, 0.0f, 1.0f, 0.0f}
            };
            
            memcpy(&vertices_buffer[total_vertices * 5], char_vertices, sizeof(char_vertices));
            total_vertices += 6;
            
            // Update current batch vertex count
            if (batch_count > 0) {
                batches[batch_count - 1].vertex_count += 6;
            }
            
            cur.x += (ch.advance >> 6) * scale;
        }
    }
    
    if (total_vertices > 0) {
        // Upload all vertices at once
        glBindVertexArray(font->VAO);
        glBindBuffer(GL_ARRAY_BUFFER, font->VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, total_vertices * 5 * sizeof(float), vertices_buffer);
        
        // Set color uniform once using cached location
        glUniform3f(gomo->shaderID.textColorID, color.x, color.y, color.z);
        
        // Render each batch with its texture
        for (size_t b = 0; b < batch_count; b++) {
            glBindTexture(GL_TEXTURE_2D, batches[b].texture_id);
            glDrawArrays(GL_TRIANGLES, batches[b].start_vertex, batches[b].vertex_count);
        }
        
        // Cleanup state
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    
    free(vertices_buffer);
}

void render_text(gomo_t *gomo, char *font, char *text, vec3_t pos, float scale, vec3_t color)
{
    // Find font with caching
    font_t *current_font = find_font_optimized(gomo, font);
    if (!current_font) return;
    
    // Always use batched rendering for better performance
    render_text_2D_batched(gomo, current_font, text, pos, scale, color);
}

// Cache for font lookup optimization
static font_t *last_used_font = NULL;
static char *last_font_name = NULL;

font_t *find_font_optimized(gomo_t *gomo, char *font_name)
{
    // Check cache first
    if (last_used_font && last_font_name && strcmp(last_font_name, font_name) == 0) {
        return last_used_font;
    }
    
    // Search for font
    font_t *current = gomo->fonts->first;
    while (current)
    {
        if (strcmp(current->name, font_name) == 0) {
            // Update cache
            last_used_font = current;
            last_font_name = font_name;
            return current;
        }
        if (current->next == NULL)
            break;
        current = current->next;
    }
    
    // Font not found, return first font as fallback
    return gomo->fonts->first;
}

void render_text_3D_batched(gomo_t *gomo, font_t *font, char *text, vec3_t pos, float scale, vec3_t color)
{
    if (!font || !text) return;
    
    size_t text_length = strlen(text);
    if (text_length == 0) return;
    
    // Group characters by texture ID for batched rendering
    typedef struct {
        GLuint texture_id;
        size_t start_vertex;
        size_t vertex_count;
    } texture_batch_t;
    
    texture_batch_t batches[128]; // Max 128 different textures
    size_t batch_count = 0;
    
    // Pre-allocate vertex buffer for entire text
    size_t max_vertices = text_length * 6 * 5;
    float *vertices_buffer = malloc(sizeof(float) * max_vertices);
    if (!vertices_buffer) return;
    
    size_t total_vertices = 0;
    vec3_t cur = pos;
    GLuint last_texture = 0;
    
    // Build vertices and batch info
    for (unsigned int i = 0; i < text_length; i++)
    {
        if ((int)text[i] >= 0 && (int)text[i] < 128)
        {
            charact_t ch = font->characters[(int)text[i]];
            GLuint tex_id = ch.id;
            
            // Check if we need a new batch
            if (tex_id != last_texture) {
                if (batch_count < 128) {
                    batches[batch_count].texture_id = tex_id;
                    batches[batch_count].start_vertex = total_vertices;
                    batches[batch_count].vertex_count = 0;
                    batch_count++;
                }
                last_texture = tex_id;
            }
            
            float xpos = cur.x + ch.bearing_x * scale;
            float ypos = cur.y - (ch.height - ch.bearing_y) * scale;
            float w = ch.width * scale;
            float h = ch.height * scale;
            float z = pos.z;

            float char_vertices[6][5] = {
                {xpos,     ypos + h, z, 0.0f, 0.0f},
                {xpos,     ypos,     z, 0.0f, 1.0f},
                {xpos + w, ypos,     z, 1.0f, 1.0f},
                {xpos,     ypos + h, z, 0.0f, 0.0f},
                {xpos + w, ypos,     z, 1.0f, 1.0f},
                {xpos + w, ypos + h, z, 1.0f, 0.0f}
            };
            
            memcpy(&vertices_buffer[total_vertices * 5], char_vertices, sizeof(char_vertices));
            total_vertices += 6;
            
            // Update current batch vertex count
            if (batch_count > 0) {
                batches[batch_count - 1].vertex_count += 6;
            }
            
            cur.x += (ch.advance >> 6) * scale;
        }
    }
    
    if (total_vertices > 0) {
        // Upload all vertices at once
        glBindVertexArray(font->VAO);
        glBindBuffer(GL_ARRAY_BUFFER, font->VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, total_vertices * 5 * sizeof(float), vertices_buffer);
        
        // Set color uniform once
        GLint colorLocation = glGetUniformLocation(gomo->shader->shaderProgramHUD, "textColor");
        glUniform3f(colorLocation, color.x, color.y, color.z);
        
        // Set texture sampler uniform once
        GLint samplerLocation = glGetUniformLocation(gomo->shader->shaderProgramHUD, "text");
        glUniform1i(samplerLocation, NB_TEXTURES);
        
        // Render each batch with its texture
        for (size_t b = 0; b < batch_count; b++) {
            glBindTexture(GL_TEXTURE_2D, batches[b].texture_id);
            glDrawArrays(GL_TRIANGLES, batches[b].start_vertex, batches[b].vertex_count);
        }
        
        // Cleanup state
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    
    free(vertices_buffer);
}

void render_text_3D(gomo_t *gomo, char *font, char *text, vec3_t pos, float scale, vec3_t color)
{
    // Find font with caching
    font_t *current_font = find_font_optimized(gomo, font);
    if (!current_font) return;
    
    // Use batched rendering for all strings (more efficient)
    size_t text_length = strlen(text);
    if (text_length <= 256) { // Use batched rendering for reasonable length strings
        render_text_3D_batched(gomo, current_font, text, pos, scale, color);
        return;
    }
    
    // Fallback to per-character rendering for mixed textures or long strings
    glBindVertexArray(current_font->VAO);
    glActiveTexture(GL_TEXTURE0 + NB_TEXTURES);
    
    // Set uniforms once per text string
    GLint colorLocation = glGetUniformLocation(gomo->shader->shaderProgramHUD, "textColor");
    glUniform3f(colorLocation, color.x, color.y, color.z);
    GLint samplerLocation = glGetUniformLocation(gomo->shader->shaderProgramHUD, "text");
    glUniform1i(samplerLocation, NB_TEXTURES);

    vec3_t cur = pos;
    for (unsigned int i = 0; i < text_length; i++)
    {
        if ((int)text[i] >= 0 && (int)text[i] < 128)
        {
            charact_t ch = current_font->characters[(int)text[i]];
            float xpos = cur.x + ch.bearing_x * scale;
            float ypos = cur.y - (ch.height - ch.bearing_y) * scale;
            float w = ch.width * scale;
            float h = ch.height * scale;
            float z = pos.z;
            
            float vertices[6][5] = {
                {xpos,     ypos + h, z, 0.0f, 0.0f},
                {xpos,     ypos,     z, 0.0f, 1.0f},
                {xpos + w, ypos,     z, 1.0f, 1.0f},
                {xpos,     ypos + h, z, 0.0f, 0.0f},
                {xpos + w, ypos,     z, 1.0f, 1.0f},
                {xpos + w, ypos + h, z, 1.0f, 0.0f}
            };

            glBindTexture(GL_TEXTURE_2D, ch.id);
            glBindBuffer(GL_ARRAY_BUFFER, current_font->VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            
            cur.x += (ch.advance >> 6) * scale;
        }
    }
    
    // Cleanup state
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

float calculate_text_width(gomo_t *gomo, char *font, char *text, float scale)
{
    // Find the correct font
    font_t *current_font = gomo->fonts->first;
    while (current_font)
    {
        if (strcmp(current_font->name, font) == 0)
            break;
        if (current_font->next == NULL)
            break;
        current_font = current_font->next;
    }
    
    float width = 0.0f;
    for (unsigned int i = 0; i < strlen(text); i++)
    {
        if ((int)text[i] >= 0 && (int)text[i] < 128)
        {
            charact_t ch = current_font->characters[(int)text[i]];
            width += (ch.advance >> 6) * scale;
        }
    }
    return width;
}

void render_all_text(gomo_t *gomo)
{
    // Batch rendering by setting up common state once
    glUseProgram(gomo->shader->shaderProgramHUD);
    
    // Set up global uniforms and texture sampler once using cached locations
    glUniform3fv(gomo->shaderID.playerPosID, 1, (float[3]){gomo->camera->eye.x, gomo->camera->eye.y, gomo->camera->eye.z});
    glUniform1i(gomo->shaderID.textSamplerID, NB_TEXTURES);
    glActiveTexture(GL_TEXTURE0 + NB_TEXTURES);
    
    // Separate 3D and 2D text rendering to minimize state changes
    
    // First pass: 3D text
    glUniformMatrix4fv(gomo->shaderID.projID, 1, GL_FALSE, &gomo->camera->mvp[0]);
    for (int i = 0; i < NB_TEXT; i++) {
        if (gomo->text[i].text != NULL && gomo->text[i].proj == 1) {
            vec3_t textCenter = {0.0f, 0.0f, 0.0f};
            vec3_t renderPos = gomo->text[i].pos;
            
            if (gomo->text[i].face_camera == 2) {
                textCenter = gomo->text[i].pos;
                renderPos = gomo->text[i].pos;
            } else {
                float textWidth = calculate_text_width(gomo, gomo->text[i].font, gomo->text[i].text, gomo->text[i].scale);
                float textHeight = 48.0f * gomo->text[i].scale;
                
                textCenter = gomo->text[i].pos;
                renderPos.x = gomo->text[i].pos.x - textWidth / 2.0f;
                renderPos.y = gomo->text[i].pos.y - textHeight / 2.0f;
                renderPos.z = gomo->text[i].pos.z;
            }
            
            // Set per-text uniforms for 3D text
            glUniform3fv(gomo->shaderID.cornerTextPosID, 1, (float[3]){textCenter.x, textCenter.y, textCenter.z});
            glUniform3fv(gomo->shaderID.textRotationID, 1, (float[3]){gomo->text[i].rotation.x, gomo->text[i].rotation.y, gomo->text[i].rotation.z});
            glUniform1i(gomo->shaderID.faceCameraID, gomo->text[i].face_camera);
            
            render_text_3D(gomo, gomo->text[i].font, gomo->text[i].text, renderPos, gomo->text[i].scale, gomo->text[i].color);
        }
    }
    
    // Second pass: 2D HUD text (if enabled)
    if (gomo->camera->options >> 0 & 1) {
        // First check if we have any 2D text to render
        int has_2d_text = 0;
        for (int i = 0; i < NB_TEXT && !has_2d_text; i++) {
            if (gomo->text[i].text != NULL && gomo->text[i].proj != 1) {
                has_2d_text = 1;
            }
        }
        
        // Only setup 2D rendering if we have 2D text
        if (has_2d_text) {
            glUniformMatrix4fv(gomo->shaderID.projID, 1, GL_FALSE, &gomo->camera->ortho[0]);
            
            // Clear 3D text uniforms to ensure 2D text doesn't get transformed
            glUniform3fv(gomo->shaderID.cornerTextPosID, 1, (float[3]){0.0f, 0.0f, 0.0f});
            glUniform3fv(gomo->shaderID.textRotationID, 1, (float[3]){0.0f, 0.0f, 0.0f});
            glUniform1i(gomo->shaderID.faceCameraID, 0);
            
            for (int i = 0; i < NB_TEXT; i++) {
                if (gomo->text[i].text != NULL && gomo->text[i].proj != 1) {
                    render_text(gomo, gomo->text[i].font, gomo->text[i].text, gomo->text[i].pos, gomo->text[i].scale, gomo->text[i].color);
                }
            }
        }
    }
}

void font_VAO(gomo_t *gomo)
{
    gomo->fonts = gomo->fonts->first;

    while (gomo->fonts)
    {
        glGenVertexArrays(1, &gomo->fonts->VAO);
        glBindVertexArray(gomo->fonts->VAO);
        glGenBuffers(1, &gomo->fonts->VBO);
        glBindBuffer(GL_ARRAY_BUFFER, gomo->fonts->VBO);
        
        // Allocate larger buffer to handle longer text strings (up to 256 characters)
        // Each character needs 6 vertices * 5 floats = 30 floats
        // 256 characters * 30 floats = 7680 floats
        size_t buffer_size = sizeof(float) * 256 * 6 * 5;
        glBufferData(GL_ARRAY_BUFFER, buffer_size, NULL, GL_DYNAMIC_DRAW);
        
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        if (gomo->fonts->next == NULL)
            break;
        gomo->fonts = gomo->fonts->next;
    }
}

void load_ttf(gomo_t *gomo, char *path, char *name)
{
    FT_Face face = NULL;
    if (gomo->fonts->path)
    {
        if (!(gomo->fonts->next = (font_t *)malloc(sizeof(font_t))))
            exit_callback(gomo, 222, "fonts malloc failed");
        gomo->fonts->next->first = gomo->fonts->first;
        gomo->fonts = gomo->fonts->next;
        gomo->fonts->path = NULL;
        gomo->fonts->next = NULL;
        if (!(gomo->fonts->characters = (charact_t *)malloc(sizeof(charact_t) * 128)))
            exit_callback(gomo, 8, "characters malloc failed");
    }

    gomo->fonts->path = path;
    gomo->fonts->name = name;

    if (FT_New_Face(gomo->ft, path, 0, &face))
        exit_callback(gomo, 223, "FT_New_Face failed");
    FT_Set_Pixel_Sizes(face, 0, 48);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (unsigned char c = 0; c < 128; c++)
    {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            exit_callback(gomo, 224, "FT_Load_Char failed");
            continue;
        }

        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        gomo->fonts->characters[c].id = textureID;
        gomo->fonts->characters[c].width = face->glyph->bitmap.width;
        gomo->fonts->characters[c].height = face->glyph->bitmap.rows;
        gomo->fonts->characters[c].bearing_x = face->glyph->bitmap_left;
        gomo->fonts->characters[c].bearing_y = face->glyph->bitmap_top;
        gomo->fonts->characters[c].advance = face->glyph->advance.x;
        gomo->fonts->characters[c].bitmap = face->glyph->bitmap.buffer;
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    FT_Done_Face(face);

    printf("Font generation [%s]\n", path);
}

void load_fonts(gomo_t *gomo)
{
    load_ttf(gomo, "resources/Fonts/texgyrecursor-regular.otf", "font_text1");
    load_ttf(gomo, "resources/Fonts/OxygenMono-Regular.otf", "font_text2");

    font_VAO(gomo);
    display_menu(gomo);
}