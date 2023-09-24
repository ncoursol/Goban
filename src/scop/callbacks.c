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
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && (action == GLFW_PRESS || action == GLFW_RELEASE))
		{
			scop_t *scop = glfwGetWindowUserPointer(window);
			scop->camera->options ^= 1 << 6;
			if (action == GLFW_PRESS) {
				glfwSetInputMode(scop->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				glfwSetCursorPos(scop->window, scop->camera->gh / MSPEED / (scop->camera->dist / 10), scop->camera->gv / MSPEED / (scop->camera->dist / 10));
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
		float zoom = yoffset * (log(scop->camera->dist + 7.5) - 2);
		if (scop->camera->dist - zoom >= 0 && scop->camera->dist - zoom <= 100)
			scop->camera->dist -= zoom;
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
		else if (key == GLFW_KEY_A)
			scop->camera->options ^= 1 << 0; // axis
		else if (key == GLFW_KEY_R)
			scop->camera->options ^= 1 << 1; // rotation
		else if (key == GLFW_KEY_M)
			scop->camera->options ^= 1 << 2; // movement
		else if (key == GLFW_KEY_T) {
			scop->camera->options ^= 1 << 3; // texture
			scop->camera->options ^= 1 << 7; // texture transition
			glUniform1i(scop->shaderID.textStateID, TEXTURE);
		} else if (key == GLFW_KEY_C) {
			scop->camera->options ^= 1 << 4; // color
			glUniform1i(scop->shaderID.colorStateID, COLOR);
		} else if (key == GLFW_KEY_V) {
			scop->camera->options ^= 1 << 8; // top view
		}
	}
}

void	framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    (void)window;
	glViewport(0, 0, width, height);
}
