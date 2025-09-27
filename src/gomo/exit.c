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
	if (gomo->shader) {
		if (gomo->shader->shaderProgram) {
			glDeleteProgram(gomo->shader->shaderProgram);
			gomo->shader->shaderProgram = 0;
		}
		if (gomo->shader->shaderProgramStones) {
			glDeleteProgram(gomo->shader->shaderProgramStones);
			gomo->shader->shaderProgramStones = 0;
		}
		if (gomo->shader->shaderProgramHUD) {
			glDeleteProgram(gomo->shader->shaderProgramHUD);
			gomo->shader->shaderProgramHUD = 0;
		}
		if (gomo->shader->shaderProgramLine) {
			glDeleteProgram(gomo->shader->shaderProgramLine);
			gomo->shader->shaderProgramLine = 0;
		}
	}
}

void	free_glfw(gomo_t *gomo, int step)
{
	if (step > 25 && gomo->window)
		glfwDestroyWindow(gomo->window);
	if (step > 24)
		glfwTerminate();
}

void	free_materials(material_t *materials)
{
	material_t *current, *next;
	
	if (!materials) return;
	
	current = materials->first;
	while (current != NULL) {
		next = current->next;
		if (current->name) {
			free(current->name);
		}
		if (current->texture) {
			free(current->texture);
		}
		free(current);
		current = next;
	}
}

void	free_obj(gomo_t *gomo)
{
	obj_t		*tmp;
	data_t	*tmp2;
	
	if (!gomo->obj) return;
	
	gomo->obj = gomo->obj->first;
	while (gomo->obj != NULL) {
		free_null((void*)gomo->obj->obj);
		if (gomo->obj->VBO)
			glDeleteBuffers(1, &gomo->obj->VBO);
		if (gomo->obj->VAO)
			glDeleteVertexArrays(1, &gomo->obj->VAO);
		
		// Free faces
		if (gomo->obj->faces) {
			for (int i = 0; i < gomo->obj->faces_size; i++) {
				if (gomo->obj->faces[i]) {
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
			}
			free_null((void*)gomo->obj->faces);
		}
		
		// Free materials
		if (gomo->obj->materials) {
			free_materials(gomo->obj->materials);
		}
		
		// Free materials_ids
		if (gomo->obj->materials_ids) {
			free(gomo->obj->materials_ids);
		}
		
		if (gomo->obj->next == NULL)
			break;
		tmp = gomo->obj->next;
		free_null((void*)gomo->obj);
		gomo->obj = tmp;
	}
}

void	free_textures(gomo_t *gomo) {
    for (int i = 0; i < NB_TEXTURES; ++i) {
        if (gomo->textures[i])
            glDeleteTextures(1, &gomo->textures[i]);
    }
}

void	free_gomo(gomo_t *gomo)
{
	free_null((void*)gomo->shader);
	free_null((void*)gomo->camera);
}

void	free_fonts(gomo_t *gomo)
{
	font_t *current, *next;
	
	if (!gomo->fonts) return;
	
	current = gomo->fonts->first;
	while (current != NULL) {
		next = current->next;
		
		// Don't free path and name - they are string literals, not malloc'd
		// current->path and current->name point to string constants
		
		if (current->characters) {
			// Delete OpenGL textures for each character
			for (int i = 0; i < 128; i++) {
				if (current->characters[i].id) {
					GLuint texture_id = (GLuint)current->characters[i].id;
					glDeleteTextures(1, &texture_id);
				}
				// Don't free bitmap - it's managed by FreeType
			}
			free(current->characters);
		}
		
		if (current->VAO)
			glDeleteVertexArrays(1, &current->VAO);
		if (current->VBO)
			glDeleteBuffers(1, &current->VBO);
		
		// Don't delete textureID here - individual character textures are deleted above
		
		free(current);
		current = next;
	}
}

void	free_all(gomo_t *gomo, int step)
{
	if (step > 5) {
		// Free camera matrices
		if (gomo->camera) {
			free_null((void*)gomo->camera->mvp);
			free_null((void*)gomo->camera->projection);
			free_null((void*)gomo->camera->ortho);
			free_null((void*)gomo->camera->view);
			free_null((void*)gomo->camera->model);
		}
		
		// Free textures
		free_textures(gomo);
		
		// Free objects
		if (gomo->obj)
		free_obj(gomo);
		
		// Free fonts
		if (gomo->fonts)
			free_fonts(gomo);
			
		// Free text array
		if (gomo->text) {
			for (int i = 0; i < NB_TEXT; i++) {
				// Don't free font - it's just a pointer to a string constant
				if (gomo->text[i].text) {
					free(gomo->text[i].text);
				}
			}
			free(gomo->text);
		}
		
		// Free game data
		if (gomo->game_data) {
			if (gomo->game_data->moves) {
				free(gomo->game_data->moves);
			}
			free(gomo->game_data);
		}
		
		// Free board
		free_null((void*)gomo->board);
		
		// Free stones
		free_null((void*)gomo->stone);
		
		// Free lines
		free_lines(gomo);
		
		// Delete OpenGL buffers
		if (gomo->lineVAO)
			glDeleteVertexArrays(1, &gomo->lineVAO);
		if (gomo->lineVBO)
			glDeleteBuffers(1, &gomo->lineVBO);
		if (gomo->instanceVBO)
			glDeleteBuffers(1, &gomo->instanceVBO);
		
		// Free shader
		free_shader(gomo);
		
		// Free GLFW and FreeType
		if (gomo->ft)
			FT_Done_FreeType(gomo->ft);
		free_glfw(gomo, step);
	}
	free_gomo(gomo);
}
