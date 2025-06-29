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

typedef struct {
	const char *path;
	const char *uniform_name;
	int 		blend; // 1 for RGBA, 0 for RGB
} texture_info_t;

texture_info_t textures_path[NB_TEXTURES] = {
	{"resources/room_textures/grid.png", "grid_text", 1},
	{"resources/room_textures/wood.jpeg", "wood_text", 0},
	{"resources/room_textures/blk_stones.jpeg", "blk_stones_text", 0},
	{"resources/room_textures/containers.jpeg", "containers_text", 0},
	{"resources/room_textures/floor_1.jpeg", "floor_1_text", 0},
	{"resources/room_textures/floor_2.png", "floor_2_text", 0},
	{"resources/room_textures/footer.jpeg", "footer_text", 0},
	{"resources/room_textures/goboard.jpeg", "goboard_text", 0},
	{"resources/room_textures/house.jpeg", "house_text", 0},
	{"resources/room_textures/gogrid.png", "gogrid_text", 1},
	{"resources/room_textures/lantern.jpeg", "lantern_text", 0},
	{"resources/room_textures/roof.jpeg", "roof_text", 0},
	{"resources/room_textures/wall.jpeg", "wall_text", 0},
	{"resources/room_textures/wht_stones.jpeg", "wht_stones_text", 0},
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
	if (blend)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	}
	else
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	free_null(data);

	printf("Texture generation [%s]\n", path);
	return textureID;
}

void bind_texture(GLuint texture, GLuint shaderProgram, const char *uniformName, int textureUnit) {
    GLuint loc = glGetUniformLocation(shaderProgram, uniformName);
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(loc, textureUnit);
}

void	load_textures(gomo_t *gomo) {
    for (int i = 0; i < NB_TEXTURES; ++i) {
        gomo->textures[i] = load_image((char*)textures_path[i].path, textures_path[i].blend);
    }
    for (int i = 0; i < NB_TEXTURES; ++i) {
		bind_texture(gomo->textures[i], gomo->shader->shaderProgram, textures_path[i].uniform_name, i);
    }
}