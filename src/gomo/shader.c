/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   shader.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncoursol <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/04/04 16:59:13 by ncoursol          #+#    #+#             */
/*   Updated: 2022/05/01 19:24:40 by ncoursol         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/gomo.h"

char *parse_shader_src(char *path)
{
	FILE *file;
	char *str = NULL;
	long file_size;
	
	file = fopen(path, "r");
	if (!file)
		return NULL;
	
	// Get file size
	fseek(file, 0, SEEK_END);
	file_size = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	if (file_size <= 0) {
		fclose(file);
		return NULL;
	}
	
	// Allocate buffer for entire file
	str = (char*)malloc(file_size + 1);
	if (!str) {
		fclose(file);
		return NULL;
	}
	
	// Read entire file
	size_t bytes_read = fread(str, 1, file_size, file);
	str[bytes_read] = '\0';
	
	fclose(file);
	return str;
}

void load_shader(gomo_t *gomo, unsigned int *shader, char *src, int type)
{
	GLenum errCode;
	int success;
	char *shader_source;

	if (!(shader_source = parse_shader_src(src)))
		exit_callback(gomo, 27, "Shader parsing failed");

	*shader = glCreateShader(type);
	if ((errCode = glGetError()) != GL_NO_ERROR || !*shader) {
		free(shader_source);
		exit_callback(gomo, 28, getErrorString(errCode));
	}

	const char *tmp = shader_source;
	glShaderSource(*shader, 1, &tmp, NULL);
	if ((errCode = glGetError()) != GL_NO_ERROR) {
		free(shader_source);
		exit_callback(gomo, 29, getErrorString(errCode));
	}

	glCompileShader(*shader);
	if ((errCode = glGetError()) != GL_NO_ERROR) {
		free(shader_source);
		exit_callback(gomo, 30, getErrorString(errCode));
	}

	glGetShaderiv(*shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		GLchar infoLog[512];
		glGetShaderInfoLog(*shader, sizeof(infoLog), NULL, infoLog);
		printf("error: %s\n", infoLog);
		free(shader_source);
		exit_callback(gomo, 31, "Shader compilation failed");
	}
	
	free(shader_source);
}

void init_shaderProgram(gomo_t *gomo, unsigned int *prog, unsigned int *vert, unsigned int *frag)
{
	GLenum errCode;
	int success;

	*prog = glCreateProgram();
	if ((errCode = glGetError()) != GL_NO_ERROR || !*prog)
		exit_callback(gomo, 37, getErrorString(errCode));

	glAttachShader(*prog, *vert);
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 38, getErrorString(errCode));

	glAttachShader(*prog, *frag);
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 40, getErrorString(errCode));

	glLinkProgram(*prog);
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 41, getErrorString(errCode));

	glGetProgramiv(*prog, GL_LINK_STATUS, &success);
	if (!success)
	{
		GLchar infoLog[512];
		glGetProgramInfoLog(*prog, sizeof(infoLog), NULL, infoLog);
		exit_callback(gomo, 42, infoLog);
	}
	glDeleteShader(*vert);
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 43, getErrorString(errCode));
	glDeleteShader(*frag);
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 44, getErrorString(errCode));
}

void init_shader(gomo_t *gomo)
{
	unsigned int vertexShader;
	unsigned int fragmentShader;
	
	typedef struct {
		char *vertexPath;
		char *fragmentPath;
		unsigned int *programPtr;
	} shader_pair_t;
	
	shader_pair_t shader_pairs[] = {
		{"src/gomo/shaders/vertex.glsl", "src/gomo/shaders/frag.glsl", &gomo->shader->shaderProgram},
		{"src/gomo/shaders/vertexStones.glsl", "src/gomo/shaders/fragStones.glsl", &gomo->shader->shaderProgramStones},
		{"src/gomo/shaders/vertexHUD.glsl", "src/gomo/shaders/fragHUD.glsl", &gomo->shader->shaderProgramHUD},
		{"src/gomo/shaders/vertexLine.glsl", "src/gomo/shaders/fragLine.glsl", &gomo->shader->shaderProgramLine}
	};
	
	int num_pairs = sizeof(shader_pairs) / sizeof(shader_pairs[0]);
	
	for (int i = 0; i < num_pairs; i++) {
		load_shader(gomo, &vertexShader, shader_pairs[i].vertexPath, GL_VERTEX_SHADER);
		load_shader(gomo, &fragmentShader, shader_pairs[i].fragmentPath, GL_FRAGMENT_SHADER);
		init_shaderProgram(gomo, shader_pairs[i].programPtr, &vertexShader, &fragmentShader);
	}
}
