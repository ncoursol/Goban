/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncoursol <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/04/04 16:59:08 by ncoursol          #+#    #+#             */
/*   Updated: 2023/02/27 15:05:28 by ncoursol         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/gomo.h"

void set_new_eye(gomo_t *gomo, vec3_t up)
{
	float d, ah, av;

	if (!(TOP_VIEW))
	{
		d = gomo->camera->dist;
		ah = gomo->camera->ah;
		av = gomo->camera->av;
	}
	else
	{
		av = PI - 0.000005f;
		ah = 0.0f;
		d = 42.0f;
	}
	if (av < 1.535f)
		av = 1.535f;
	if (up.z)
	{
		gomo->camera->eye = (vec3_t){
			(d * -sinf(av) * cosf(ah)) + gomo->camera->center.x + gomo->camera->gap.x,
			(d * -sinf(av) * sinf(ah)) + gomo->camera->center.y + gomo->camera->gap.y,
			(d * -cosf(av)) + gomo->camera->center.z + gomo->camera->gap.z};
	}
	else
	{
		gomo->camera->eye = (vec3_t){
			(d * -sinf(av) * cosf(ah)) + gomo->camera->center.x + gomo->camera->gap.x,
			(d * -cosf(av)) + gomo->camera->center.y + gomo->camera->gap.y,
			(d * -sinf(av) * sinf(ah)) + gomo->camera->center.z + gomo->camera->gap.z,
		};
	}
}

void set_new_camera_angles(gomo_t *gomo)
{
	double xpos, ypos;
	float tmp;

	glfwGetCursorPos(gomo->window, &xpos, &ypos);
	gomo->camera->ah = xpos * MSPEED;
	tmp = ypos * MSPEED;
	if (tmp > PI - 0.05f)
	{
		gomo->camera->av = PI - 0.05f;
		glfwSetCursorPos(gomo->window, gomo->camera->ah / MSPEED, gomo->camera->av / MSPEED);
	}
	else if (tmp < 1.0f)
	{
		gomo->camera->av = 1.0f;
		glfwSetCursorPos(gomo->window, gomo->camera->ah / MSPEED, gomo->camera->av / MSPEED);
	}
	else
		gomo->camera->av = tmp;
}

void updateCamera(gomo_t *gomo)
{
	vec3_t new_center;
	vec3_t up;

	up = (vec3_t){0, 1, 0};

	if (LEFT_MOUSE && !(TOP_VIEW))
		set_new_camera_angles(gomo);

	set_new_eye(gomo, up);

	new_center = (vec3_t){
		gomo->camera->center.x + gomo->camera->gap.x,
		gomo->camera->center.y + gomo->camera->gap.y,
		gomo->camera->center.z + gomo->camera->gap.z};
	camera(gomo, new_center, up);

}

int main(void)
{
	gomo_t gomo;
	int nb_frames = 0;
	double t, last_t;

	init_all(&gomo);
	last_t = glfwGetTime();

	// RENDER LOOP
	while (!glfwWindowShouldClose(gomo.window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		updateCamera(&gomo);

		// Draw HUD

		// Frame rate (ms/f)
		t = glfwGetTime();
		nb_frames++;

		char buff[100];
		char buff2[100];
		if (t - last_t >= 1.0f)
		{
			gcvt((double)(nb_frames), 2, buff);
			gcvt(1000.0f / (double)(nb_frames), 5, buff2);
			strcat(buff, " fps");
			strcat(buff2, " ms/f");
			nb_frames = 0;
			last_t += 1.0;
			add_text_to_render(&gomo, "font_text2", buff, (vec3_t){5, HEIGHT - 15, 0.0f}, 0.3f, (vec3_t){0.9f, 0.9f, 0.9f}, 0);
			add_text_to_render(&gomo, "font_text2", buff2, (vec3_t){70, HEIGHT - 15, 0.0f}, 0.3f, (vec3_t){0.9f, 0.9f, 0.9f}, 1);
		}

		// Render lines
		if (HUD)
			render_lines(&gomo);
		// Draw 3D objects
		glUseProgram(gomo.shader->shaderProgram);
		glUniformMatrix4fv(gomo.shaderID.mvpID, 1, GL_FALSE, &gomo.camera->mvp[0]);

		// Draw each object
		gomo.obj = gomo.obj->first;
		while (gomo.obj != NULL)
		{
			glBindVertexArray(gomo.obj->VAO);
			if (gomo.obj->id != 0)
			{
				glBindTexture(GL_TEXTURE_2D, 0);
				glDrawArraysInstanced(GL_TRIANGLES, 0, gomo.obj->nb_vertices, gomo.nb_stones);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, gomo.textures[1]);
				glDrawArrays(GL_TRIANGLES, 0, gomo.obj->nb_vertices);
			}
			if (gomo.obj->next == NULL)
				break;
			gomo.obj = gomo.obj->next;
		}

		glBindVertexArray(0);


		// Render all text
		if (HUD)
			render_all_text(&gomo);

		glfwSwapBuffers(gomo.window);
		glfwPollEvents();
	}
	free_all(&gomo, 100);
	return 0;
}
