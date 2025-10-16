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
	ret[10] = -(near + far) / (far - near);
	ret[11] = -1.0f;
	ret[14] = -(2.0f * far * near) / (far - near);

	return (ret);
}

GLfloat *orthographic(float left, float right, float bottom, float top, float near, float far)
{
	GLfloat *ret;

	ret = new_mat4();
	ret[0] = 2.0f / (right - left);
	ret[5] = 2.0f / (top - bottom);
	ret[10] = -2.0f / (far - near);
	ret[12] = -(right + left) / (right - left);
	ret[13] = -(top + bottom) / (top - bottom);
	ret[14] = -(far + near) / (far - near);
	ret[15] = 1.0f;

	return (ret);
}

GLfloat *lookAt(const vec3_t eye, const vec3_t center, const vec3_t up)
{
	GLfloat *matrix;

	matrix = new_mat4_model();
	vec3_t z = norm_vec3(sub_vec3(eye, center)); // Backward
	vec3_t x = norm_vec3(cross_vec3(up, z));     // Right
	vec3_t y = cross_vec3(z, x);                 // Up
	x = norm_vec3(x);

	matrix[0] = x.x;
	matrix[1] = y.x;
	matrix[2] = z.x; // Not -z.x

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
	// Free previous matrices if they exist
	if (gomo->camera->mvp) {
		free(gomo->camera->mvp);
		gomo->camera->mvp = NULL;
	}
	if (gomo->camera->projection) {
		free(gomo->camera->projection);
		gomo->camera->projection = NULL;
	}
	if (gomo->camera->view) {
		free(gomo->camera->view);
		gomo->camera->view = NULL;
	}
	if (gomo->camera->ortho) {
		free(gomo->camera->ortho);
		gomo->camera->ortho = NULL;
	}
	
	gomo->camera->projection = perspective(gomo->camera->fov, (float)WIDTH / (float)HEIGHT, 0.2f, 250.0f);
	gomo->camera->view = lookAt(gomo->camera->eye, center, up);
	gomo->camera->model = new_mat4_model();

	gomo->camera->mvp = prod_mat4(prod_mat4(gomo->camera->model, gomo->camera->view), gomo->camera->projection);
	gomo->camera->ortho = orthographic(0, WIDTH, 0, HEIGHT, 0.0f, 1000.0f);

	free(gomo->camera->model);
	gomo->camera->model = NULL;
}

void set_new_eye(gomo_t *gomo, vec3_t up)
{
	float d, ah, av;

	d = gomo->camera->dist;
	ah = gomo->camera->ah;
	av = gomo->camera->av;

	if (av < 1.535f)
		av = 1.535f;
	if (up.z)
	{
		gomo->camera->eye = (vec3_t){
			(d * -sinf(av) * cosf(ah)) + gomo->camera->center.x,
			(d * -sinf(av) * sinf(ah)) + gomo->camera->center.y,
			(d * -cosf(av)) + gomo->camera->center.z
		};
	}
	else
	{
		gomo->camera->eye = (vec3_t){
			(d * -sinf(av) * cosf(ah)) + gomo->camera->center.x,
			(d * -cosf(av)) + gomo->camera->center.y,
			(d * -sinf(av) * sinf(ah)) + gomo->camera->center.z,
		};
	}
}

void set_new_camera_angles(gomo_t *gomo)
{
	double xpos, ypos;
	float tmp;

	glfwGetCursorPos(gomo->window, &xpos, &ypos);
	gomo->camera->ah = xpos * MSPEED;
	tmp = ypos * MSPEED;
	if (tmp > PI - 0.05f)
	{
		gomo->camera->av = PI - 0.05f;
		glfwSetCursorPos(gomo->window, gomo->camera->ah / MSPEED, gomo->camera->av / MSPEED);
	}
	else if (tmp < 1.0f)
	{
		gomo->camera->av = 1.0f;
		glfwSetCursorPos(gomo->window, gomo->camera->ah / MSPEED, gomo->camera->av / MSPEED);
	}
	else
		gomo->camera->av = tmp;
}

void updateCamera(gomo_t *gomo, float delta_time)
{
	vec3_t new_center;
	vec3_t up;

	up = (vec3_t){0, 1, 0};
	if (ROTATION) {
		gomo->camera->ah -= 0.2f * delta_time;  // 0.2 radians per second
		gomo->camera->ah = fmodf(gomo->camera->ah, 2 * PI); // Keep ah within [0, 2π]
	}

	if (LEFT_MOUSE && !(TOP_VIEW) && !(ANIMATE))
		set_new_camera_angles(gomo);

	set_new_eye(gomo, up);
	new_center = gomo->camera->center;
	camera(gomo, new_center, up);

}

int animateCamera(gomo_t *gomo, vec3_t targetPos, vec3_t targetCenter, float threshold, float lerp_speed, float delta_time)
{
	float target_av = targetPos.x;
	float target_ah = targetPos.y;
	float target_dist = targetPos.z;
	
	// Check if we need to animate
	if (fabsf(gomo->camera->av - target_av) > threshold || 
		fabsf(gomo->camera->ah - target_ah) > threshold || 
		fabsf(gomo->camera->dist - target_dist) > threshold ||
		fabsf(gomo->camera->center.x - targetCenter.x) > threshold ||
		fabsf(gomo->camera->center.y - targetCenter.y) > threshold ||
		fabsf(gomo->camera->center.z - targetCenter.z) > threshold) {

		// Smooth interpolation for center position
		gomo->camera->center.x += (targetCenter.x - gomo->camera->center.x) * lerp_speed * delta_time;
		gomo->camera->center.y += (targetCenter.y - gomo->camera->center.y) * lerp_speed * delta_time;
		gomo->camera->center.z += (targetCenter.z - gomo->camera->center.z) * lerp_speed * delta_time;

		// Smooth interpolation with easing
		gomo->camera->av += (target_av - gomo->camera->av) * lerp_speed * delta_time;
		gomo->camera->ah += (target_ah - gomo->camera->ah) * lerp_speed * delta_time;
		gomo->camera->dist += (target_dist - gomo->camera->dist) * lerp_speed * delta_time;

		// Snap to target values when very close
		if (fabsf(target_av - gomo->camera->av) < threshold)
			gomo->camera->av = target_av;
		if (fabsf(target_ah - gomo->camera->ah) < threshold)
			gomo->camera->ah = target_ah;
		if (fabsf(target_dist - gomo->camera->dist) < threshold)
			gomo->camera->dist = target_dist;
		if (fabsf(targetCenter.x - gomo->camera->center.x) < threshold)
			gomo->camera->center.x = targetCenter.x;
		if (fabsf(targetCenter.y - gomo->camera->center.y) < threshold)
			gomo->camera->center.y = targetCenter.y;
		if (fabsf(targetCenter.z - gomo->camera->center.z) < threshold)
			gomo->camera->center.z = targetCenter.z;

		return 1; // Animation in progress
	}
	
	return 0; // Animation complete
}

void handleCameraAnimations(gomo_t *gomo, float delta_time)
{
	if (ANIMATE) {
		int animating = animateCamera(gomo, gomo->camera->targetPos, gomo->camera->targetCenter, 0.001f, 4.0f, delta_time);

		if (!animating) {
			gomo->camera->options ^= 1 << 1;
			if (MENU && !(ROTATION) && gomo->camera->targetPos.x == RAD(120.0f) && gomo->camera->targetPos.y == RAD(10.0f) && gomo->camera->targetPos.z == 3.0f) {
				gomo->camera->options ^= 1 << 4;
			} else if (!(MENU)) {
				gomo->textHover = -1;
			}
		}
	}
}

void init_camera(gomo_t *gomo)
{
	vec3_t up;

	set_scale(gomo);
	gomo->camera->options = 0b01010000; // 01010000 (see boolean options in include/gomo.h)
	gomo->camera->fov = 60.0f;
	gomo->camera->ah = RAD(10.0f);
	gomo->camera->av = RAD(120.0f);
	gomo->camera->gh = 0.0f;
	gomo->camera->gv = 0.0f;
	gomo->camera->eye = (vec3_t){0, 0, 0};
	gomo->camera->projection = NULL;
	gomo->camera->view = NULL;
	gomo->camera->model = NULL;
	gomo->camera->mvp = NULL;
	gomo->camera->center = (vec3_t){0, 0.5, 0};
	gomo->camera->dist = 3.0f;
	gomo->camera->targetCenter = gomo->camera->center;
	gomo->camera->targetPos = (vec3_t){gomo->camera->av, gomo->camera->ah, gomo->camera->dist};
	up = (vec3_t){0, 1, 0};
	glUseProgram(gomo->shader->shaderProgram);
	glfwSetCursorPos(gomo->window, gomo->camera->ah, gomo->camera->av);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	camera(gomo, gomo->camera->center, up);
}