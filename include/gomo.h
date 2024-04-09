/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   gomo.h                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncoursol <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/04/04 16:58:59 by ncoursol          #+#    #+#             */
/*   Updated: 2022/04/17 20:54:01 by ncoursol         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef GOMO_H
#define GOMO_H

#include "../ext/glad/include/glad.h"
#include "../ext/glfw-3.3.5/include/GLFW/glfw3.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define WIDTH 1280 // 1024
#define HEIGHT 960 // 768

#define MSPEED 0.005f // Mouse speed
#define PI 3.14159265359
#define V_BUFF_SIZE 20000
#define AXIS gomo->camera->options & 1			  // top axis (y/z)
#define LEFT_MOUSE gomo->camera->options >> 5 & 1 // left mouse button press (yes/no)
#define TOP_VIEW gomo->camera->options >> 8 & 1	  // top view (yes/no)
#define RAY_T_MIN 0.0001f // Minimum ray t value

#define ABS(x) (x >= 0 ? x : -x)
#define RAD(x) (x * 0.0174533f)

typedef struct vec3_s
{
	float x;
	float y;
	float z;
} vec3_t;

typedef struct vec4_t
{
	float x;
	float y;
	float z;
	float w;
} vec4_t;

typedef struct hit_s
{
	int hit;
	float t;
	vec3_t point;
	vec3_t normal;
} hit_t;

typedef struct ray_s
{
	vec3_t origin;
	vec3_t direction;
} ray_t;

typedef struct grid_s
{
	int state;
	vec3_t color;
	vec3_t pos;
} grid_t;

typedef struct instance_s
{
	vec3_t pos;
	vec3_t color;
} instance_t;

typedef struct data_s
{
	int vertex;
	int texture;
	int normal;
	struct data_s *next;
	struct data_s *first;
} data_t;

typedef struct camera_s
{
	vec3_t center;		  // Center of the scene
	vec3_t gap;			  // Gap between the old and the new scene center
	vec3_t eye;			  // Eye of the cameragt
	unsigned int options; // See #define section
	float gv;			  // Screen vertical gap
	float gh;			  // Screen horizontal gap
	float av;			  // Vertical camera rotation angle
	float ah;			  // Horizontal camera rotation angle
	float dist;			  // Distance between camera and center
	float scale;		  // Scale apply to the scene
	float fov;			  // Field Of View of the camera
	float max[3];
	float min[3];
	GLfloat *projection; // Projection Matrice
	GLfloat *view;		 // View Matrice
	GLfloat *model;		 // Model Matrice
	GLfloat *mvp;		 // Model x View x Projection
} camera_t;

typedef struct obj_s // Chained list of each object of the scene
{
	int id;			 // Id of the object
	int smooth;		 // Smooth value
	int nb_faces;	 // Number of faces
	int nb_vertices; // Number of triangles
	int texCoord;	 // Texture coordinate
	int faces_size;	 // Size of faces array
	data_t **faces;	 // All indices (see arrays in gomo_s below) of each faces
	float *obj;		 // All vertices data ready for render (x1,y1,z1,u1,v1,r1,g1,b1,x2,...)
	float max[3];
	float min[3];
	unsigned int VBO;	 // Vertex Buffer Object
	unsigned int VAO;	 // Vertex Array Object
	struct obj_s *next;	 // Next object
	struct obj_s *first; // First object
} obj_t;

typedef struct shaderID_s
{
	GLuint textureID1;	// Texture ID for the first texture
	GLuint textureID2;	// Texture ID for the second texture
	GLuint mvpID;		// MVP ID
	GLuint mvp_stoneID; // MVP ID for instances go stones
} shaderID_t;

typedef struct shader_s
{
	const char *vertexShaderSrc;   // Vertex shader source code
	const char *fragmentShaderSrc; // Fragment shader source code
	unsigned int vertexShader;	   // Vertex shader
	unsigned int fragmentShader;   // Fragment shader
	unsigned int shaderProgram;	   // Shader program
} shader_t;

typedef struct gomo_s
{
	GLFWwindow *window;
	shader_t *shader;
	camera_t *camera;
	shaderID_t shaderID;
	GLuint grid_text;
	GLuint wood_text;
	instance_t *stone;
	unsigned int instanceVBO; // Vertex Buffer Object for instances go stones
	grid_t *board;
	int nb_stones;
	int tmp_stone;
	hit_t tmp_hit;
	obj_t *obj;
} gomo_t;

// Callbacks fct
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow *window, int key, int action, int mods);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_move_callback(GLFWwindow *window, double xpos, double ypos);
void exit_callback(gomo_t *gomo, int state, char *description);
char *getErrorString(int code);

// Utils fct
void *realloc_s(void **ptr, size_t taille);
char *strjoin(const char *s1, const char *s2);
float *float_copy(gomo_t *gomo, float *dest, int len, char *line, int *nb, int *nb_buff);
char *string_copy(gomo_t *gomo, char *dest, char *line);
data_t *data_copy(data_t *a, data_t *b);
int count_space(char *a);

// Init fct
void init_all(gomo_t *gomo);
void init_camera(gomo_t *gomo);
void init_shader(gomo_t *gomo, shader_t *shader, char *vert_src, char *frag_src);
void init_obj(gomo_t *gomo);
void init_face_data(gomo_t *gomo, int nb_buff);
void init_text(gomo_t *gomo);
void load_obj(gomo_t *gomo, char *path);
void load_texture(gomo_t *gomo);
void VAOs(gomo_t *gomo, obj_t *obj);

// Matrice4 fct
GLfloat *new_mat4(void);
GLfloat *new_mat4_model(void);
GLfloat *prod_mat4(float *a, float *b);
vec4_t mulv_mat4(float *m, vec4_t v);
float *inv_mat4(float *m);

// Vector3 fct
GLfloat dot_vec3(vec3_t a, vec3_t b);
float dist_btw_two_vec3(vec3_t a, vec3_t b);
vec3_t prod_vec3(vec3_t, float b);
vec3_t cross_vec3(vec3_t a, vec3_t b);
vec3_t norm_vec3(vec3_t a);
vec3_t sub_vec3(vec3_t a, vec3_t b);
vec3_t add_vec3(vec3_t a, vec3_t b);

// Parser fct
void new_vertex(gomo_t *gomo, int f);
void create_obj(gomo_t *gomo, float *vertices, float *normals, float *textures);

// Render fct
void camera(gomo_t *gomo, vec3_t center, vec3_t up);

// Exit fct
void free_all(gomo_t *gomo, int step);
void free_null(void *a);

#endif
