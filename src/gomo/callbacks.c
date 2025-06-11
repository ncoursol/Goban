/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   callbacks.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncoursol <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/04/13 16:01:06 by ncoursol          #+#    #+#             */
/*   Updated: 2023/02/27 15:04:54 by ncoursol         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/gomo.h"

char *getErrorString(int code)
{
	if (code == 0x0500)
		return ("GL_INVALID_ENUM");
	else if (code == 0x0501)
		return ("GL_INVALID_VALUE");
	else if (code == 0x0502)
		return ("GL_INVALID_OPERATION");
	else if (code == 0x0503)
		return ("GL_STACK_OVERFLOW");
	else if (code == 0x0504)
		return ("GL_STACK_UNDERFLOW");
	else if (code == 0x0505)
		return ("GL_OUT_OF_MEMORY");
	else if (code == 0x0506)
		return ("GL_INVALID_FRAMEBUFFER_OPERATION");
	else if (code == 0x0507)
		return ("GL_CONTEXT_LOST");
	else if (code == 0x8031)
		return ("GL_TABLE_TOO_LARGE1");
	else
		return ("GL_ERROR");
	return ("GL_ERROR");
}

void exit_callback(gomo_t *gomo, int state, char *description)
{
	fprintf(stderr, "Error: %s [%d]\n", description, state);
	free_all(gomo, state);
	exit(1);
}

void updateData(gomo_t *gomo)
{
	GLenum errCode;

	gomo->obj = gomo->obj->first->next;
	glBindVertexArray(gomo->obj->VAO);
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 218, getErrorString(errCode));
	glBindBuffer(GL_ARRAY_BUFFER, gomo->instanceVBO);
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 219, getErrorString(errCode));

	instance_t *data = (instance_t *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 220, getErrorString(errCode));

	memcpy(data, &gomo->stone[0], gomo->nb_stones * sizeof(instance_t));

	glUnmapBuffer(GL_ARRAY_BUFFER);
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 221, getErrorString(errCode));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

int intersect(ray_t ray, hit_t *intersection)
{
    float dDotN = dot_vec3(ray.direction, (vec3_t){0.0f, 1.0f, 0.0f});

    // Use epsilon for floating-point comparison
    if (fabsf(dDotN) < 1e-6f)
        return 0;

    float t = (5.7f - ray.origin.y) / ray.direction.y;

    // Ignore intersections behind the ray origin
    if (t < 0)
        return 0;

    intersection->point.x = ray.origin.x + t * ray.direction.x;
    intersection->point.y = -0.26f; // Goban height
    intersection->point.z = ray.origin.z + t * ray.direction.z;

    return 1;
}

ray_t createRay(gomo_t *gomo, double xpos, double ypos)
{
    ray_t ray;
    char tmp[200];

    // Convert screen coordinates to normalized device coordinates (NDC)
    float ndcX = (2.0f * (float)xpos) / (float)WIDTH - 1.0f;
    float ndcY = 1.0f - (2.0f * (float)ypos) / (float)HEIGHT;
	vec4_t ray_clip = {ndcX, ndcY, -1.0f, 1.0f};
	vec4_t ray_eye = mulv_mat4(inv_mat4(gomo->camera->projection), ray_clip);
	ray_eye = (vec4_t){ray_eye.x, ray_eye.y, -1.0f, 0.0f};
	vec4_t ray_world = mulv_mat4(inv_mat4(gomo->camera->view), ray_eye);
	ray.direction = norm_vec3((vec3_t){ray_world.x, ray_world.y, ray_world.z});
	
    ray.origin = gomo->camera->eye;
	ray.direction = norm_vec3(ray.direction); // Normalize the direction vector

    // Debug text
    sprintf(tmp, "mouse NDC: x[%f] y[%f]", ndcX, ndcY);
    add_text_to_render(gomo, "font_text2", tmp, (vec3_t){5, HEIGHT - 50, 0.0f}, 0.3f, (vec3_t){0.9f, 0.9f, 0.9f}, 2);
    sprintf(tmp, "origin    : x[%f] y[%f] z[%f]", ray.origin.x, ray.origin.y, ray.origin.z);
    add_text_to_render(gomo, "font_text2", tmp, (vec3_t){5, HEIGHT - 80, 0.0f}, 0.3f, (vec3_t){0.9f, 0.9f, 0.9f}, 3);
    sprintf(tmp, "direction : x[%f] y[%f] z[%f]", ray.direction.x, ray.direction.y, ray.direction.z);
    add_text_to_render(gomo, "font_text2", tmp, (vec3_t){5, HEIGHT - 110, 0.0f}, 0.3f, (vec3_t){0.9f, 0.9f, 0.9f}, 4);

    return ray;
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
	(void)mods;
	if (button == GLFW_MOUSE_BUTTON_RIGHT && (action == GLFW_PRESS || action == GLFW_RELEASE))
	{
		gomo_t *gomo = glfwGetWindowUserPointer(window);
		if (action == GLFW_PRESS && gomo->nb_stones < 361)
		{
			if (gomo->tmp_stone && !gomo->board[gomo->tmp_stone - 1].state) {
				gomo->board[gomo->tmp_stone - 1].state = 1;
				gomo->stone[gomo->nb_stones - 1].color = gomo->nb_stones % 2 ? (vec3_t){0.0f, 0.0f, 0.0f} : (vec3_t){1.0f, 1.0f, 1.0f};
				gomo->tmp_stone = 0;
			}
		}
	} else if (button == GLFW_MOUSE_BUTTON_LEFT && (action == GLFW_PRESS || action == GLFW_RELEASE))
	{
		gomo_t *gomo = glfwGetWindowUserPointer(window);
		if (!(gomo->camera->options >> 8 & 1))
		{
			gomo->camera->options ^= 1 << 5;
			if (action == GLFW_PRESS)
			{
				glfwSetInputMode(gomo->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				glfwSetCursorPos(gomo->window, gomo->camera->ah / MSPEED, gomo->camera->av / MSPEED);
			}
			else if (action == GLFW_RELEASE)
			{
				glfwSetInputMode(gomo->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}
		}
	}
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
	(void)xoffset;
	if (yoffset)
	{
		gomo_t *gomo = glfwGetWindowUserPointer(window);
		if (!(TOP_VIEW))
		{
			float zoom = yoffset * (log(gomo->camera->dist + 7.5) - 2);
			if (gomo->camera->dist - zoom >= 0 && gomo->camera->dist - zoom <= 150)
				gomo->camera->dist -= zoom;
		}
	}
}

void mouse_move_callback(GLFWwindow *window, double xpos, double ypos)
{
	gomo_t *gomo = glfwGetWindowUserPointer(window);
	if (gomo->nb_stones < 361)
	{
		ray_t ray;
		hit_t res;
		ray = createRay(gomo, xpos, ypos);
		gomo->obj = gomo->obj->first;
		if (intersect(ray, &res))
		{
			gomo->obj = gomo->obj->next;
			int closest_case = find_closest_case(gomo, res.point);
			if (closest_case >= 0 && gomo->board[closest_case].state == 0)
			{
				if (!gomo->tmp_stone) {
					gomo->nb_stones++;
				}
				gomo->tmp_stone = closest_case + 1;
				gomo->stone[gomo->nb_stones - 1].pos = gomo->board[closest_case].pos;
				gomo->stone[gomo->nb_stones - 1].color = gomo->nb_stones % 2 ? (vec3_t){0.3f, 0.3f, 0.3f} : (vec3_t){0.9f, 0.9f, 0.9f};
			} else if (gomo->tmp_stone) {
				gomo->tmp_stone = 0;
				gomo->nb_stones--;
			}
			updateData(gomo);
		}
	}
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	(void)mods;
	(void)scancode;
	if (action == GLFW_PRESS)
	{
		gomo_t *gomo = glfwGetWindowUserPointer(window);
		if (key == GLFW_KEY_ESCAPE)
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		else if (key == GLFW_KEY_H)
			gomo->camera->options ^= 1 << 0;
		else if (key == GLFW_KEY_W)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else if (key == GLFW_KEY_F)
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		else if (key == GLFW_KEY_P)
			glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		else if (key == GLFW_KEY_SPACE)
		{
			free_null((void *)gomo->camera->mvp);
			init_camera(gomo);
		}
		else if (key == GLFW_KEY_V)
		{
			gomo->camera->options ^= 1 << 8; // top view
		}
	}
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
	(void)window;
	glViewport(0, 0, width, height);
}
