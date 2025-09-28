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

void add_text_to_render(gomo_t *gomo, char *font, char *text, vec3_t pos, float scale, vec3_t color, int proj, int id)
{
    if (id < 0 || id >= NB_TEXT)
        return;
        
    if (gomo->text[id].text) {
        free(gomo->text[id].text);
        gomo->text[id].text = NULL;
    }
    
    if (!(gomo->text[id].text = (char *)malloc(sizeof(char) * (strlen(text) + 1))))
        exit_callback(gomo, 8, "text malloc failed");

    gomo->text[id].font = font;
    strcpy(gomo->text[id].text, text);
    gomo->text[id].pos = pos;
    gomo->text[id].scale = scale;
    gomo->text[id].color = color;
    gomo->text[id].proj = (proj == 1) ? 1 : 0;
}

void render_text(gomo_t *gomo, char *font, char *text, vec3_t pos, float scale, vec3_t color)
{
    glUseProgram(gomo->shader->shaderProgramHUD);
    glUniformMatrix4fv(gomo->shaderID.projID, 1, GL_FALSE, &gomo->camera->ortho[0]);

    // Locate the correct font
    gomo->fonts = gomo->fonts->first;
    while (gomo->fonts)
    {
        if (strcmp(gomo->fonts->name, font) == 0)
            break;
        if (gomo->fonts->next == NULL)
            break;
        gomo->fonts = gomo->fonts->next;
    }

    // Bind the font's VAO
    glBindVertexArray(gomo->fonts->VAO);

    // Activate the texture unit (use unit 12 to avoid conflicts with regular textures 0-11)
    glActiveTexture(GL_TEXTURE0 + NB_TEXTURES);

    // Set the text color in the shader
    GLint colorLocation;
    GLint samplerLocation;

    colorLocation = glGetUniformLocation(gomo->shader->shaderProgramHUD, "textColor");
    glUniform3f(colorLocation, color.x, color.y, color.z);
    samplerLocation = glGetUniformLocation(gomo->shader->shaderProgramHUD, "text");
    glUniform1i(samplerLocation, NB_TEXTURES);

    // Render each character
    for (unsigned int i = 0; i < strlen(text); i++)
    {
        if ((int)text[i] >= 0 && (int)text[i] < 128)
        {
            charact_t ch = gomo->fonts->characters[(int)text[i]];
            float xpos = pos.x + ch.bearing_x * scale;
            float ypos = pos.y - (ch.height - ch.bearing_y) * scale;
            float w = ch.width * scale;
            float h = ch.height * scale;

            // Update VBO for each character
            // vertex format: x, y, z, u, v (5 floats)
            float vertices[6][5] = {
                {xpos,     ypos + h, 0.0f, 0.0f, 0.0f},
                {xpos,     ypos,     0.0f, 0.0f, 1.0f},
                {xpos + w, ypos,     0.0f, 1.0f, 1.0f},
                {xpos,     ypos + h, 0.0f, 0.0f, 0.0f},
                {xpos + w, ypos,     0.0f, 1.0f, 1.0f},
                {xpos + w, ypos + h, 0.0f, 1.0f, 0.0f}};

            // Bind the character texture
            glBindTexture(GL_TEXTURE_2D, ch.id);

            // Update the content of the VBO memory
            glBindBuffer(GL_ARRAY_BUFFER, gomo->fonts->VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            // Render the quad
            glDrawArrays(GL_TRIANGLES, 0, 6);

            // Advance cursor to the next glyph
            pos.x += (ch.advance >> 6) * scale;
        }
    }

    // Unbind the texture and VAO
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
}

void render_text_3D(gomo_t *gomo, char *font, char *text, vec3_t pos, float scale, vec3_t color)
{
    glUseProgram(gomo->shader->shaderProgramHUD);
    glUniformMatrix4fv(gomo->shaderID.projID, 1, GL_FALSE, &gomo->camera->mvp[0]);

    // Locate the correct font
    gomo->fonts = gomo->fonts->first;
    while (gomo->fonts)
    {
        if (strcmp(gomo->fonts->name, font) == 0)
            break;
        if (gomo->fonts->next == NULL)
            break;
        gomo->fonts = gomo->fonts->next;
    }

    // Bind the font's VAO
    glBindVertexArray(gomo->fonts->VAO);

    // Activate the texture unit
    glActiveTexture(GL_TEXTURE0 + NB_TEXTURES);

    // Set the text color in the shader
    GLint colorLocation = glGetUniformLocation(gomo->shader->shaderProgramHUD, "textColor");
    glUniform3f(colorLocation, color.x, color.y, color.z);
    GLint samplerLocation = glGetUniformLocation(gomo->shader->shaderProgramHUD, "text");
    glUniform1i(samplerLocation, NB_TEXTURES);


    // Render each character as a quad in world-space (XY plane at pos.z)
    vec3_t cur = pos; // use a cursor in world-space
    for (unsigned int i = 0; i < strlen(text); i++)
    {
        if ((int)text[i] >= 0 && (int)text[i] < 128)
        {
            charact_t ch = gomo->fonts->characters[(int)text[i]];
            float xpos = cur.x + ch.bearing_x * scale;
            float ypos = cur.y - (ch.height - ch.bearing_y) * scale;
            float w = ch.width * scale;
            float h = ch.height * scale;

            // Build 3D quad at Z = pos.z
            float z = pos.z;
            float vertices[6][5] = {
                {xpos,     ypos + h, z, 0.0f, 0.0f},
                {xpos,     ypos,     z, 0.0f, 1.0f},
                {xpos + w, ypos,     z, 1.0f, 1.0f},
                {xpos,     ypos + h, z, 0.0f, 0.0f},
                {xpos + w, ypos,     z, 1.0f, 1.0f},
                {xpos + w, ypos + h, z, 1.0f, 0.0f}};

            // Bind the character texture
            glBindTexture(GL_TEXTURE_2D, ch.id);

            // Update VBO
            glBindBuffer(GL_ARRAY_BUFFER, gomo->fonts->VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            // Draw
            glDrawArrays(GL_TRIANGLES, 0, 6);

            // Advance cursor
            cur.x += (ch.advance >> 6) * scale;
        }
    }
    
    
    // Unbind
    glBindTexture(GL_TEXTURE_2D, 0);
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
    for (int i = 0; i < NB_TEXT; i++) {
        if (gomo->text[i].text != NULL) {
            vec3_t textCenter = {0.0f, 0.0f, 0.0f};
            vec3_t renderPos = gomo->text[i].pos;
            
            if (gomo->text[i].proj == 1) {
                float textWidth = calculate_text_width(gomo, gomo->text[i].font, gomo->text[i].text, gomo->text[i].scale);
                float textHeight = 48.0f * gomo->text[i].scale; // Using font size of 48
                
                textCenter = gomo->text[i].pos; // This is the center position
                renderPos.x = gomo->text[i].pos.x - textWidth / 2.0f;  // Start position for text rendering
                renderPos.y = gomo->text[i].pos.y - textHeight / 2.0f;
                renderPos.z = gomo->text[i].pos.z;
            }
            
            glUseProgram(gomo->shader->shaderProgramHUD);
            glUniform3fv(gomo->shaderID.cornerTextPosID, 1, (float[3]){textCenter.x, textCenter.y, textCenter.z});
            glUniform3fv(gomo->shaderID.playerPosID, 1, (float[3]){gomo->camera->eye.x, gomo->camera->eye.y, gomo->camera->eye.z});
            
            if (gomo->text[i].proj == 1)
                render_text_3D(gomo, gomo->text[i].font, gomo->text[i].text, renderPos, gomo->text[i].scale, gomo->text[i].color);
            else if (HUD)
                render_text(gomo, gomo->text[i].font, gomo->text[i].text, gomo->text[i].pos, gomo->text[i].scale, gomo->text[i].color);
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
        // store 5 floats per vertex: x,y,z,u,v
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 5, NULL, GL_DYNAMIC_DRAW);
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
    add_text_to_render(gomo, "font_text2", "Main Menu", (vec3_t){0.0f, 1.5f, 0.0f}, 0.003f, (vec3_t){1.0f, 0.5f, 0.5f}, 1, 2);
	add_text_to_render(gomo, "font_text2", "Human VS Human", (vec3_t){0.0f, 1.00f, 0.0f}, 0.003f, (vec3_t){1.0f, 0.5f, 0.5f}, 1, 3);
	add_text_to_render(gomo, "font_text2", "Human VS IA", (vec3_t){0.0f, 0.80f, 0.0f}, 0.003f, (vec3_t){1.0f, 0.5f, 0.5f}, 1, 4);


}