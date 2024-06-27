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

	// check if the ray is embedded to the plane
	if (dDotN == 0.0f)
		return 0;

	float t = -ray.origin.y / ray.direction.y;

	// Calculate intersection point
	intersection->point.x = ray.origin.x + t * ray.direction.x;
	intersection->point.y = 0; // Plane is at y = 0
	intersection->point.z = ray.origin.z + t * ray.direction.z;

	return 1;
}

ray_t createRay(gomo_t *gomo, double xpos, double ypos)
{
	ray_t ray;
	vec3_t direction, forward, right, up;
	float h, w;
	char tmp[200];

	float normalizedX = (2.0f * xpos) / WIDTH - 1.0f;
	float normalizedY = 1.0f - (2.0f * ypos) / HEIGHT;

	// Calculate forward, right, and up vectors
	forward = norm_vec3(sub_vec3(gomo->camera->center, gomo->camera->eye));
	right = norm_vec3(cross_vec3(forward, (vec3_t){0.0f, 1.0f, 0.0f}));
	up = cross_vec3(right, forward);

	h = tanf(RAD(gomo->camera->fov) / 2.0f);
	w = h * (WIDTH / HEIGHT);

	// Compute the direction
	direction = add_vec3(add_vec3(forward, prod_vec3(right, normalizedX * w)), prod_vec3(up, normalizedY * h));

	ray.origin = gomo->camera->eye;
	ray.direction = norm_vec3(direction);

	sprintf(tmp, "mouse :  x[%f] y[%f]", normalizedX, normalizedY);
	add_text_to_render(gomo, "font_text2", tmp, (vec3_t){5, HEIGHT - 50, 0.0f}, 0.3f, (vec3_t){0.9f, 0.9f, 0.9f}, 2);

	sprintf(tmp, "forward : x[%f] y[%f] z[%f]", forward.x, forward.y, forward.z);
	add_text_to_render(gomo, "font_text2", tmp, (vec3_t){5, HEIGHT - 90, 0.0f}, 0.3f, (vec3_t){0.9f, 0.9f, 0.9f}, 3);
	sprintf(tmp, "right   : x[%f] y[%f] z[%f]", right.x, right.y, right.z);
	add_text_to_render(gomo, "font_text2", tmp, (vec3_t){5, HEIGHT - 110, 0.0f}, 0.3f, (vec3_t){0.9f, 0.9f, 0.9f}, 4);
	sprintf(tmp, "up      : x[%f] y[%f] z[%f]", up.x, up.y, up.z);
	add_text_to_render(gomo, "font_text2", tmp, (vec3_t){5, HEIGHT - 130, 0.0f}, 0.3f, (vec3_t){0.9f, 0.9f, 0.9f}, 5);

	sprintf(tmp, "origin    : x[%f] y[%f] z[%f]", ray.origin.x, ray.origin.y, ray.origin.z);
	add_text_to_render(gomo, "font_text2", tmp, (vec3_t){5, HEIGHT - 170, 0.0f}, 0.3f, (vec3_t){0.9f, 0.9f, 0.9f}, 6);
	sprintf(tmp, "direction : x[%f] y[%f] z[%f]", ray.direction.x, ray.direction.y, ray.direction.z);
	add_text_to_render(gomo, "font_text2", tmp, (vec3_t){5, HEIGHT - 190, 0.0f}, 0.3f, (vec3_t){0.9f, 0.9f, 0.9f}, 7);

	return ray;
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
	(void)mods;
	if (button == GLFW_MOUSE_BUTTON_RIGHT && (action == GLFW_PRESS || action == GLFW_RELEASE))
	{
		double xpos, ypos;
		gomo_t *gomo = glfwGetWindowUserPointer(window);
		if (action == GLFW_PRESS && gomo->nb_stones < 361 + gomo->tmp_stone)
		{
			ray_t ray;
			hit_t res;
			glfwGetCursorPos(gomo->window, &xpos, &ypos);
			ray = createRay(gomo, xpos, ypos);
			gomo->obj = gomo->obj->first;
			if (intersect(ray, &res))
			{
				if (gomo->tmp_stone)
				{
					gomo->nb_stones--;
					gomo->tmp_stone = 0;
				}
				gomo->nb_stones++;
				gomo->obj = gomo->obj->next;
				int closest_case = 0;
				float shortest_dist = INFINITY;
				float tmp_dist = 0;
				for (int i = 0; i < 19 * 19; i++)
				{
					tmp_dist = dist_btw_two_vec3(gomo->board[i].pos, res.point);

					if (!gomo->board[i].state && tmp_dist < shortest_dist)
					{
						shortest_dist = dist_btw_two_vec3(gomo->board[i].pos, res.point);
						closest_case = i;
					}
				}
				gomo->board[closest_case].state = 1;
				gomo->stone[gomo->nb_stones - 1].pos = gomo->board[closest_case].pos;
				gomo->stone[gomo->nb_stones - 1].color = gomo->nb_stones % 2 ? (vec3_t){0.0f, 0.0f, 0.0f} : (vec3_t){1.0f, 1.0f, 1.0f};
				updateData(gomo);
			}
			else
				printf("No hit\n");
		}
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && (action == GLFW_PRESS || action == GLFW_RELEASE))
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
			if (gomo->camera->dist - zoom >= 0 && gomo->camera->dist - zoom <= 100)
				gomo->camera->dist -= zoom;
		}
	}
}

void mouse_move_callback(GLFWwindow *window, double xpos, double ypos)
{
	gomo_t *gomo = glfwGetWindowUserPointer(window);
	if (gomo->nb_stones < 361 + gomo->tmp_stone)
	{
		ray_t ray;
		hit_t res;
		ray = createRay(gomo, xpos, ypos);
		gomo->obj = gomo->obj->first;
		if (intersect(ray, &res))
		{
			if (!gomo->tmp_stone)
			{
				gomo->nb_stones++;
				gomo->tmp_stone = 1;
			}
			gomo->obj = gomo->obj->next;
			gomo->stone[gomo->nb_stones - 1].pos = res.point;
			gomo->stone[gomo->nb_stones - 1].color = (vec3_t){1.0f, 0.0f, 0.0f};
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
		else if (key == GLFW_KEY_F1)
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
