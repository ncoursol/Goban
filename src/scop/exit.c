/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exit.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncoursol <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/04/04 16:59:08 by ncoursol          #+#    #+#             */
/*   Updated: 2022/04/17 18:00:44 by ncoursol         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/scop.h"

void	free_shader(scop_t *scop)
{
	free_null((void*)scop->shader->vertexShaderSrc);
	free_null((void*)scop->shader->fragmentShaderSrc);
	if (scop->shader->shaderProgram)
		glDeleteProgram(scop->shader->shaderProgram);
}

void	free_glfw(scop_t *scop, int step)
{
	if (step > 25 && scop->window)
		glfwDestroyWindow(scop->window);
	if (step > 24)
		glfwTerminate();
}

void	free_obj(scop_t *scop)
{
	obj_t		*tmp;
	data_t	*tmp2;
	scop->obj = scop->obj->first;
	while (scop->obj != NULL) {
		free_null((void*)scop->obj->name);
		free_null((void*)scop->obj->matName);
		free_null((void*)scop->obj->matDef);
		free_null((void*)scop->obj->obj);
		if (scop->obj->VBO)
			glDeleteBuffers(1, &scop->obj->VBO);
		if (scop->obj->VAO)
			glDeleteVertexArrays(1, &scop->obj->VAO);
		for (int i = 0; i < scop->obj->faces_size; i++) {
			scop->obj->faces[i] = scop->obj->faces[i]->first;
			while (scop->obj->faces[i] != NULL) {
				if (scop->obj->faces[i]->next == NULL) {
					free_null((void*)scop->obj->faces[i]);
					break;
				}
				tmp2 = scop->obj->faces[i]->next;
				free_null((void*)scop->obj->faces[i]);
				scop->obj->faces[i] = tmp2;
			}
		}
		free_null((void*)scop->obj->faces);
		if (scop->obj->next == NULL)
			break;
		tmp = scop->obj->next;
		free_null((void*)scop->obj);
		scop->obj = tmp;
	}
}

void	free_scop(scop_t *scop)
{
	free_null((void*)scop->obj);
	free_null((void*)scop->shader);
	free_null((void*)scop->camera);
	free_null((void*)scop->vertices);
	free_null((void*)scop->textures);
	free_null((void*)scop->normals);
}

void	free_all(scop_t *scop, int step)
{
	if (step > 5) {
		free_null((void*)scop->camera->mvp);
		if (scop->texture)
			glDeleteTextures(1, &scop->texture);
		if (scop->obj)
			free_obj(scop);
		free_shader(scop);
		free_glfw(scop, step);
	}
	free_scop(scop);
}
