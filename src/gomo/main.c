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

	d = gomo->camera->dist;
	ah = gomo->camera->ah;
	av = gomo->camera->av;

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

	if (LEFT_MOUSE && !(TOP_VIEW) && !(ANIMATE))
		set_new_camera_angles(gomo);

	set_new_eye(gomo, up);

	new_center = (vec3_t){
		gomo->camera->center.x + gomo->camera->gap.x,
		gomo->camera->center.y + gomo->camera->gap.y,
		gomo->camera->center.z + gomo->camera->gap.z};
	camera(gomo, new_center, up);

}

void animateCamera(gomo_t *gomo)
{
	if (TOP_VIEW) {
		float target_av = PI - 0.000005f;
		float target_ah = 0.0f;
		float target_dist = 1.0f;
		float threshold = 0.001f;
		float lerp_speed = 0.1f;
		
		// Check if we need to animate
		if (fabsf(gomo->camera->av - target_av) > threshold || 
			fabsf(gomo->camera->ah - target_ah) > threshold || 
			fabsf(gomo->camera->dist - target_dist) > threshold) {
			
			// Smooth interpolation with easing
			gomo->camera->av += (target_av - gomo->camera->av) * lerp_speed;
			gomo->camera->ah += (target_ah - gomo->camera->ah) * lerp_speed;
			gomo->camera->dist += (target_dist - gomo->camera->dist) * lerp_speed;
			
			// Snap to target values when very close
			if (fabsf(target_av - gomo->camera->av) < threshold)
				gomo->camera->av = target_av;
			if (fabsf(target_ah - gomo->camera->ah) < threshold)
				gomo->camera->ah = target_ah;
			if (fabsf(target_dist - gomo->camera->dist) < threshold)
				gomo->camera->dist = target_dist;
		}
	} else if (ANIMATE) {
		float target_av = PI * 0.75f;
		float target_ah = 0.0f;
		float target_dist = 2.0f;
		float threshold = 0.001f;
		float lerp_speed = 0.1f;
		
		// Check if we need to animate
		if (fabsf(gomo->camera->av - target_av) > threshold || 
			fabsf(gomo->camera->ah - target_ah) > threshold || 
			fabsf(gomo->camera->dist - target_dist) > threshold) {
			
			// Smooth interpolation with easing
			gomo->camera->av += (target_av - gomo->camera->av) * lerp_speed;
			gomo->camera->ah += (target_ah - gomo->camera->ah) * lerp_speed;
			gomo->camera->dist += (target_dist - gomo->camera->dist) * lerp_speed;
			
			// Snap to target values when very close
			if (fabsf(target_av - gomo->camera->av) < threshold)
				gomo->camera->av = target_av;
			if (fabsf(target_ah - gomo->camera->ah) < threshold)
				gomo->camera->ah = target_ah;
			if (fabsf(target_dist - gomo->camera->dist) < threshold)
				gomo->camera->dist = target_dist;
		} else {
			gomo->camera->options ^= 1 << 9;
		}
	}
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
		animateCamera(&gomo);
		updateCamera(&gomo);

		// Draw HUD

		t = glfwGetTime();
		nb_frames++;
		if (t - last_t >= 1.0) {
		    double fps = (double)nb_frames / (t - last_t);
		    double ms_per_frame = 1000.0 / fps;
		
		    // Use snprintf for better control over precision
		    char buff[100];
		    char buff2[100];
		    snprintf(buff, sizeof(buff), "%.1f fps", fps);
		    snprintf(buff2, sizeof(buff2), "%.3f ms/f", ms_per_frame);
		
		    nb_frames = 0;
		    last_t = t;  // Reset to current time, not last_t + 1.0
		
		    add_text_to_render(&gomo, "font_text2", buff, (vec3_t){5, HEIGHT - 15, 0.0f}, 0.3f, (vec3_t){0.9f, 0.9f, 0.9f}, 0, 0);
		    add_text_to_render(&gomo, "font_text2", buff2, (vec3_t){80, HEIGHT - 15, 0.0f}, 0.3f, (vec3_t){0.9f, 0.9f, 0.9f}, 0, 1);
		}

		// Draw each object
		gomo.obj = gomo.obj->first;
		while (gomo.obj != NULL)
		{
			glBindVertexArray(gomo.obj->VAO);
			if (gomo.obj->id != 0)
			{
				glUseProgram(gomo.shader->shaderProgramStones);
				glUniformMatrix4fv(gomo.shaderID.mvpID, 1, GL_FALSE, &gomo.camera->mvp[0]);
				glUniform1f(gomo.shaderID.timeID, (float)t);
				glBindTexture(GL_TEXTURE_2D, 0);
				glDrawArraysInstanced(GL_TRIANGLES, 0, gomo.obj->nb_vertices, gomo.nb_stones);
			}
			else
			{
				glUseProgram(gomo.shader->shaderProgram);
				glUniformMatrix4fv(gomo.shaderID.mvpID, 1, GL_FALSE, &gomo.camera->mvp[0]);
				glDrawArrays(GL_TRIANGLES, 0, gomo.obj->nb_vertices);
			}
			if (gomo.obj->next == NULL)
				break;
			gomo.obj = gomo.obj->next;
		}
		glBindVertexArray(0);

		if (HUD) {
			render_lines(&gomo);
			render_all_text(&gomo);
		}

		glfwSwapBuffers(gomo.window);
		glfwPollEvents();
	}
	free_all(&gomo, 100);
	return 0;
}
