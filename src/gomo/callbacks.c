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

hit_t rayTriangle(ray_t ray, vec3_t triA, vec3_t triB, vec3_t triC)
{
	vec3_t edgeAB = sub_vec3(triB, triA);
	vec3_t edgeAC = sub_vec3(triC, triA);
	vec3_t normal = cross_vec3(edgeAB, edgeAC);
	vec3_t ao = sub_vec3(ray.origin, triA);
	vec3_t dao = cross_vec3(ao, ray.direction);

	float determinant = -dot_vec3(ray.direction, normal);
	float invDet = 1 / determinant;

	float dst = dot_vec3(ao, normal) * invDet;
	float u = dot_vec3(edgeAC, dao) * invDet;
	float v = -dot_vec3(edgeAB, dao) * invDet;
	float w = 1 - u - v;

	hit_t hit;
	hit.hit = determinant >= 1e-6 && dst >= 0 && u >= 0 && v >= 0 && w >= 0;
	hit.point = add_vec3(ray.origin, prod_vec3(ray.direction, dst));
	hit.t = dst;
	return (hit);
}

hit_t ray_intersect_obj(ray_t ray, obj_t *obj)
{
	hit_t closest_hit, curr;
	closest_hit.hit = 0;
	closest_hit.t = INFINITY;

	for (int i = 0; i < obj->nb_triangles / 3; i++)
	{
		vec3_t triA = {
			obj->obj[i * 24 + 0],
			obj->obj[i * 24 + 1],
			obj->obj[i * 24 + 2]};
		vec3_t triB = {
			obj->obj[i * 24 + 8],
			obj->obj[i * 24 + 9],
			obj->obj[i * 24 + 10]};
		vec3_t triC = {
			obj->obj[i * 24 + 16],
			obj->obj[i * 24 + 17],
			obj->obj[i * 24 + 18]};

		curr = rayTriangle(ray, triA, triB, triC);
		if (curr.hit && curr.t < closest_hit.t)
		{
			closest_hit.hit = 1;
			closest_hit.t = curr.t;
			closest_hit.point = curr.point;
			closest_hit.normal = curr.normal;
		}
	}

	return closest_hit;
}

ray_t createRay(vec3_t cameraPos, double xpos, double ypos)
{
	float normalizedX = (2.0f * xpos / WIDTH) - 1.0f;
	float normalizedY = 1.0f - (2.0f * ypos / HEIGHT);
	vec3_t screenPoint = {normalizedX * 2.0f - 1.0f, normalizedY * 2.0f - 1.0f, -1.0f}; // near plane z = -1

	// Direction from camera position to screen point
	vec3_t direction = {
		screenPoint.x - cameraPos.x,
		screenPoint.y - cameraPos.y,
		screenPoint.z - cameraPos.z};

	// Normalize the direction vector
	float length = sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
	direction.x /= length;
	direction.y /= length;
	direction.z /= length;

	// Create and return the ray
	ray_t ray = {cameraPos, direction};
	printf("Ray Direction: (%f, %f, %f)\n", ray.direction.x, ray.direction.y, ray.direction.z);
	return ray;
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
	(void)mods;
	if (button == GLFW_MOUSE_BUTTON_RIGHT && (action == GLFW_PRESS || action == GLFW_RELEASE))
	{
		GLenum errCode;
		double xpos, ypos;
		gomo_t *gomo = glfwGetWindowUserPointer(window);
		if (action == GLFW_PRESS && gomo->nb_stones < 361)
		{
			ray_t ray;
			hit_t res;
			glfwGetCursorPos(gomo->window, &xpos, &ypos);
			ray = createRay(gomo->camera->eye, xpos, ypos);
			gomo->obj = gomo->obj->first;
			res = ray_intersect_obj(ray, gomo->obj);
			gomo->obj = gomo->obj->next;
			if (res.hit)
			{
				printf("Hit\n");
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
				gomo->stone[gomo->nb_stones].pos = gomo->board[closest_case].pos;
				gomo->nb_stones++;
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
			else
				printf("No hit\n");
		}
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && (action == GLFW_PRESS || action == GLFW_RELEASE))
	{
		gomo_t *gomo = glfwGetWindowUserPointer(window);
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

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	(void)mods;
	(void)scancode;
	if (action == GLFW_PRESS)
	{
		gomo_t *gomo = glfwGetWindowUserPointer(window);
		if (key == GLFW_KEY_ESCAPE)
			glfwSetWindowShouldClose(window, GLFW_TRUE);
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
