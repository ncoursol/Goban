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

	glfwSetFramebufferSizeCallback(gomo->window, framebuffer_size_callback);

	glViewport(0, 0, WIDTH, HEIGHT);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_BLEND);
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
	printf("id: %d\n", gomo->obj->id);
	for (int i = 0; i < gomo->obj->nb_vertices; i++) {
		printf("obj[%d]\t%f / %f / %f\t   %f / %f\t   %f / %f / %f\n", i,
			   gomo->obj->obj[i * 8], gomo->obj->obj[i * 8 + 1], gomo->obj->obj[i * 8 + 2],
			   gomo->obj->obj[i * 8 + 3], gomo->obj->obj[i * 8 + 4],
			   gomo->obj->obj[i * 8 + 5], gomo->obj->obj[i * 8 + 6], gomo->obj->obj[i * 8 + 7]);
	}
	*/
	glGenBuffers(1, &gomo->obj->VBO);
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 46, getErrorString(errCode));
	glBindBuffer(GL_ARRAY_BUFFER, gomo->obj->VBO);
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 47, getErrorString(errCode));
	glBufferData(GL_ARRAY_BUFFER,
				 gomo->obj->nb_vertices * 8 * sizeof(float),
				 &gomo->obj->obj[0],
				 GL_STATIC_DRAW);
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 48, getErrorString(errCode));

	// Vertex format(Coordinate/TextureUV/Color): x1,y1,z1, u1,v1, r1,g1,b1, x2...

	// Coordinate
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 49, getErrorString(errCode));
	glEnableVertexAttribArray(0);
	// TextureUV
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 50, getErrorString(errCode));
	glEnableVertexAttribArray(1);
	// Color
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(5 * sizeof(float)));
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

void init_VAOs(gomo_t *gomo)
{
	gomo->obj = gomo->obj->first;
	init_VAO(gomo);
	gomo->obj = gomo->obj->next;
	init_VAO(gomo);
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
			gomo->board[index].color = index % 2 ? (vec3_t){0.0f, 0.0f, 0.0f} : (vec3_t){1.0f, 1.0f, 1.0f};
			gomo->board[index].pos.x = (float)x + 1.0f + (0.07 * (x + 1));
			gomo->board[index].pos.y = 0.0f;
			gomo->board[index].pos.z = (float)z + 1.0f + (0.07 * (z + 1));
			index++;
		}
	}
}

void init_gomo(gomo_t *gomo)
{
	gomo->obj = NULL;
	gomo->shader = NULL;
	gomo->camera = NULL;
	gomo->tmp_stone = -1;
	gomo->tmp_hit = (hit_t){0, 0, (vec3_t){0.0f, 0.0f, 0.0f}, (vec3_t){0.0f, 0.0f, 0.0f}};
	if (!(gomo->obj = (obj_t *)malloc(sizeof(obj_t))))
		exit_callback(gomo, 0, "object malloc failed");
	gomo->obj->id = 0;
	if (!(gomo->stone = (instance_t *)malloc(sizeof(instance_t) * 19 * 19)))
		exit_callback(gomo, 0, "stone malloc failed");
	gomo->nb_stones = 0;
	if (!(gomo->shader = (shader_t *)malloc(sizeof(shader_t))))
		exit_callback(gomo, 4, "vertices malloc failed");
	if (!(gomo->camera = (camera_t *)malloc(sizeof(camera_t))))
		exit_callback(gomo, 5, "camera malloc failed");
	if (!(gomo->board = (grid_t *)malloc(sizeof(grid_t) * 19 * 19)))
		exit_callback(gomo, 6, "board malloc failed");
	init_board(gomo);
}

void	new_obj(gomo_t *gomo) {
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

void init_all(gomo_t *gomo)
{
	init_gomo(gomo);
	init_obj(gomo);
	gomo->obj->first = gomo->obj;
	load_obj(gomo, "resources/goban.obj");
	new_obj(gomo);
	load_obj(gomo, "resources/stone.obj");
	gomo->obj = gomo->obj->first;
	init_glfw(gomo);
	init_gl(gomo);
	init_shader(gomo, gomo->shader, "./src/shader.vert", "./src/shader.frag");
	init_VAOs(gomo);
	init_camera(gomo);
	load_texture(gomo);
}
