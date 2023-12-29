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

void	init_glfw(gomo_t *gomo)
{
	if (!glfwInit())
		exit_callback(gomo, 24, "glfwInit failed");

	/* Set window spec. */
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	/* Create a windowed mode window and its OpenGL context */
	if(!(gomo->window = glfwCreateWindow(WIDTH, HEIGHT, "gomo Ncoursol", NULL, NULL)))
		exit_callback(gomo, 25, "glfwCreateWindow failed");

	/* Set callback functions */
	glfwSetWindowUserPointer(gomo->window, gomo);
	glfwSetKeyCallback(gomo->window, key_callback);
	glfwSetScrollCallback(gomo->window, scroll_callback);
	glfwSetMouseButtonCallback(gomo->window, mouse_button_callback);

	/* Make the window's context current */
	glfwMakeContextCurrent(gomo->window);

	glfwSetInputMode(gomo->window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetCursorPos(gomo->window, WIDTH / 2, HEIGHT / 2);
}

void	init_gl(gomo_t *gomo)
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
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void	VAOs(gomo_t *gomo, obj_t *obj)
{
	GLenum	errCode;
	
	// VAO
	glGenVertexArrays(1, &obj->VAO);  
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 44, getErrorString(errCode));
	glBindVertexArray(gomo->obj->VAO);
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 45, getErrorString(errCode));

	// VBO
	glGenBuffers(1, &gomo->obj->VBO);
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 46, getErrorString(errCode));
	glBindBuffer(GL_ARRAY_BUFFER, gomo->obj->VBO);
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 47, getErrorString(errCode));
	glBufferData(GL_ARRAY_BUFFER,
			gomo->obj->nb_triangles * 8 * sizeof(float),
			&gomo->obj->obj[0],
			GL_STATIC_DRAW);
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 48, getErrorString(errCode));

	// Vertex format(Coordinate/TextureUV/Color): x1,y1,z1, u1,v1, r1,g1,b1, x2...

	// Coordinate
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 49, getErrorString(errCode));
	glEnableVertexAttribArray(0);
	// TextureUV
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 50, getErrorString(errCode));
	glEnableVertexAttribArray(1);
	// Color
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 51, getErrorString(errCode));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (gomo->obj->id == 1) {
		glGenBuffers(1, &gomo->stone_VBO);
		glBindBuffer(GL_ARRAY_BUFFER, gomo->stone_VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * 361, &gomo->stone_coord[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glVertexAttribDivisor(3, 1);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	glBindVertexArray(0);
}

void	init_VAOs(gomo_t *gomo)
{
	gomo->obj = gomo->obj->first;
	while (gomo->obj != NULL) {
		VAOs(gomo, gomo->obj);
		if (gomo->obj->next == NULL)
			break;
		gomo->obj = gomo->obj->next;
	}
	gomo->obj = gomo->obj->first;
}

void	init_face_data(gomo_t *gomo, int nb_buff)
{
	for (int i = (V_BUFF_SIZE * nb_buff) - V_BUFF_SIZE; i < V_BUFF_SIZE * nb_buff; i++) {
		if (!(gomo->obj->faces[i] = (data_t*)malloc(sizeof(data_t))))
			exit_callback(gomo, 7, "faces data malloc failed");
		gomo->obj->faces[i]->vertex = 0;
		gomo->obj->faces[i]->texture = 0;
		gomo->obj->faces[i]->normal = 0;
		gomo->obj->faces[i]->next = NULL;
		gomo->obj->faces[i]->first = gomo->obj->faces[i];
	}
}

void	init_obj(gomo_t *gomo)
{
	gomo->obj->name = NULL;
	gomo->obj->matName = NULL;
	gomo->obj->matDef = NULL;
	gomo->obj->faces = NULL;
	gomo->obj->smooth = 0;
	gomo->obj->texCoord = 0;
	gomo->obj->faces_size = 0;
	gomo->obj->nb_faces = 0;
	gomo->obj->nb_triangles = 0;
	if (!(gomo->obj->faces = (data_t**)malloc(sizeof(data_t*) * V_BUFF_SIZE)))
		exit_callback(gomo, 6, "faces malloc failed");
	gomo->obj->faces_size = V_BUFF_SIZE;
	gomo->obj->obj = NULL;
	for (int i = 0; i < 3; i++) {
		gomo->obj->min[i] = 0;
		gomo->obj->max[i] = 0;
	}
	gomo->obj->next = NULL;
	init_face_data(gomo, 1);
}

void	init_gomo(gomo_t *gomo)
{
	gomo->obj = NULL;
	gomo->vertices = NULL;
	gomo->textures = NULL;
	gomo->normals = NULL;
	gomo->shader = NULL;
	gomo->camera = NULL;
	if (!(gomo->obj = (obj_t*)malloc(sizeof(obj_t))))
		exit_callback(gomo, 0, "object malloc failed");
	gomo->obj->id = 0;
	gomo->nb_vertices = 0;
	gomo->nb_textures = 0;
	gomo->nb_normals = 0;
	gomo->tmp_id.x = 0;
	gomo->tmp_id.y = 0;
	gomo->tmp_id.z = 0;
	if (!(gomo->stone_coord = (float*)malloc(sizeof(float) * 19 * 19 * 3)))
		exit_callback(gomo, 0, "stone_coord malloc failed");
	for (int i = 0; i < 1083; i++)
		gomo->stone_coord[i] = 100;
	gomo->nb_stones = 0;
	if (!(gomo->vertices = (float*)malloc(sizeof(float) * V_BUFF_SIZE)))
		exit_callback(gomo, 1, "vertices malloc failed");
	if (!(gomo->textures = (float*)malloc(sizeof(float) * V_BUFF_SIZE)))
		exit_callback(gomo, 2, "textures malloc failed");
	if (!(gomo->normals = (float*)malloc(sizeof(float) * V_BUFF_SIZE)))
		exit_callback(gomo, 3, "normals malloc failed");
	if (!(gomo->shader = (shader_t*)malloc(sizeof(shader_t))))
		exit_callback(gomo, 4, "vertices malloc failed");
	if (!(gomo->camera = (camera_t*)malloc(sizeof(camera_t))))
		exit_callback(gomo, 5, "camera malloc failed");
}

void	init_all(gomo_t *gomo)
{
	init_gomo(gomo);
	init_obj(gomo);
	gomo->obj->first = gomo->obj;
	load_obj(gomo, "resources/goban.obj");
	init_glfw(gomo);
	init_gl(gomo);
	init_shader(gomo, gomo->shader, "./src/shader.vert", "./src/shader.frag");
	init_VAOs(gomo);
	init_camera(gomo);
	load_texture(gomo);
}
