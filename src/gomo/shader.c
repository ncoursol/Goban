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
	char buf[V_BUFF_SIZE + 1];
	int fd;
	char *str;
	char *tmp;
	int i;

	if ((fd = open(path, O_RDONLY)) > 2)
	{
		str = NULL;
		while ((i = read(fd, buf, V_BUFF_SIZE)) > 0)
		{
			if (i > 0)
			{
				buf[i] = '\0';
				tmp = str;
				if (!(str = strjoin(str, buf)))
					return (NULL);
				if (tmp)
				{
					free(tmp);
					tmp = NULL;
				}
			}
			else
			{
				break;
			}
		}
		close(fd);
		return (str);
	}
	return (NULL);
}

void load_shader(gomo_t *gomo, unsigned int *shader, char *src, int type)
{
	GLenum errCode;
	int success;
	const char *tmp;

	if (!(tmp = parse_shader_src(src)))
		exit_callback(gomo, 27, "Shader parsing failed");

	*shader = glCreateShader(type);
	if ((errCode = glGetError()) != GL_NO_ERROR || !*shader)
		exit_callback(gomo, 28, getErrorString(errCode));

	glShaderSource(*shader, 1, &tmp, NULL);
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 29, getErrorString(errCode));

	glCompileShader(*shader);
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 30, getErrorString(errCode));

	glGetShaderiv(*shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		GLchar infoLog[512];
		glGetShaderInfoLog(*shader, sizeof(infoLog), NULL, infoLog);
		printf("error: %s\n", infoLog);
		exit_callback(gomo, 31, "Shader compilation failed");
	}
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
		{"src/shaders/vertex.glsl", "src/shaders/frag.glsl", &gomo->shader->shaderProgram},
		{"src/shaders/vertexStones.glsl", "src/shaders/fragStones.glsl", &gomo->shader->shaderProgramStones},
		{"src/shaders/vertexHUD.glsl", "src/shaders/fragHUD.glsl", &gomo->shader->shaderProgramHUD},
		{"src/shaders/vertexLine.glsl", "src/shaders/fragLine.glsl", &gomo->shader->shaderProgramLine}
	};
	
	int num_pairs = sizeof(shader_pairs) / sizeof(shader_pairs[0]);
	
	for (int i = 0; i < num_pairs; i++) {
		load_shader(gomo, &vertexShader, shader_pairs[i].vertexPath, GL_VERTEX_SHADER);
		load_shader(gomo, &fragmentShader, shader_pairs[i].fragmentPath, GL_FRAGMENT_SHADER);
		init_shaderProgram(gomo, shader_pairs[i].programPtr, &vertexShader, &fragmentShader);
	}
}
