/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncoursol <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/04/04 16:59:13 by ncoursol          #+#    #+#             */
/*   Updated: 2022/05/01 19:24:40 by ncoursol         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/gomo.h"

void init_glfw(gomo_t *gomo)
{
	if (!glfwInit())
		exit_callback(gomo, 24, "glfwInit failed");

	/* Set window spec. */
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	/* Create a windowed mode window and its OpenGL context */
	if (!(gomo->window = glfwCreateWindow(WIDTH, HEIGHT, "gomo Ncoursol", NULL, NULL)))
		exit_callback(gomo, 25, "glfwCreateWindow failed");

	/* Set callback functions */
	glfwSetWindowUserPointer(gomo->window, gomo);
	glfwSetKeyCallback(gomo->window, key_callback);
	glfwSetScrollCallback(gomo->window, scroll_callback);
	glfwSetMouseButtonCallback(gomo->window, mouse_button_callback);
	glfwSetCursorPosCallback(gomo->window, mouse_move_callback);

	/* Make the window's context current */
	glfwMakeContextCurrent(gomo->window);

	glfwSetInputMode(gomo->window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetCursorPos(gomo->window, WIDTH / 2, HEIGHT / 2);
}

void init_gl(gomo_t *gomo)
{
	/* Load the openGL librarie */
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		exit_callback(gomo, 26, "gladLoadGLLoader failed");
	
	if (FT_Init_FreeType(&gomo->ft))
		exit_callback(gomo, 27, "FT_Init_FreeType failed");

	glfwSetFramebufferSizeCallback(gomo->window, framebuffer_size_callback);

	glViewport(0, 0, WIDTH, HEIGHT);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void init_VAO(gomo_t *gomo)
{
	GLenum errCode;

	// VAO
	glGenVertexArrays(1, &gomo->obj->VAO);
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 44, getErrorString(errCode));
	glBindVertexArray(gomo->obj->VAO);
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 45, getErrorString(errCode));

	// VBO
	/*
	for (int i = 0; i < gomo->obj->nb_vertices; i++) {
		printf("obj[%d]\t%f / %f / %f     %f / %f     %f\n", i,
			   gomo->obj->obj[i * 6], gomo->obj->obj[i * 6 + 1], gomo->obj->obj[i * 6 + 2],
			   gomo->obj->obj[i * 6 + 3], gomo->obj->obj[i * 6 + 4],
			   gomo->obj->obj[i * 6 + 5]);
	}*/

	glGenBuffers(1, &gomo->obj->VBO);
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 46, getErrorString(errCode));
	glBindBuffer(GL_ARRAY_BUFFER, gomo->obj->VBO);
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 47, getErrorString(errCode));
	glBufferData(GL_ARRAY_BUFFER,
				 gomo->obj->nb_vertices * 6 * sizeof(float),
				 &gomo->obj->obj[0],
				 GL_STATIC_DRAW);
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 48, getErrorString(errCode));

	// Vertex format(Coordinate/TextureUV/Color): x1,y1,z1, u1,v1, i1, x2...

	// Coordinate
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 49, getErrorString(errCode));
	glEnableVertexAttribArray(0);
	// TextureUV
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 50, getErrorString(errCode));
	glEnableVertexAttribArray(1);
	// Id
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(5 * sizeof(float)));
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 51, getErrorString(errCode));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	if (gomo->obj->id)
	{
		// Stone instance VBO
		glGenBuffers(1, &gomo->instanceVBO);
		glBindBuffer(GL_ARRAY_BUFFER, gomo->instanceVBO);
		if ((errCode = glGetError()) != GL_NO_ERROR)
			exit_callback(gomo, 200, getErrorString(errCode));
		glBufferData(GL_ARRAY_BUFFER, sizeof(instance_t) * 19 * 19, &gomo->stone[0], GL_STATIC_DRAW);
		if ((errCode = glGetError()) != GL_NO_ERROR)
			exit_callback(gomo, 201, getErrorString(errCode));
		// Coordinate
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(instance_t), (void *)0);
		if ((errCode = glGetError()) != GL_NO_ERROR)
			exit_callback(gomo, 202, getErrorString(errCode));
		glEnableVertexAttribArray(3);
		glVertexAttribDivisor(3, 1);
		if ((errCode = glGetError()) != GL_NO_ERROR)
			exit_callback(gomo, 203, getErrorString(errCode));
		// Color
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(instance_t), (void*)offsetof(instance_t, color));
		if ((errCode = glGetError()) != GL_NO_ERROR)
			exit_callback(gomo, 202, getErrorString(errCode));
		glEnableVertexAttribArray(4);
		glVertexAttribDivisor(4, 1);
		if ((errCode = glGetError()) != GL_NO_ERROR)
			exit_callback(gomo, 203, getErrorString(errCode));

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	glBindVertexArray(0);
}

void init_lineVAO(gomo_t *gomo)
{
	GLenum errCode;
	/*
	// display lines
	for (int i = 0; i < 4; i++)
	{
		printf("line[%d]\t%f / %f / %f\t%f / %f / %f\t%f / %f / %f\n", i,
			   gomo->lines[i].start.x, gomo->lines[i].start.y, gomo->lines[i].start.z,
			   gomo->lines[i].end.x, gomo->lines[i].end.y, gomo->lines[i].end.z,
			   gomo->lines[i].color.x, gomo->lines[i].color.y, gomo->lines[i].color.z);
	}
	*/
	// VAO
	glGenVertexArrays(1, &gomo->lineVAO);
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 52, getErrorString(errCode));
	glBindVertexArray(gomo->lineVAO);
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 53, getErrorString(errCode));

	// VBO
	glGenBuffers(1, &gomo->lineVBO);
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 54, getErrorString(errCode));
	glBindBuffer(GL_ARRAY_BUFFER, gomo->lineVBO);
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 55, getErrorString(errCode));
	glBufferData(GL_ARRAY_BUFFER,
				 gomo->nb_lines * 12 * sizeof(float),
				 &gomo->lines_buffer[0],
				 GL_STATIC_DRAW);
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 56, getErrorString(errCode));

	// Vertex format(Coordinate/Color): x1,y1,z1, r1, g1, b1

	// Coordinate
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 57, getErrorString(errCode));
	glEnableVertexAttribArray(0);
	// Color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 58, getErrorString(errCode));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void init_VAOs(gomo_t *gomo)
{
	gomo->obj = gomo->obj->first;
	init_VAO(gomo);
	gomo->obj = gomo->obj->next;
	init_VAO(gomo);
	init_lineVAO(gomo);

}

void init_face_data(gomo_t *gomo, int nb_buff)
{
	for (int i = (V_BUFF_SIZE * nb_buff) - V_BUFF_SIZE; i < V_BUFF_SIZE * nb_buff; i++)
	{
		if (!(gomo->obj->faces[i] = (data_t *)malloc(sizeof(data_t))))
			exit_callback(gomo, 7, "faces data malloc failed");
		gomo->obj->faces[i]->vertex = 0;
		gomo->obj->faces[i]->texture = 0;
		gomo->obj->faces[i]->normal = 0;
		gomo->obj->faces[i]->next = NULL;
		gomo->obj->faces[i]->first = gomo->obj->faces[i];
	}
}

void init_fonts(gomo_t *gomo)
{
	if (!(gomo->fonts->characters = (charact_t *)malloc(sizeof(charact_t) * 128)))
		exit_callback(gomo, 8, "characters malloc failed");
	gomo->fonts->path = NULL;
	gomo->fonts->next = NULL;
	gomo->fonts->first = gomo->fonts;

	for (int i = 0; i < NB_TEXT; i++)
	{
		gomo->text[i].id = i;
		gomo->text[i].font = NULL;
		gomo->text[i].text = NULL;
		gomo->text[i].scale = 0.0f;
		gomo->text[i].pos = (vec3_t){0.0f, 0.0f, 0.0f};
		gomo->text[i].color = (vec3_t){0.0f, 0.0f, 0.0f};
	}
}

void init_obj(gomo_t *gomo)
{
	gomo->obj->faces = NULL;
	gomo->obj->smooth = 0;
	gomo->obj->texCoord = 0;
	gomo->obj->faces_size = 0;
	gomo->obj->nb_faces = 0;
	gomo->obj->nb_vertices = 0;
	if (!(gomo->obj->faces = (data_t **)malloc(sizeof(data_t *) * V_BUFF_SIZE)))
		exit_callback(gomo, 6, "faces malloc failed");
	if (!(gomo->obj->materials = (material_t *)malloc(sizeof(material_t) * V_BUFF_SIZE)))
		exit_callback(gomo, 7, "materials malloc failed");
	gomo->obj->faces_size = V_BUFF_SIZE;
	gomo->obj->obj = NULL;
	for (int i = 0; i < 3; i++)
	{
		gomo->obj->min[i] = 0;
		gomo->obj->max[i] = 0;
	}
	gomo->obj->next = NULL;
	init_face_data(gomo, 1);
}

void init_board(gomo_t *gomo)
{
	int index = 0;
	for (int z = -19; z < 19; z += 2)
	{
		for (int x = -19; x < 19; x += 2)
		{
			gomo->board[index].state = 0;
			gomo->board[index].color = (vec3_t){1.0f, 0.0f, 1.0f};
			gomo->board[index].pos.x = 0.02f * (float)x;
			gomo->board[index].pos.y = 0.0f;	// magic number for the gap btw goban and stones
			gomo->board[index].pos.z = 0.02f * (float)z;
			index++;
		}
	}
}

void init_gomo(gomo_t *gomo)
{
	gomo->obj = NULL;
	gomo->shader = NULL;
	gomo->camera = NULL;
	gomo->tmp_stone = 0;
	gomo->cursor = 0;
	gomo->tmp_hit = (hit_t){0, 0, (vec3_t){0.0f, 0.0f, 0.0f}, (vec3_t){0.0f, 0.0f, 0.0f}};
	if (!(gomo->obj = (obj_t *)malloc(sizeof(obj_t))))
		exit_callback(gomo, 0, "object malloc failed");
	gomo->obj->id = 0;
	if (!(gomo->stone = (instance_t *)malloc(sizeof(instance_t) * 19 * 19)))
		exit_callback(gomo, 1, "stone malloc failed");
	gomo->nb_stones = 0;
	
	if (!(gomo->shader = (shader_t *)malloc(sizeof(shader_t))))
		exit_callback(gomo, 4, "vertices malloc failed");
	if (!(gomo->camera = (camera_t *)malloc(sizeof(camera_t))))
		exit_callback(gomo, 5, "camera malloc failed");
	if (!(gomo->board = (grid_t *)malloc(sizeof(grid_t) * 19 * 19)))
		exit_callback(gomo, 6, "board malloc failed");
	if (!(gomo->fonts = (font_t *)malloc(sizeof(font_t))))
		exit_callback(gomo, 7, "fonts malloc failed");
	if (!(gomo->text = (text_t *)malloc(sizeof(text_t) * NB_TEXT)))
		exit_callback(gomo, 8, "text malloc failed");
	if (!(gomo->game_data = (game_data_t *)malloc(sizeof(game_data_t))))
		exit_callback(gomo, 9, "game data malloc failed");
	if (!(gomo->game_data->moves = (move_t *)malloc(sizeof(move_t) * MAX_MOVES)))
		exit_callback(gomo, 10, "game data move malloc failed");

}

void new_obj(gomo_t *gomo) {
	obj_t *new;
	obj_t *first;

	first = gomo->obj->first;
	if (!(new = (obj_t*)malloc(sizeof(obj_t))))
		exit_callback(gomo, 10, "new object malloc failed");
	new->id = gomo->obj->id + 1;
	gomo->obj->next = new;
	gomo->obj = gomo->obj->next;
	init_obj(gomo);
	gomo->obj->first = first;
}

void read_game(gomo_t *gomo) {
	read_sgf_game(gomo, "./resources/games/Agon/01/1.sgf");
}

void init_all(gomo_t *gomo)
{
	init_gomo(gomo);
	init_board(gomo);
	init_obj(gomo);
	init_lines(gomo);
	gomo->obj->first = gomo->obj;
	load_obj(gomo, "resources/room.obj");
	new_obj(gomo);
	load_obj(gomo, "resources/stone2.obj");
	gomo->obj = gomo->obj->first;
	init_glfw(gomo);
	init_gl(gomo);
	printf("OpenGL version: %s\n", glGetString(GL_VERSION));
	init_shader(gomo);
	init_VAOs(gomo);
	init_camera(gomo);
	load_textures(gomo);
	init_fonts(gomo);
	load_fonts(gomo);
	read_game(gomo);
}
