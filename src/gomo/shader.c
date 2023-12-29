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

char		*parse_shader_src(char *path)
{
	char	buf[V_BUFF_SIZE + 1];
	int		fd;
	char	*str;
	char	*tmp;
	int		i;

	if ((fd = open(path, O_RDONLY)) > 2)
	{
		str = NULL;
		while ((i = read(fd, buf, V_BUFF_SIZE)) > 0)
		{
			if (i > 0) {
				buf[i] = '\0';
				tmp = str;
				if (!(str = strjoin(str, buf)))
					return (NULL);
				if (tmp) {
					free(tmp);
					tmp = NULL;
				}
			} else {
				break;
			}
		}
		close(fd);
		return (str);
	}
	return (NULL);
}

void	init_vertexShader(gomo_t *gomo, shader_t *shader, char *vert_src)
{
	GLenum errCode;
	int success;

	if (!(shader->vertexShaderSrc = parse_shader_src(vert_src)))
		exit_callback(gomo, 27, "Shader vertex parsing failed");

	shader->vertexShader = glCreateShader(GL_VERTEX_SHADER);
	if ((errCode = glGetError()) != GL_NO_ERROR || !shader->vertexShader)
		exit_callback(gomo, 28, getErrorString(errCode));

	glShaderSource(shader->vertexShader, 1, &shader->vertexShaderSrc, NULL);
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 29, getErrorString(errCode));

	glCompileShader(shader->vertexShader);
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 30, getErrorString(errCode));

	glGetShaderiv(shader->vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
		exit_callback(gomo, 31, "Shader vertex compilation failed");
}

void	init_fragmentShader(gomo_t *gomo, shader_t *shader, char *frag_src)
{
	GLenum errCode;
	int success;

	if (!(shader->fragmentShaderSrc = parse_shader_src(frag_src)))
		exit_callback(gomo, 32, "Shader fragment parsing failed");

	shader->fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	if ((errCode = glGetError()) != GL_NO_ERROR || !shader->fragmentShader)
		exit_callback(gomo, 33, getErrorString(errCode));

	glShaderSource(shader->fragmentShader, 1, &shader->fragmentShaderSrc, NULL);
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 34, getErrorString(errCode));

	glCompileShader(shader->fragmentShader);
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 35, getErrorString(errCode));

	glGetShaderiv(shader->fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
		exit_callback(gomo, 36, "Shader fragment compilation failed");
}

void	init_shaderProgram(gomo_t *gomo, shader_t *shader)
{
	GLenum errCode;
	int success;

	shader->shaderProgram = glCreateProgram();
	if ((errCode = glGetError()) != GL_NO_ERROR || !shader->shaderProgram)
		exit_callback(gomo, 37, getErrorString(errCode));

	glAttachShader(shader->shaderProgram, shader->vertexShader);
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 38, getErrorString(errCode));

	glAttachShader(shader->shaderProgram, shader->fragmentShader);
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 39, getErrorString(errCode));

	glLinkProgram(shader->shaderProgram);
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 40, getErrorString(errCode));
	
	glGetProgramiv(shader->shaderProgram, GL_LINK_STATUS, &success); 	
	if (!success)
		exit_callback(gomo, 41, "Shader program compilation failed");
}

void	init_shader(gomo_t *gomo, shader_t *shader, char *vert_src, char *frag_src)
{
	GLenum errCode;

	init_vertexShader(gomo, shader, vert_src);
	init_fragmentShader(gomo, shader, frag_src);
	init_shaderProgram(gomo, shader);

	glDeleteShader(shader->vertexShader);
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 42, getErrorString(errCode));
	glDeleteShader(shader->fragmentShader);
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 43, getErrorString(errCode));
}
