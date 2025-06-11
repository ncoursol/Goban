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

#include "../include/gomo.h"

void	free_shader(gomo_t *gomo)
{
	if (gomo->shader->shaderProgram)
		glDeleteProgram(gomo->shader->shaderProgram);
}

void	free_glfw(gomo_t *gomo, int step)
{
	if (step > 25 && gomo->window)
		glfwDestroyWindow(gomo->window);
	if (step > 24)
		glfwTerminate();
}

void	free_obj(gomo_t *gomo)
{
	obj_t		*tmp;
	data_t	*tmp2;
	gomo->obj = gomo->obj->first;
	while (gomo->obj != NULL) {
		free_null((void*)gomo->obj->obj);
		if (gomo->obj->VBO)
			glDeleteBuffers(1, &gomo->obj->VBO);
		if (gomo->obj->VAO)
			glDeleteVertexArrays(1, &gomo->obj->VAO);
		for (int i = 0; i < gomo->obj->faces_size; i++) {
			gomo->obj->faces[i] = gomo->obj->faces[i]->first;
			while (gomo->obj->faces[i] != NULL) {
				if (gomo->obj->faces[i]->next == NULL) {
					free_null((void*)gomo->obj->faces[i]);
					break;
				}
				tmp2 = gomo->obj->faces[i]->next;
				free_null((void*)gomo->obj->faces[i]);
				gomo->obj->faces[i] = tmp2;
			}
		}
		free_null((void*)gomo->obj->faces);
		if (gomo->obj->next == NULL)
			break;
		tmp = gomo->obj->next;
		free_null((void*)gomo->obj);
		gomo->obj = tmp;
	}
}

void	free_gomo(gomo_t *gomo)
{
	free_null((void*)gomo->obj);
	free_null((void*)gomo->shader);
	free_null((void*)gomo->camera);
}

void	free_all(gomo_t *gomo, int step)
{
	if (step > 5) {
		free_null((void*)gomo->camera->mvp);
		if (gomo->grid_text)
			glDeleteTextures(1, &gomo->grid_text);
		if (gomo->wood_text)
			glDeleteTextures(1, &gomo->wood_text);
		if (gomo->obj)
			free_obj(gomo);
		free_shader(gomo);
		free_glfw(gomo, step);
		free_lines(gomo);
		free_null((void*)gomo->lines);
	}
	free_gomo(gomo);
}
