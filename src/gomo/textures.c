/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   textures.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncoursol <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/04/04 16:59:08 by ncoursol          #+#    #+#             */
/*   Updated: 2022/04/17 18:00:44 by ncoursol         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/gomo.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../../include/stb_image.h"

texture_info_t textures_path[NB_TEXTURES] = {
	{"resources/textures/sky.jpg", "sky_text", 0},
	{"resources/textures/blackStones_10.jpeg", "blk_stones_text", 0},
	{"resources/textures/containersBake_7.jpeg", "containers_text", 0},
	{"resources/textures/floor_3.png", "floor_1_text", 1},
	{"resources/textures/floor_2.jpeg", "floor_2_text", 0},
	{"resources/textures/footerBake_sm_1.jpeg", "footer_text", 0},
	{"resources/textures/goboardBake_11.jpeg", "goboard_text", 0},
	{"resources/textures/HouseFrame_4.jpeg", "house_text", 0},
	{"resources/textures/Image_3.png", "gogrid_text", 1},
	{"resources/textures/RoofBake_sm_0.jpeg", "roof_text", 0},
	{"resources/textures/WallAndPaper_5.jpeg", "wall_text", 0},
	{"resources/textures/whiteStones_9.jpeg", "wht_stones_text", 0},
};

GLuint load_image(char *path, int blend)
{
    int width, height, nrChannels;
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (!data)
    {
        fprintf(stderr, "Failed to load texture at path: %s\n", path);
        return 0;
    }
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    GLenum format = GL_RGB;
    if (nrChannels == 1)
        format = GL_RED;
    else if (nrChannels == 2)
        format = GL_RG;
    else if (nrChannels == 3)
        format = GL_RGB;
    else if (nrChannels == 4)
        format = GL_RGBA;

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    // Set wrapping mode
    if ((format == GL_RGBA || format == GL_RG) && blend)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        float borderColor[] = {0.0f, 0.0f, 0.0f, 0.0f};
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    free_null(data);
    printf("Texture generation [%s] (blend: %d)\n", path, blend);
    return textureID;
}

void load_textures(gomo_t *gomo) {
	glUseProgram(gomo->shader->shaderProgram);
    for (int i = 0; i < NB_TEXTURES; i++) {
        gomo->textures[i] = load_image((char*)textures_path[i].path, textures_path[i].blend);
    }

    // Bind all textures to the shader as an array
    GLint texUniform = glGetUniformLocation(gomo->shader->shaderProgram, "textures");
    if (texUniform == -1) {
        fprintf(stderr, "Could not find 'textures' uniform array in shader\n");
        return;
    }
    GLint samplers[NB_TEXTURES];
    for (int i = 0; i < NB_TEXTURES; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, gomo->textures[i]);
        samplers[i] = i;
    }
    glUniform1iv(texUniform, NB_TEXTURES, samplers);
    glActiveTexture(GL_TEXTURE0 + NB_TEXTURES);
}