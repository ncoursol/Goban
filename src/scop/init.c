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

#include "../include/scop.h"

void	init_glfw(scop_t *scop)
{
	if (!glfwInit())
		exit_callback(scop, 24, "glfwInit failed");

	/* Set window spec. */
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	/* Create a windowed mode window and its OpenGL context */
	if(!(scop->window = glfwCreateWindow(WIDTH, HEIGHT, "Scop Ncoursol", NULL, NULL)))
		exit_callback(scop, 25, "glfwCreateWindow failed");

	/* Set callback functions */
	glfwSetWindowUserPointer(scop->window, scop);
	glfwSetKeyCallback(scop->window, key_callback);
	glfwSetScrollCallback(scop->window, scroll_callback);
	glfwSetMouseButtonCallback(scop->window, mouse_button_callback);

	/* Make the window's context current */
	glfwMakeContextCurrent(scop->window);

	glfwSetInputMode(scop->window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetCursorPos(scop->window, WIDTH / 2, HEIGHT / 2);
}

void	init_gl(scop_t *scop)
{
	/* Load the openGL librarie */
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		exit_callback(scop, 26, "gladLoadGLLoader failed");

	glfwSetFramebufferSizeCallback(scop->window, framebuffer_size_callback);

	glViewport(0, 0, WIDTH, HEIGHT);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void	init_VAOs(scop_t *scop)
{
	GLenum	errCode;
	
	scop->obj = scop->obj->first;
	while (scop->obj != NULL) {
		// VAO
		glGenVertexArrays(1, &scop->obj->VAO);  
		if ((errCode = glGetError()) != GL_NO_ERROR)
			exit_callback(scop, 44, getErrorString(errCode));
		glBindVertexArray(scop->obj->VAO);
		if ((errCode = glGetError()) != GL_NO_ERROR)
			exit_callback(scop, 45, getErrorString(errCode));

		// VBO
		glGenBuffers(1, &scop->obj->VBO);
		if ((errCode = glGetError()) != GL_NO_ERROR)
			exit_callback(scop, 46, getErrorString(errCode));
		glBindBuffer(GL_ARRAY_BUFFER, scop->obj->VBO);
		if ((errCode = glGetError()) != GL_NO_ERROR)
			exit_callback(scop, 47, getErrorString(errCode));
		glBufferData(GL_ARRAY_BUFFER,
				scop->obj->nb_triangles * 8 * sizeof(float),
				&scop->obj->obj[0],
				GL_STATIC_DRAW);
		if ((errCode = glGetError()) != GL_NO_ERROR)
			exit_callback(scop, 48, getErrorString(errCode));

		// Vertex format(Vertex/Texture/Color): x1,y1,z1, u1,v1, r1,g1,b1, x2...

		// Vertex
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		if ((errCode = glGetError()) != GL_NO_ERROR)
			exit_callback(scop, 49, getErrorString(errCode));
		glEnableVertexAttribArray(0);
		// Texture
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		if ((errCode = glGetError()) != GL_NO_ERROR)
			exit_callback(scop, 50, getErrorString(errCode));
		glEnableVertexAttribArray(1);
		// Color
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
		if ((errCode = glGetError()) != GL_NO_ERROR)
			exit_callback(scop, 51, getErrorString(errCode));
		glEnableVertexAttribArray(2);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		if (scop->obj->next == NULL)
			break;
		scop->obj = scop->obj->next;
	}
	scop->obj = scop->obj->first;
}

void	init_face_data(scop_t *scop, int nb_buff)
{
	for (int i = (V_BUFF_SIZE * nb_buff) - V_BUFF_SIZE; i < V_BUFF_SIZE * nb_buff; i++) {
		if (!(scop->obj->faces[i] = (data_t*)malloc(sizeof(data_t))))
			exit_callback(scop, 7, "faces data malloc failed");
		scop->obj->faces[i]->vertex = 0;
		scop->obj->faces[i]->texture = 0;
		scop->obj->faces[i]->normal = 0;
		scop->obj->faces[i]->next = NULL;
		scop->obj->faces[i]->first = scop->obj->faces[i];
	}
}

void	init_obj(scop_t *scop)
{
	scop->obj->name = NULL;
	scop->obj->matName = NULL;
	scop->obj->matDef = NULL;
	scop->obj->faces = NULL;
	scop->obj->smooth = 0;
	scop->obj->texCoord = 0;
	scop->obj->faces_size = 0;
	scop->obj->nb_faces = 0;
	scop->obj->nb_triangles = 0;
	if (!(scop->obj->faces = (data_t**)malloc(sizeof(data_t*) * V_BUFF_SIZE)))
		exit_callback(scop, 6, "faces malloc failed");
	scop->obj->faces_size = V_BUFF_SIZE;
	scop->obj->obj = NULL;
	for (int i = 0; i < 3; i++) {
		scop->obj->min[i] = 0;
		scop->obj->max[i] = 0;
	}
	scop->obj->next = NULL;
	init_face_data(scop, 1);
}

void	init_scop(scop_t *scop)
{
	scop->obj = NULL;
	scop->vertices = NULL;
	scop->textures = NULL;
	scop->normals = NULL;
	scop->shader = NULL;
	scop->camera = NULL;
	if (!(scop->obj = (obj_t*)malloc(sizeof(obj_t))))
		exit_callback(scop, 0, "object malloc failed");
	scop->obj->id = 0;
	scop->nb_vertices = 0;
	scop->nb_textures = 0;
	scop->nb_normals = 0;
	if (!(scop->vertices = (float*)malloc(sizeof(float) * V_BUFF_SIZE)))
		exit_callback(scop, 1, "vertices malloc failed");
	if (!(scop->textures = (float*)malloc(sizeof(float) * V_BUFF_SIZE)))
		exit_callback(scop, 2, "textures malloc failed");
	if (!(scop->normals = (float*)malloc(sizeof(float) * V_BUFF_SIZE)))
		exit_callback(scop, 3, "normals malloc failed");
	if (!(scop->shader = (shader_t*)malloc(sizeof(shader_t))))
		exit_callback(scop, 4, "vertices malloc failed");
	if (!(scop->camera = (camera_t*)malloc(sizeof(camera_t))))
		exit_callback(scop, 5, "camera malloc failed");
}

void	init_all(scop_t *scop, char **argv)
{
	init_scop(scop);
	init_obj(scop);
	scop->obj->first = scop->obj;
	load_obj(scop, argv);
	init_glfw(scop);
	init_gl(scop);
	init_shader(scop, scop->shader, "./src/shader.vert", "./src/shader.frag");
	init_VAOs(scop);
	init_camera(scop);
	load_texture(scop, argv[2]);
}
