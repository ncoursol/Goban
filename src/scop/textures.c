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

#include "../include/scop.h"

GLuint loadBMP_file(scop_t *scop, char *path)
{
	FILE	*file;	
	char 	header[54];
	int 	dataPos;
	int 	imageSize;
	int 	width, height;
	char 	*data;

	if (!(file = fopen(path,"rb")))
		exit_callback(scop, 52, "wrong texture file (need .bmp)");

	if (fread(header, 1, 54, file) != 54)
		exit_callback(scop, 53, "wrong .bmp file");

	if (header[0] != 'B' || header[1] != 'M')
		exit_callback(scop, 54, "wrong .bmp file");

	if (*(int*)&(header[0x1E]) != 0)
		exit_callback(scop, 55, "wrong .bmp file");

	if (*(int*)&(header[0x1C]) != 24)
		exit_callback(scop, 56, "wrong .bmp file");

	dataPos = *(int*)&(header[0x0A]);
	imageSize = *(int*)&(header[0x22]);
	width = *(int*)&(header[0x12]);
	height = *(int*)&(header[0x16]);

	if (imageSize == 0)
		imageSize = width * height * 3;
	if (dataPos == 0)
		dataPos = 54;

	if (!(data = (char*)malloc(sizeof(char) * imageSize)))
		exit_callback(scop, 57, "texture data malloc failed");

	fread(data, 1, imageSize, file);
	fclose (file);

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	free_null(data);
	return textureID;
}

void	load_texture(scop_t *scop, char *path) {
	scop->texture = loadBMP_file(scop, path);
	scop->shaderID.textureID = glGetUniformLocation(scop->shader->shaderProgram, "myTextureSampler");
}
