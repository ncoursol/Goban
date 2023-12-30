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

#include "../include/gomo.h"

void set_scale(gomo_t *gomo)
{
	int tmp;

	gomo->camera->scale = 1;
	gomo->obj = gomo->obj->first;

	float max[3] = {gomo->obj->max[0], gomo->obj->max[1], gomo->obj->max[2]};
	float min[3] = {gomo->obj->min[0], gomo->obj->min[1], gomo->obj->min[2]};
	while (gomo->obj != NULL)
	{
		for (int i = 0; i < 3; i++)
		{
			if (gomo->obj->max[i] > max[i])
				max[i] = gomo->obj->max[i];
			if (gomo->obj->min[i] < min[i])
				min[i] = gomo->obj->min[i];
		}
		if (gomo->obj->next == NULL)
			break;
		gomo->obj = gomo->obj->next;
	}
	for (int i = 0; i < 3; i++)
	{
		gomo->camera->max[i] = max[i];
		gomo->camera->min[i] = min[i];
		tmp = sqrt(powf(max[i] - min[i], 2));
		if (tmp > gomo->camera->scale)
			gomo->camera->scale = tmp;
	}
	gomo->camera->scale = 10 / gomo->camera->scale;
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

GLfloat *lookAt(const vec3_t eye, const vec3_t center, const vec3_t up)
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

void camera(gomo_t *gomo, vec3_t center, vec3_t up)
{
	float *mv;

	if (gomo->camera->mvp != NULL)
		free_null((void *)gomo->camera->mvp);
	gomo->shaderID.mvpID = glGetUniformLocation(gomo->shader->shaderProgram, "MVP");
	gomo->camera->projection = perspective(gomo->camera->fov, 4.0f / 3.0f, 0.1f, 100.0f);
	gomo->camera->view = lookAt(gomo->camera->eye, center, up);
	gomo->camera->model = new_mat4_model();
	gomo->camera->model[0] = gomo->camera->scale;
	gomo->camera->model[5] = gomo->camera->scale;
	gomo->camera->model[10] = gomo->camera->scale;
	mv = prod_mat4(gomo->camera->model, gomo->camera->view);
	gomo->camera->mvp = prod_mat4(mv, gomo->camera->projection);

	free_null((void *)gomo->camera->projection);
	free_null((void *)gomo->camera->view);
	free_null((void *)gomo->camera->model);
	free_null((void *)mv);
}

void init_camera(gomo_t *gomo)
{
	vec3_t eye;
	vec3_t up;

	set_scale(gomo);
	gomo->camera->options = 11; // 00001011 (see boolean options include/gomo.h)
	gomo->camera->fov = 45.0f;
	gomo->camera->ah = RAD(10.0f);
	gomo->camera->av = RAD(120.0f);
	gomo->camera->gh = 0.0f;
	gomo->camera->gv = 0.0f;
	gomo->camera->eye = (vec3_t){0, 0, 0};
	gomo->camera->gap = (vec3_t){0, 0, 0};
	gomo->camera->projection = NULL;
	gomo->camera->view = NULL;
	gomo->camera->model = NULL;
	gomo->camera->mvp = NULL;
	gomo->camera->center = (vec3_t){
		(gomo->camera->scale * (gomo->camera->max[0] + gomo->camera->min[0])) / 2,
		(gomo->camera->scale * (gomo->camera->max[1] + gomo->camera->min[1])) / 2,
		(gomo->camera->scale * (gomo->camera->max[2] + gomo->camera->min[2])) / 2};
	gomo->camera->dist = 20;
	up = (vec3_t){0, 1, 0};
	eye = (vec3_t){
		gomo->camera->dist * sinf(gomo->camera->av) * cosf(gomo->camera->ah),
		gomo->camera->dist * sinf(gomo->camera->av) * sinf(gomo->camera->ah),
		gomo->camera->dist * cosf(gomo->camera->av)};
	glUseProgram(gomo->shader->shaderProgram);
	glfwSetCursorPos(gomo->window, gomo->camera->ah, gomo->camera->av);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	camera(gomo, gomo->camera->center, up);
}
