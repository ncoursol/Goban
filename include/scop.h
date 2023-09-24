/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scop.h                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncoursol <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/04/04 16:58:59 by ncoursol          #+#    #+#             */
/*   Updated: 2022/04/17 20:54:01 by ncoursol         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SCOP_H
#define SCOP_H

#include "../ext/glad/include/glad.h"
#include "../ext/glfw-3.3.5/include/GLFW/glfw3.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define WIDTH 1024
#define HEIGHT 768

#define MSPEED 0.01f					// Mouse speed
#define PI 3.14159265359
#define V_BUFF_SIZE 20000
															// scop->camera->options (int) to (7 Boolean) :
                                                      // 11111111
#define AXIS scop->camera->options & 1                // |||||||╰> top axis (y/z)
#define ROTATION scop.camera->options >> 1 & 1        // ||||||╰−> rotation (on/off)
#define MOVEMENT scop.camera->options >> 2 & 1        // |||||╰−−> movement (on/off)
#define TEXTURE scop->camera->options >> 3 & 1        // ||||╰−−−> texture	(on/off)
#define COLOR scop->camera->options >> 4 & 1          // |||╰−−−−> color (on/off)
#define LEFT_MOUSE scop->camera->options >> 5 & 1     // ||╰−−−−−> left mouse button press (yes/no)
#define MIDDLE_MOUSE scop->camera->options >> 6 & 1   // |╰−−−−−−> middle mouse button press (yes/no)
#define TRANSITION scop.camera->options >> 7 & 1   		// ╰−−−−−−−> texture transition (yes/no)
#define TOP_VIEW scop->camera->options >> 8 & 1

#define ABS(x) (x >= 0 ? x : -x)
#define RAD(x) (x * 0.0174533f)

typedef	struct	vec3_s
{
	float					x;
	float					y;
	float					z;
}								vec3_t;

typedef	struct	data_s
{
	int						vertex;
	int						texture;
	int						normal;
	struct data_s *next;
	struct data_s *first;
}								data_t;

typedef struct	camera_s
{
	vec3_t				center;				// Center of the scene
	vec3_t				gap;					// Gap between the old and the new scene center
	unsigned int	options;			// See #define section
	float					gv;						// Screen vertical gap
	float					gh;						// Screen horizontal gap
	float					av;						// Vertical camera rotation angle
	float					ah;						// Horizontal camera rotation angle
	float					dist;					// Distance between camera and center
	float					scale;				// Scale apply to the scene
	float					fov;					// Field Of View of the camera
	float					max[3];
	float					min[3];
	GLfloat				*projection;
	GLfloat				*view;
	GLfloat				*model;
	GLfloat				*mvp;					// Model x View x Projection Matrice
}								camera_t;

typedef struct	obj_s					// Chained list of each object of the scene
{
	int						id;
	char					*name;
	char					*matName;
	char					*matDef;
	int						smooth;
	int						nb_faces;
	int						nb_triangles;
	int						texCoord;
	int						faces_size;
	data_t				**faces;			// All indices (see arrays in scop_s below) of each faces
	float					*obj;					// All vertices data ready for render (x1,y1,z1,u1,v1,r1,g1,b1,x2,...)
	float					max[3];
	float					min[3];
	unsigned int 	VBO;					// Vertex Buffer Object
	unsigned int 	VAO;					// Vertex Array Object
	struct obj_s 	*next;
	struct obj_s	*first;
}								obj_t;

typedef	struct	shaderID_s 
{
	GLuint				textureID;
	GLuint				movementID;
	GLuint				transitionID;
	GLuint				textStateID;
	GLuint				colorStateID;
	GLuint				mvpID;
}								shaderID_t;

typedef	struct	shader_s
{
	const char		*vertexShaderSrc;
	const char		*fragmentShaderSrc;
	unsigned int 	vertexShader;
	unsigned int 	fragmentShader;
	unsigned int 	shaderProgram;
}								shader_t;

typedef struct	scop_s
{
	GLFWwindow		*window;
	shader_t			*shader;
	camera_t			*camera;
	shaderID_t		shaderID;
	GLuint				texture;
	int						nb_vertices;
	int						nb_textures;
	int						nb_normals;
	float					*vertices;		// Array of all vertices in the scene (x1,y1,z1,x2,y2,z2,...)
	float					*textures;		// Array of all textures coordinate in the scene (u1,v1,u2,v2,...)
	float					*normals;			// Array of all normals in the scene (x1,y1,z1,x2,y2,z2,...)
	obj_t					*obj;
}								scop_t;

// Callbacks fct
void 			key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void 			mouse_button_callback(GLFWwindow* window, int key, int action, int mods);
void 			scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void			framebuffer_size_callback(GLFWwindow* window, int width, int height);
void			exit_callback(scop_t *scop, int state, char *description);
char			*getErrorString(int code);

// Utils fct
void			*realloc_s(void **ptr, size_t taille);
char			*strjoin(const char *s1, const char *s2);
float			*float_copy(scop_t *scop, float *dest, int len, char *line, int *nb, int *nb_buff);
char			*string_copy(scop_t *scop, char *dest, char *line);
data_t		*data_copy(data_t *a, data_t *b);
int				count_space(char *a);

// Init fct
void			init_all(scop_t *scop, char **argv);
void			init_camera(scop_t *scop);
void			init_shader(scop_t *scop, shader_t *shader, char *vert_src, char *frag_src);
void			init_obj(scop_t *scop);
void			init_face_data(scop_t *scop, int nb_buff);
void			init_text(scop_t *scop);
void			load_obj(scop_t *scop, char **argv);
void			load_texture(scop_t *scop, char *path);

// Matrice4 fct
GLfloat		*new_mat4(void);
GLfloat		*new_mat4_model(void);
GLfloat		*prod_mat4(float *a, float *b);

// Vector3 fct
GLfloat		dot_vec3(vec3_t a, vec3_t b);
float			dist_btw_two_vec3(vec3_t a, vec3_t b);
vec3_t		prod_vec3(vec3_t, float b);
vec3_t		cross_vec3(vec3_t a, vec3_t b);
vec3_t		norm_vec3(vec3_t a);
vec3_t		sub_vec3(vec3_t a, vec3_t b);
vec3_t		add_vec3(vec3_t a, vec3_t b);

// Parser fct
void			triangulate(scop_t *scop, int *f, int *b_f, int nb);
void			new_vertex(scop_t *scop, int f);
void			new_obj(scop_t *scop, char *line);
void			create_obj(scop_t *scop);

// Render fct
void			camera(scop_t *scop, vec3_t eye, vec3_t center, vec3_t up);

// Exit fct
void			free_all(scop_t *scop, int step);
void			free_null(void *a);

#endif
