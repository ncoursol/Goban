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

// Thread function that runs MCTS in the background
void *ai_compute_thread(void *arg)
{
	ai_thread_data_t *data = (ai_thread_data_t *)arg;
	int x, y;
	
	// Run MCTS computation
	run_mcts(data->game, &x, &y);
	
	// Store results
	pthread_mutex_lock(&data->mutex);
	data->x = x;
	data->y = y;
	data->has_result = 1;
	data->is_computing = 0;
	pthread_mutex_unlock(&data->mutex);
	
	return NULL;
}

void play_ai_move(gomo_t *gomo)
{
	if (!gomo->game || !gomo->game->mode || gomo->game->game_over != 0)
		return;
	
	// Only process if it's AI's turn
	if (gomo->game->players[gomo->game->swap2_player].is_human || gomo->game->swap2_step == 1 || gomo->game->swap2_step == 3)
		return;
	
	pthread_mutex_lock(&gomo->ai_thread.mutex);
	
	// If AI is not computing and doesn't have a result, start computation
	if (!gomo->ai_thread.is_computing && !gomo->ai_thread.has_result) {
		gomo->ai_thread.is_computing = 1;
		gomo->ai_thread.game = gomo->game;
		gomo->ai_thread.start_time = glfwGetTime();
		pthread_mutex_unlock(&gomo->ai_thread.mutex);
		pthread_create(&gomo->ai_thread.thread, NULL, ai_compute_thread, &gomo->ai_thread);
		return;
	}
	
	// If AI has finished computing, check if minimum delay has passed
	if (gomo->ai_thread.has_result) {
		double current_time = glfwGetTime();
		double elapsed = current_time - gomo->ai_thread.start_time;
		
		// Only apply move if minimum delay has passed
		if (elapsed >= gomo->ai_thread.min_delay) {
			int x = gomo->ai_thread.x;
			int y = gomo->ai_thread.y;
			gomo->ai_thread.has_result = 0;
			pthread_mutex_unlock(&gomo->ai_thread.mutex);
			
			// Wait for thread to finish
			pthread_join(gomo->ai_thread.thread, NULL);
			
			// Apply the move
			int ret = place_stone(gomo->game, x, y);
			if (ret) {
				gomo->game->game_over = ret;
				return;
			}
			sync_game_state(gomo, gomo->game);
			render_helpers(gomo);
			display_swap2_info(gomo);
			change_camera_target(gomo);
			return;
		}
	}
	
	pthread_mutex_unlock(&gomo->ai_thread.mutex);
}

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
		    add_text_to_render(gomo, "font_text2", buff, (vec3_t){5, HEIGHT - 15, 0.0f}, (vec3_t){0.0f, 0.0f, 0.0f}, 0, 0, 0.3f, (vec3_t){0.9f, 0.9f, 0.9f}, 0, NB_TEXT - 1);
			snprintf(buff, sizeof(buff), "%.3f ms/f", ms_per_frame);
		    add_text_to_render(gomo, "font_text2", buff, (vec3_t){110, HEIGHT - 15, 0.0f}, (vec3_t){0.0f, 0.0f, 0.0f}, 0, 0, 0.3f, (vec3_t){0.9f, 0.9f, 0.9f}, 0, NB_TEXT - 2);
		    snprintf(buff, sizeof(buff), "V-Sync : %s", V_SYNC ? "ON" : "OFF");
		    add_text_to_render(gomo, "font_text2", buff, (vec3_t){5, HEIGHT - 35, 0.0f}, (vec3_t){0.0f, 0.0f, 0.0f}, 0, 0, 0.3f, (vec3_t){0.9f, 0.9f, 0.9f}, 0, NB_TEXT - 3);
		    snprintf(buff, sizeof(buff), "target Location {%.2f, %.2f, %.2f}", gomo->camera->targetPos.x, gomo->camera->targetPos.y, gomo->camera->targetPos.z);
		    add_text_to_render(gomo, "font_text2", buff, (vec3_t){5, HEIGHT - 55, 0.0f}, (vec3_t){0.0f, 0.0f, 0.0f}, 0, 0, 0.3f, (vec3_t){0.9f, 0.9f, 0.9f}, 0, NB_TEXT - 4);
		    
		    snprintf(buff, sizeof(buff), "Animation : %s", ANIMATE ? "ON" : "OFF");
		    add_text_to_render(gomo, "font_text2", buff, (vec3_t){5, HEIGHT - 75, 0.0f}, (vec3_t){0.0f, 0.0f, 0.0f}, 0, 0, 0.3f, (vec3_t){0.9f, 0.9f, 0.9f}, 0, NB_TEXT - 5);
		    snprintf(buff, sizeof(buff), "Center {%.2f, %.2f, %.2f}", gomo->camera->center.x, gomo->camera->center.y, gomo->camera->center.z);
		    add_text_to_render(gomo, "font_text2", buff, (vec3_t){5, HEIGHT - 95, 0.0f}, (vec3_t){0.0f, 0.0f, 0.0f}, 0, 0, 0.3f, (vec3_t){0.9f, 0.9f, 0.9f}, 0, NB_TEXT - 6);

			snprintf(buff, sizeof(buff), "current color: [%d]", gomo->game->current_player);
		    add_text_to_render(gomo, "font_text2", buff, (vec3_t){5, HEIGHT - 125, 0.0f}, (vec3_t){0.0f, 0.0f, 0.0f}, 0, 0, 0.3f, (vec3_t){0.9f, 0.9f, 0.9f}, 0, NB_TEXT - 7);
			snprintf(buff, sizeof(buff), "swap2_player: [%d]", gomo->game->swap2_player);
		    add_text_to_render(gomo, "font_text2", buff, (vec3_t){5, HEIGHT - 145, 0.0f}, (vec3_t){0.0f, 0.0f, 0.0f}, 0, 0, 0.3f, (vec3_t){0.9f, 0.9f, 0.9f}, 0, NB_TEXT - 8);
			snprintf(buff, sizeof(buff), "swap2_step: [%d]", gomo->game->swap2_step);
		    add_text_to_render(gomo, "font_text2", buff, (vec3_t){5, HEIGHT - 165, 0.0f}, (vec3_t){0.0f, 0.0f, 0.0f}, 0, 0, 0.3f, (vec3_t){0.9f, 0.9f, 0.9f}, 0, NB_TEXT - 9);
		}

		play_ai_move(gomo);

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

	// Initialize AI thread data
	pthread_mutex_init(&gomo.ai_thread.mutex, NULL);
	gomo.ai_thread.is_computing = 0;
	gomo.ai_thread.has_result = 0;
	gomo.ai_thread.game = NULL;
	gomo.ai_thread.min_delay = 0.3; // Minimum 0.3 second delay

	int option = 0;

	if (argc < 1 || argc > 2)
		return 0;

	if (argc == 2 && strcmp(argv[1], "-v") == 0) // OpenGL version
		option = 1;
	else if (argc == 2 && strcmp(argv[1], "-g") == 0) // Run mcts game generation
		option = 2;
	
	if (option == 1) {
		init_all(&gomo);
		int res = render_loop(&gomo);
		if (res) {
			// Clean up AI thread if still running
			pthread_mutex_lock(&gomo.ai_thread.mutex);
			if (gomo.ai_thread.is_computing) {
				pthread_mutex_unlock(&gomo.ai_thread.mutex);
				pthread_join(gomo.ai_thread.thread, NULL);
			} else {
				pthread_mutex_unlock(&gomo.ai_thread.mutex);
			}
			pthread_mutex_destroy(&gomo.ai_thread.mutex);
			free_all(&gomo, 100);
		}
	} else if (option == 2) {
		int x, y, ret;
		ret = init_game(gomo.game, 1);
		if (!ret)
			return ret;
		gomo.game->board[1][1] = 1;
		gomo.game->board[2][2] = 1;
		gomo.game->board[3][3] = 1;
		gomo.game->board[4][4] = 1;
		print_board(gomo.game->board);
		run_mcts(gomo.game, &x, &y);
		gomo.game->board[x][y] = gomo.game->current_player == 0 ? 1 : 2;
		print_board(gomo.game->board);

		pthread_mutex_destroy(&gomo.ai_thread.mutex);
		free_all(&gomo, 0);
	}
	return 0;
}