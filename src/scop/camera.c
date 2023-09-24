/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   camera.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncoursol <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/04/14 13:55:17 by ncoursol          #+#    #+#             */
/*   Updated: 2022/04/14 13:55:58 by ncoursol         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/scop.h"

void		set_scale(scop_t *scop)
{
	int		tmp;

	scop->camera->scale = 1;
	scop->obj = scop->obj->first;

	float max[3] = {scop->obj->max[0], scop->obj->max[1], scop->obj->max[2]};
	float min[3] = {scop->obj->min[0], scop->obj->min[1], scop->obj->min[2]};
	while (scop->obj != NULL) {
		for (int i = 0; i < 3; i++) {
			if (scop->obj->max[i] > max[i])
				max[i] = scop->obj->max[i];
			if (scop->obj->min[i] < min[i])
				min[i] = scop->obj->min[i];
		}
		if (scop->obj->next == NULL)
			break;
		scop->obj = scop->obj->next;
	}
	for (int i = 0; i < 3; i++) {
		scop->camera->max[i] = max[i];
		scop->camera->min[i] = min[i];
		tmp = sqrt(powf(max[i] - min[i], 2));
		if (tmp > scop->camera->scale)
			scop->camera->scale = tmp;
	}
	scop->camera->scale = 10 / scop->camera->scale;
}

GLfloat *perspective(float angle, float ratio, float near, float far)
{
		GLfloat *ret;
    GLfloat half_tan;

		ret = new_mat4();
		half_tan = 1.0f / tanf(RAD(angle / 2.0f));
		ret[0] = half_tan / ratio;
		ret[5] = half_tan;
		ret[10] = (-near - far) / (near - far);
		ret[11] = 1.0f;
		ret[14] = 2.0f * far * near / (near - far);

    return (ret);
}

GLfloat *lookAt( const vec3_t eye, const vec3_t center, const vec3_t up)
{
    GLfloat *matrix;

		matrix = new_mat4_model();
    vec3_t x, y, z;
		z = norm_vec3(sub_vec3(center, eye));
    x = cross_vec3(up, z);
    y = norm_vec3(cross_vec3(z, x));
    x = norm_vec3(x);

    matrix[0] = x.x;
    matrix[1] = y.x;
    matrix[2] = z.x;

    matrix[4] = x.y;
    matrix[5] = y.y;
    matrix[6] = z.y;

    matrix[8] = x.z;
    matrix[9] = y.z;
    matrix[10] = z.z;

    matrix[12] = -dot_vec3(x, eye);
    matrix[13] = -dot_vec3(y, eye);
    matrix[14] = -dot_vec3(z, eye);

    return (matrix);
}

void	camera(scop_t *scop, vec3_t eye, vec3_t center, vec3_t up)
{
	float *mv;

	if (scop->camera->mvp != NULL)
		free_null((void*)scop->camera->mvp);
	scop->shaderID.mvpID = glGetUniformLocation(scop->shader->shaderProgram, "MVP");
	scop->camera->projection = perspective(scop->camera->fov, 4.0f / 3.0f, 0.1f, 100.0f);
	scop->camera->view = lookAt(eye, center, up);
	scop->camera->model = new_mat4_model();
	scop->camera->model[0] = scop->camera->scale;
	scop->camera->model[5] = scop->camera->scale;
	scop->camera->model[10] = scop->camera->scale;
	mv = prod_mat4(scop->camera->model, scop->camera->view);
	scop->camera->mvp = prod_mat4(mv, scop->camera->projection);

	free_null((void*)scop->camera->projection);
	free_null((void*)scop->camera->view);
	free_null((void*)scop->camera->model);
	free_null((void*)mv);
}

void	set_shader_var(scop_t *scop)
{
	glUseProgram(scop->shader->shaderProgram);
	scop->shaderID.movementID = glGetUniformLocation(scop->shader->shaderProgram, "movement");
	scop->shaderID.transitionID = glGetUniformLocation(scop->shader->shaderProgram, "alpha");
	scop->shaderID.textStateID = glGetUniformLocation(scop->shader->shaderProgram, "texture_state");
	glUniform1i(scop->shaderID.textStateID, TEXTURE);
	scop->shaderID.colorStateID = glGetUniformLocation(scop->shader->shaderProgram, "color_state");
	glUniform1i(scop->shaderID.colorStateID, COLOR);
}

void	init_camera(scop_t *scop)
{
	vec3_t 	eye;
	vec3_t 	up;

	set_scale(scop);
	scop->camera->options = 11; // 00001011 (see boolean options include/scop.h)
	scop->camera->fov = 45.0f;
	scop->camera->ah = RAD(10.0f);
	scop->camera->av = RAD(120.0f);
	scop->camera->gh = 0.0f;
	scop->camera->gv = 0.0f;
	scop->camera->gap = (vec3_t){0, 0, 0};
	scop->camera->projection = NULL;
	scop->camera->view = NULL;
	scop->camera->model = NULL;
	scop->camera->mvp = NULL;
	scop->camera->center = (vec3_t){
		(scop->camera->scale * (scop->camera->max[0] + scop->camera->min[0])) / 2,
		(scop->camera->scale * (scop->camera->max[1] + scop->camera->min[1])) / 2,
		(scop->camera->scale * (scop->camera->max[2] + scop->camera->min[2])) / 2
	};
	scop->camera->dist = 20; 
	up = (vec3_t){0, 1, 0};
	eye = (vec3_t){
		scop->camera->dist * sinf(scop->camera->av) * cosf(scop->camera->ah), 
		scop->camera->dist * sinf(scop->camera->av) * sinf(scop->camera->ah),
		scop->camera->dist * cosf(scop->camera->av)
	};

	set_shader_var(scop);
	glfwSetCursorPos(scop->window, scop->camera->ah, scop->camera->av);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	camera(scop, eye, scop->camera->center, up);
}
