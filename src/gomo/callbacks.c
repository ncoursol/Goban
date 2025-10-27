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

void update_stones(gomo_t *gomo)
{
	int i = 0;
	int index = 0;

	for (i = 0; i < 19 * 19; i++)
	{
		if (gomo->board[i].state == 2 && !gomo->tmp_stone)
			gomo->board[i].state = 0;
		else if (gomo->board[i].state) {
			gomo->stone[index].pos = gomo->board[i].pos;
			gomo->stone[index].color = gomo->board[i].color;
			index++;
		}
	}
	gomo->nb_stones = index;
}

void updateData(gomo_t *gomo)
{
	GLenum errCode;

	update_stones(gomo);
	glUseProgram(gomo->shader->shaderProgramStones);
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


void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
	(void)mods;
	gomo_t *gomo = glfwGetWindowUserPointer(window);
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		if (can_place_stone(gomo))
		{
			int ret = place_stone(gomo->game, (gomo->tmp_stone - 1) / 19, (gomo->tmp_stone - 1) % 19);
			if (ret)
				printf("game over %d\n", ret - 1);
			gomo->tmp_stone = 0;
			sync_game_state(gomo, gomo->game);
			render_helpers(gomo);
			display_swap2_info(gomo);
			change_camera_target(gomo);
		}
	} 
	else if (button == GLFW_MOUSE_BUTTON_LEFT && (action == GLFW_PRESS || action == GLFW_RELEASE))
	{
		if (TEXT_HOVER && action == GLFW_PRESS)
		{
			if (MENU && BACK_BTN) {
				if (!(ANIMATE))
					C_ANIMATE;
				gomo->camera->targetPos = (vec3_t){RAD(120.0f), RAD(10.0f), 3.0f};
				gomo->camera->targetCenter = (vec3_t){0.0f, 0.5f, 0.0f};
				clear_text_to_render(gomo, 17);
				gomo->textHover = 4; // tutorial
			} else if (SUMMARY_BTNS && (int)gomo->cursor != gomo->textHover) {
				gomo->cursor = gomo->textHover;
				change_tutorial(gomo);
			} else if (SWAP2_BTNS) {
				clear_text_to_render(gomo, 5);
				clear_text_to_render(gomo, 6);
				clear_text_to_render(gomo, 7);
				clear_text_to_render(gomo, 8);
				if (BLACK_BTN || WHITE_BTN) {
					pick_color(gomo->game, BLACK_BTN ? 0 : 1);
					change_camera_target(gomo);
				} else {
					gomo->game->swap2_step++;
				}
			} else if (MENU) {
				C_ROTATION;
				if (!(ANIMATE))
					C_ANIMATE;
				if (TUTO_BTN) {
					gomo->camera->targetPos = TUTO_POS;
					gomo->camera->targetCenter = TUTO_CENTER;
					display_tutorial(gomo);
				} else if (GAMEMODE_BTNS) {
					if (!init_game(gomo->game, gomo->textHover - 1))
						return;
					C_MENU;
					gomo->camera->targetPos = P1_POS;
					for (int i = 0; i < 5; i++)
       					clear_text_to_render(gomo, i);
				}
			}
		}
		else if (!(MENU) && !(TOP_VIEW) && (gomo->textHover < 1 || gomo->textHover > 17))
		{
			if (action == GLFW_PRESS && !(LEFT_MOUSE))
			{
				C_LEFT_MOUSE;
				glfwSetInputMode(gomo->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				glfwSetCursorPos(gomo->window, gomo->camera->ah / MSPEED, gomo->camera->av / MSPEED);
			}
			else if (action == GLFW_RELEASE && LEFT_MOUSE)
			{
				C_LEFT_MOUSE;
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
		if (!(TOP_VIEW) && !(ANIMATE) && !(MENU))
		{
			float zoom = yoffset * (log(gomo->camera->dist + 7.5) - 2);
			if (gomo->camera->dist - zoom >= 1.0f && gomo->camera->dist - zoom <= 3.4f)
				gomo->camera->dist -= zoom;
		}
	}
}

void mouse_move_callback(GLFWwindow *window, double xpos, double ypos)
{
	gomo_t *gomo = glfwGetWindowUserPointer(window);
	ray_t ray;
	hit_t res;
	ray = createRay(gomo, xpos, ypos);
	gomo->obj = gomo->obj->first;
	if (!(MENU) && gomo->nb_stones < 361 && intersectBoard(ray, &res) && gomo->game->swap2_step != 1 && gomo->game->swap2_step != 3)
	{
		gomo->obj = gomo->obj->next;
		int closest_case = find_closest_case(gomo, res.point);
		if (closest_case >= 0 && gomo->board[closest_case].state == 0)
		{
			if (gomo->tmp_stone && gomo->tmp_stone - 1 != closest_case) {
				gomo->board[gomo->tmp_stone - 1].state = 0;
				gomo->board[gomo->tmp_stone - 1].color = (vec3_t){1.0f, 0.0f, 1.0f};
			}
			gomo->tmp_stone = closest_case + 1;
			gomo->board[closest_case].state = 2;
			gomo->board[closest_case].color = gomo->nb_stones % 2 ? (vec3_t){0.5f, 0.5f, 0.5f} : (vec3_t){0.8f, 0.8f, 0.8f};
		} else if (closest_case < 0 && gomo->tmp_stone) {
			gomo->board[gomo->tmp_stone - 1].state = 0;
			gomo->board[gomo->tmp_stone - 1].color = (vec3_t){1.0f, 0.0f, 1.0f};
			gomo->tmp_stone = 0;
		}
		updateData(gomo);
	} else if (intersectText(gomo, ray, &res)) {
		if (res.hit && gomo->text[res.hit].clickable) {
			if (gomo->text[res.hit].text != NULL && (gomo->text[res.hit].color.x != 1.0f || gomo->text[res.hit].color.y != 1.0f || gomo->text[res.hit].color.z != 1.0f)) {
				if (TEXT_HOVER && gomo->textHover != res.hit && gomo->text[gomo->textHover].text != NULL) {
					add_text_to_render(gomo, gomo->text[gomo->textHover].font, gomo->text[gomo->textHover].text, gomo->text[gomo->textHover].pos, gomo->text[gomo->textHover].rotation, gomo->text[gomo->textHover].face_camera, gomo->text[gomo->textHover].clickable, gomo->text[gomo->textHover].scale, (vec3_t){1.0f, 0.5f, 0.5f}, 1, gomo->textHover);
				}
				gomo->textHover = res.hit;
				if (TEXT_HOVER && gomo->text[gomo->textHover].text != NULL)
					add_text_to_render(gomo, gomo->text[res.hit].font, gomo->text[res.hit].text, gomo->text[res.hit].pos, gomo->text[res.hit].rotation, gomo->text[res.hit].face_camera, gomo->text[res.hit].clickable, gomo->text[res.hit].scale, (vec3_t){1.0f, 1.0f, 1.0f}, 1, res.hit);
			}
		}
		gomo->textHover = res.hit;
	}
	if (gomo->text[gomo->textHover].text != NULL && res.hit == -1 && TEXT_HOVER) {
		add_text_to_render(gomo, gomo->text[gomo->textHover].font, gomo->text[gomo->textHover].text, gomo->text[gomo->textHover].pos, gomo->text[gomo->textHover].rotation, gomo->text[gomo->textHover].face_camera, gomo->text[gomo->textHover].clickable, gomo->text[gomo->textHover].scale, (vec3_t){1.0f, 0.5f, 0.5f}, 1, gomo->textHover);
		gomo->textHover = -1;
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
		else if (key == GLFW_KEY_H) {
			C_HUD;
			if (!HUD) {
		        clear_text_to_render(gomo, 0);
		        clear_text_to_render(gomo, 1);
		        clear_text_to_render(gomo, 2);
		        clear_text_to_render(gomo, 8);
		        clear_text_to_render(gomo, 9);
		        clear_text_to_render(gomo, 10);
			}
		}
		else if (key == GLFW_KEY_W)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else if (key == GLFW_KEY_F)
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		else if (key == GLFW_KEY_P)
			glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		else if (!(MENU) && key == GLFW_KEY_V)
		{
			vec3_t tmp_target = TOP_POS;
			if (!(ANIMATE))
				C_ANIMATE;
			if (gomo->camera->targetPos.x != tmp_target.x || gomo->camera->targetPos.y != tmp_target.y || gomo->camera->targetPos.z != tmp_target.z) {
				gomo->camera->targetPos = tmp_target;
			} else {
				gomo->camera->targetPos = P1_POS;
			}
		}
		else if (key == GLFW_KEY_Y)
		{
			C_V_SYNC;
			glfwSwapInterval(V_SYNC ? 1 : 0);
		}
	}
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
	(void)window;
	glViewport(0, 0, width, height);
}
