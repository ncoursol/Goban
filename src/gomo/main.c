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
		d = 15;
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

vec3_t set_new_center(gomo_t *gomo, vec3_t up)
{
	vec3_t ret = (vec3_t){0, 0, 0};
	double xpos, ypos;
	float av, ah;
	float gv, gh;

	av = gomo->camera->av - (PI / 2);
	ah = gomo->camera->ah - (PI / 2);
	glfwGetCursorPos(gomo->window, &xpos, &ypos);
	gh = xpos * MSPEED * (gomo->camera->dist / 10);
	gv = ypos * MSPEED * (gomo->camera->dist / 10);
	if (up.z)
	{
		ret = (vec3_t){
			gh * cosf(ah) + gv * -sinf(av) * sinf(ah),
			gh * sinf(ah) + gv * sinf(av) * cosf(ah),
			gv * cosf(av)};
	}
	else
	{
		ret = (vec3_t){
			gh * -cosf(ah) + gv * -sinf(av) * sinf(ah),
			gv * cosf(av),
			gh * -sinf(ah) + gv * sinf(av) * cosf(ah)};
	}
	gomo->camera->gh = gh;
	gomo->camera->gv = gv;
	return (ret);
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
	else if (tmp < 0.05f)
	{
		gomo->camera->av = 0.05f;
		glfwSetCursorPos(gomo->window, gomo->camera->ah / MSPEED, gomo->camera->av / MSPEED);
	}
	else
		gomo->camera->av = tmp;
}

void updateCamera(gomo_t *gomo)
{
	vec3_t new_center;
	vec3_t up;

	up = (vec3_t){0, AXIS, !(AXIS)};

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

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gomo.grid_text);
	glUniform1i(gomo.shaderID.textureID1, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gomo.wood_text);
	glUniform1i(gomo.shaderID.textureID2, 1);

	// RENDER LOOP
	while (!glfwWindowShouldClose(gomo.window))
	{
		// Frame rate (ms/f)
		t = glfwGetTime();
		nb_frames++;
		if (t - last_t >= 1.0)
		{
			printf("%f ms/f\n", 1000.0 / (double)(nb_frames));
			nb_frames = 0;
			last_t += 1.0;
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Apply inputs events to the camera
		updateCamera(&gomo);

		// Set shader var
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
				glBindTexture(GL_TEXTURE_2D, gomo.grid_text);
				glBindTexture(GL_TEXTURE_2D, gomo.wood_text);
				glDrawArrays(GL_TRIANGLES, 0, gomo.obj->nb_vertices);
			}
			if (gomo.obj->next == NULL)
				break;
			gomo.obj = gomo.obj->next;
		}
		glBindVertexArray(0);

		glfwSwapBuffers(gomo.window);
		glfwPollEvents();
	}
	free_all(&gomo, 100);
	return 0;
}
