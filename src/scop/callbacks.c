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

#include "../include/scop.h"

char	*getErrorString(int code)
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

void	exit_callback(scop_t *scop, int state, char *description)
{
	fprintf(stderr, "Error: %s [%d]\n", description, state);
	free_all(scop, state);
	exit(1);
}

void 	mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	(void)mods;
	if (button == GLFW_MOUSE_BUTTON_RIGHT && (action == GLFW_PRESS || action == GLFW_RELEASE))
	{
		double xpos, ypos;
		scop_t *scop = glfwGetWindowUserPointer(window);
		if (action == GLFW_PRESS && scop->nb_stones < 362) {
			glfwGetCursorPos(scop->window, &xpos, &ypos);
			while (scop->obj->next != NULL)
				scop->obj = scop->obj->next;
			if (scop->obj->id == 0) {
				load_obj(scop, "resources/stone.obj");
				VAOs(scop, scop->obj);
			}
			scop->nb_stones++;
			int i = scop->nb_stones * 3;
			//float offset = 2.15f;
			float xt, yt; 
			xt = (2.0 * xpos) / WIDTH - 1.0;
			yt = 1.0 - (2.0 * ypos) / HEIGHT;
			float aspect_ratio = WIDTH / HEIGHT;

			float camera_fov_vertical = 2.0 * atan(tan(0.5 * scop->camera->fov) / aspect_ratio);

			float view_x = xt / aspect_ratio * tan(0.5 * scop->camera->fov);
			float view_y = yt / tan(0.5 * camera_fov_vertical);

			float world_x = view_x * cos(scop->camera->ah) - view_y * sin(scop->camera->ah);
			float world_y = view_x * sin(scop->camera->ah) + view_y * cos(scop->camera->ah);

			scop->stone_coord[i + 0] = world_x * scop->camera->dist / cos(scop->camera->av);
			scop->stone_coord[i + 1] = 0.f;
			scop->stone_coord[i + 2] = world_y * scop->camera->dist / cos(scop->camera->av);
			printf("x[%f] y[%f]\n", scop->stone_coord[i + 0], scop->stone_coord[i + 2]);
			glBindBuffer(GL_ARRAY_BUFFER, scop->stone_VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * 361, &scop->stone_coord[0], GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && (action == GLFW_PRESS || action == GLFW_RELEASE))
	{
		scop_t *scop = glfwGetWindowUserPointer(window);
		scop->camera->options ^= 1 << 5;
		if (action == GLFW_PRESS) {
			glfwSetInputMode(scop->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			glfwSetCursorPos(scop->window, scop->camera->ah / MSPEED, scop->camera->av / MSPEED);
		}
		else if (action == GLFW_RELEASE) {
			glfwSetInputMode(scop->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}
}

void	scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	(void)xoffset;
	if (yoffset)
	{
		scop_t *scop = glfwGetWindowUserPointer(window);
		if (!(TOP_VIEW)) {
			float zoom = yoffset * (log(scop->camera->dist + 7.5) - 2);
			if (scop->camera->dist - zoom >= 0 && scop->camera->dist - zoom <= 100)
				scop->camera->dist -= zoom;
		}
	}
}

void	key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	(void)mods;
	(void)scancode;
	if (action == GLFW_PRESS) {
		scop_t *scop = glfwGetWindowUserPointer(window);
		if (key == GLFW_KEY_ESCAPE)
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		else if (key == GLFW_KEY_W)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else if (key == GLFW_KEY_F)
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		else if (key == GLFW_KEY_P)
			glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		else if (key == GLFW_KEY_SPACE) {
			free_null((void*)scop->camera->mvp);
			init_camera(scop);
		}
		else if (key == GLFW_KEY_V) {
			scop->camera->options ^= 1 << 8; // top view
		}
	}
}

void	framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	(void)window;
	glViewport(0, 0, width, height);
}
