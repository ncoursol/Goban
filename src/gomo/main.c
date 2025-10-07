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

int render_loop(gomo_t *gomo)
{
	int nb_frames = 0;
	double t, last_t, current_t, fps_timer;
	float delta_time;

	last_t = glfwGetTime();
	fps_timer = last_t;

	// RENDER LOOP
	while (!glfwWindowShouldClose(gomo->window))
	{
		current_t = glfwGetTime();
		delta_time = (float)(current_t - last_t);
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		handleCameraAnimations(gomo, delta_time);
		updateCamera(gomo, delta_time);

		// Update HUD text periodically (not every frame)
		t = current_t;
		nb_frames++;
		if (t - fps_timer >= 0.5 && HUD) {
		    double fps = (double)nb_frames / (t - fps_timer);
		    double ms_per_frame = 1000.0 / fps;
		    // Use snprintf for better control over precision
		    char buff[100];

		    nb_frames = 0;
		    fps_timer = t;  // Reset fps timer to current time

		    // Only update HUD text if HUD is enabled to avoid unnecessary work
		    snprintf(buff, sizeof(buff), "%.1f fps", fps);
		    add_text_to_render(gomo, "font_text2", buff, (vec3_t){5, HEIGHT - 15, 0.0f}, (vec3_t){0.0f, 0.0f, 0.0f}, 0, 0.3f, (vec3_t){0.9f, 0.9f, 0.9f}, 0, 0);
			snprintf(buff, sizeof(buff), "%.3f ms/f", ms_per_frame);
		    add_text_to_render(gomo, "font_text2", buff, (vec3_t){110, HEIGHT - 15, 0.0f}, (vec3_t){0.0f, 0.0f, 0.0f}, 0, 0.3f, (vec3_t){0.9f, 0.9f, 0.9f}, 0, 1);
		    snprintf(buff, sizeof(buff), "V-Sync : %s", V_SYNC ? "ON" : "OFF");
		    add_text_to_render(gomo, "font_text2", buff, (vec3_t){5, HEIGHT - 35, 0.0f}, (vec3_t){0.0f, 0.0f, 0.0f}, 0, 0.3f, (vec3_t){0.9f, 0.9f, 0.9f}, 0, 2);
		    snprintf(buff, sizeof(buff), "target Location {%.2f, %.2f, %.2f}", gomo->camera->targetPos.x, gomo->camera->targetPos.y, gomo->camera->targetPos.z);
		    add_text_to_render(gomo, "font_text2", buff, (vec3_t){5, HEIGHT - 55, 0.0f}, (vec3_t){0.0f, 0.0f, 0.0f}, 0, 0.3f, (vec3_t){0.9f, 0.9f, 0.9f}, 0, 8);
		    
		    snprintf(buff, sizeof(buff), "Animation : %s", ANIMATE ? "ON" : "OFF");
		    add_text_to_render(gomo, "font_text2", buff, (vec3_t){5, HEIGHT - 75, 0.0f}, (vec3_t){0.0f, 0.0f, 0.0f}, 0, 0.3f, (vec3_t){0.9f, 0.9f, 0.9f}, 0, 9);
		    snprintf(buff, sizeof(buff), "Center {%.2f, %.2f, %.2f}", gomo->camera->center.x, gomo->camera->center.y, gomo->camera->center.z);
		    add_text_to_render(gomo, "font_text2", buff, (vec3_t){5, HEIGHT - 95, 0.0f}, (vec3_t){0.0f, 0.0f, 0.0f}, 0, 0.3f, (vec3_t){0.9f, 0.9f, 0.9f}, 0, 10);
		}

		// Draw each object
		gomo->obj = gomo->obj->first;
		while (gomo->obj != NULL)
		{
			glBindVertexArray(gomo->obj->VAO);
			if (gomo->obj->id != 0)
			{
				glUseProgram(gomo->shader->shaderProgramStones);
				glUniformMatrix4fv(gomo->shaderID.mvpID, 1, GL_FALSE, &gomo->camera->mvp[0]);
				glUniform1f(gomo->shaderID.timeID, (float)t);
				glBindTexture(GL_TEXTURE_2D, 0);
				glDrawArraysInstanced(GL_TRIANGLES, 0, gomo->obj->nb_vertices, gomo->nb_stones);
			}
			else
			{
				glUseProgram(gomo->shader->shaderProgram);
				glUniformMatrix4fv(gomo->shaderID.mvpID, 1, GL_FALSE, &gomo->camera->mvp[0]);
				glDrawArrays(GL_TRIANGLES, 0, gomo->obj->nb_vertices);
			}
			if (gomo->obj->next == NULL)
				break;
			gomo->obj = gomo->obj->next;
		}
		glBindVertexArray(0);

		render_all_lines(gomo);
		render_all_text(gomo);

		glfwSwapBuffers(gomo->window);
		glfwPollEvents();
		
		last_t = current_t;
	}
	return 1;
}

int main(int argc, char **argv)
{
	gomo_t gomo;

	memset(&gomo, 0, sizeof(gomo_t));
	if (!(gomo.game = (game_t *)malloc(sizeof(game_t))))
		exit_callback(&gomo, 0, "object malloc failed");

	int verbose = 0;
	int ret = 0;

	if (argc < 1 || argc > 2)
		return 0;

	if (argc == 2 && strcmp(argv[1], "-v") == 0)
		verbose = 1;

	ret = init_game(gomo.game);
	if (!ret)
		return ret;
	if (verbose) {
		init_all(&gomo);
		int res = render_loop(&gomo);
		if (res)
			free_all(&gomo, 100);
	}
	return 0;
}