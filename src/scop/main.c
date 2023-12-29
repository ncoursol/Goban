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

#include "../include/scop.h"

vec3_t set_new_eye(scop_t *scop, vec3_t up)
{
	vec3_t eye;
	float d, ah, av;

	if (!(TOP_VIEW))
	{
		d = scop->camera->dist;
		ah = scop->camera->ah;
		av = scop->camera->av;
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
		eye = (vec3_t){
			(d * -sinf(av) * cosf(ah)) + scop->camera->center.x + scop->camera->gap.x,
			(d * -sinf(av) * sinf(ah)) + scop->camera->center.y + scop->camera->gap.y,
			(d * -cosf(av)) + scop->camera->center.z + scop->camera->gap.z};
	}
	else
	{
		eye = (vec3_t){
			(d * -sinf(av) * cosf(ah)) + scop->camera->center.x + scop->camera->gap.x,
			(d * -cosf(av)) + scop->camera->center.y + scop->camera->gap.y,
			(d * -sinf(av) * sinf(ah)) + scop->camera->center.z + scop->camera->gap.z,
		};
	}
	return (eye);
}

vec3_t set_new_center(scop_t *scop, vec3_t up)
{
	vec3_t ret = (vec3_t){0, 0, 0};
	double xpos, ypos;
	float av, ah;
	float gv, gh;

	av = scop->camera->av - (PI / 2);
	ah = scop->camera->ah - (PI / 2);
	glfwGetCursorPos(scop->window, &xpos, &ypos);
	gh = xpos * MSPEED * (scop->camera->dist / 10);
	gv = ypos * MSPEED * (scop->camera->dist / 10);
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
	scop->camera->gh = gh;
	scop->camera->gv = gv;
	return (ret);
}

void set_new_camera_angles(scop_t *scop)
{
	double xpos, ypos;
	float tmp;

	glfwGetCursorPos(scop->window, &xpos, &ypos);
	scop->camera->ah = xpos * MSPEED;
	tmp = ypos * MSPEED;
	if (tmp > PI - 0.05f)
	{
		scop->camera->av = PI - 0.05f;
		glfwSetCursorPos(scop->window, scop->camera->ah / MSPEED, scop->camera->av / MSPEED);
	}
	else if (tmp < 0.05f)
	{
		scop->camera->av = 0.05f;
		glfwSetCursorPos(scop->window, scop->camera->ah / MSPEED, scop->camera->av / MSPEED);
	}
	else
		scop->camera->av = tmp;
}

void updateCamera(scop_t *scop)
{
	vec3_t eye;
	vec3_t new_center;
	vec3_t up;

	up = (vec3_t){0, AXIS, !(AXIS)};

	if (LEFT_MOUSE && !(TOP_VIEW))
		set_new_camera_angles(scop);

	eye = set_new_eye(scop, up);

	new_center = (vec3_t){
		scop->camera->center.x + scop->camera->gap.x,
		scop->camera->center.y + scop->camera->gap.y,
		scop->camera->center.z + scop->camera->gap.z};
	camera(scop, eye, new_center, up);
}

int main(void)
{
	scop_t scop;
	int nb_frames = 0;
	double t, last_t;

	init_all(&scop);
	last_t = glfwGetTime();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, scop.grid_text);
	glUniform1i(scop.shaderID.textureID1, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, scop.wood_text);
	glUniform1i(scop.shaderID.textureID2, 1);

	// RENDER LOOP
	while (!glfwWindowShouldClose(scop.window))
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
		updateCamera(&scop);

		// Set shader var
		glUniformMatrix4fv(scop.shaderID.mvpID, 1, GL_FALSE, &scop.camera->mvp[0]);

		// Draw each object
		scop.obj = scop.obj->first;
		while (scop.obj != NULL)
		{
			glBindVertexArray(scop.obj->VAO);
			if (scop.obj->id != 0)
			{
				glBindTexture(GL_TEXTURE_2D, 0);
				glDrawArraysInstanced(GL_TRIANGLES, 0, scop.obj->nb_triangles * 3, scop.nb_stones);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, scop.grid_text);
				glBindTexture(GL_TEXTURE_2D, scop.wood_text);
				glDrawArrays(GL_TRIANGLES, 0, scop.obj->nb_triangles * 3);
			}
			if (scop.obj->next == NULL)
				break;
			scop.obj = scop.obj->next;
		}
		glBindVertexArray(0);

		glfwSwapBuffers(scop.window);
		glfwPollEvents();
	}
	free_all(&scop, 100);
	return 0;
}
