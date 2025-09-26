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

void add_text_to_render(gomo_t *gomo, char *font, char *text, vec3_t pos, float scale, vec3_t color, int id)
{
    if (gomo->text[id].text)
        free(gomo->text[id].text);
    if (!(gomo->text[id].text = (char *)malloc(sizeof(char) * (strlen(text) + 1))))
        exit_callback(gomo, 8, "text malloc failed");

    gomo->text[id].font = font;
    memcpy(gomo->text[id].text, text, strlen(text) + 1);
    gomo->text[id].pos = pos;
    gomo->text[id].scale = scale;
    gomo->text[id].color = color;
}

void render_text(gomo_t *gomo, char *font, char *text, vec3_t pos, float scale, vec3_t color)
{

    glUseProgram(gomo->shader->shaderProgramHUD);
    glUniformMatrix4fv(gomo->shaderID.orthoID, 1, GL_FALSE, &gomo->camera->ortho[0]);

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
            float vertices[6][4] = {
                {xpos, ypos + h, 0.0f, 0.0f},
                {xpos, ypos, 0.0f, 1.0f},
                {xpos + w, ypos, 1.0f, 1.0f},
                {xpos, ypos + h, 0.0f, 0.0f},
                {xpos + w, ypos, 1.0f, 1.0f},
                {xpos + w, ypos + h, 1.0f, 0.0f}};

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

void render_all_text(gomo_t *gomo)
{
    for (int i = 0; i < NB_TEXT; i++) {
        if (gomo->text[i].text != NULL)
            render_text(gomo, gomo->text[i].font, gomo->text[i].text, gomo->text[i].pos, gomo->text[i].scale, gomo->text[i].color);
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
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
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

    FT_Done_FreeType(gomo->ft);
}