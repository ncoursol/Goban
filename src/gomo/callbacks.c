/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   callbacks.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncoursol <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/04/13 16:01:06 by ncoursol          #+#    #+#             */
/*   Updated: 2023/02/27 15:04:54 by ncoursol         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/gomo.h"

char *getErrorString(int code)
{
	if (code == 0x0500)
		return ("GL_INVALID_ENUM");
	else if (code == 0x0501)
		return ("GL_INVALID_VALUE");
	else if (code == 0x0502)
		return ("GL_INVALID_OPERATION");
	else if (code == 0x0503)
		return ("GL_STACK_OVERFLOW");
	else if (code == 0x0504)
		return ("GL_STACK_UNDERFLOW");
	else if (code == 0x0505)
		return ("GL_OUT_OF_MEMORY");
	else if (code == 0x0506)
		return ("GL_INVALID_FRAMEBUFFER_OPERATION");
	else if (code == 0x0507)
		return ("GL_CONTEXT_LOST");
	else if (code == 0x8031)
		return ("GL_TABLE_TOO_LARGE1");
	else
		return ("GL_ERROR");
	return ("GL_ERROR");
}

void exit_callback(gomo_t *gomo, int state, char *description)
{
	fprintf(stderr, "Error: %s [%d]\n", description, state);
	free_all(gomo, state);
	exit(1);
}

void update_stones(gomo_t *gomo)
{
	int i = 0;
	int index = 0;

	for (i = 0; i < 19 * 19; i++)
	{
		if (gomo->board[i].state == 2 && !gomo->tmp_stone)
			gomo->board[i].state = 0;
		else if (gomo->board[i].state) {
			gomo->stone[index].pos = gomo->board[i].pos;
			gomo->stone[index].color = gomo->board[i].color;
			index++;
		}
	}
	gomo->nb_stones = index;
}

void updateData(gomo_t *gomo)
{
	GLenum errCode;

	update_stones(gomo);
	glUseProgram(gomo->shader->shaderProgramStones);
	gomo->obj = gomo->obj->first->next;
	glBindVertexArray(gomo->obj->VAO);
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 218, getErrorString(errCode));
	glBindBuffer(GL_ARRAY_BUFFER, gomo->instanceVBO);
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 219, getErrorString(errCode));

	instance_t *data = (instance_t *)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 220, getErrorString(errCode));

	memcpy(data, &gomo->stone[0], gomo->nb_stones * sizeof(instance_t));

	glUnmapBuffer(GL_ARRAY_BUFFER);
	if ((errCode = glGetError()) != GL_NO_ERROR)
		exit_callback(gomo, 221, getErrorString(errCode));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

int ray_intersects_quad(gomo_t *gomo, ray_t ray, vec3_t quadPos, float halfWidth, float halfHeight)
{
	// Compute plane normal: use camera->eye and quadPos like the shader does
	vec3_t toCamera = sub_vec3(gomo->camera->eye, quadPos);
	toCamera = norm_vec3(toCamera); // normalize to match shader's normalize()
	float angle = atan2f(toCamera.x, toCamera.z); // matches shader: atan(toCamera.x, toCamera.z)
	float cosA = cosf(angle);
	float sinA = sinf(angle);
	vec3_t quadNormal = (vec3_t){ sinA, 0.0f, cosA };
	// quadNormal is already unit length (sin^2+cos^2==1)

	// Denominator for ray-plane intersection
	float denom = dot_vec3(ray.direction, quadNormal);
	if (fabsf(denom) < 1e-6f)
		return 0; // Ray parallel to quad plane

	// Compute t using general plane intersection: t = dot(P0 - O, N) / dot(D, N)
	vec3_t p0_minus_o = (vec3_t){ quadPos.x - ray.origin.x, quadPos.y - ray.origin.y, quadPos.z - ray.origin.z };
	float t = dot_vec3(p0_minus_o, quadNormal) / denom;
	if (t < 0)
		return 0; // intersection behind origin

	// Intersection point in world space
	vec3_t intersectionPoint;
	intersectionPoint.x = ray.origin.x + t * ray.direction.x;
	intersectionPoint.y = ray.origin.y + t * ray.direction.y;
	intersectionPoint.z = ray.origin.z + t * ray.direction.z;

	// Undo shader rotation: rotate intersection point by -angle around Y to text-local frame
	float dx = intersectionPoint.x - quadPos.x;
	float dz = intersectionPoint.z - quadPos.z;
	float localX = dx * cosA + dz * sinA; // R(-a) * (dx,dz)
	float localY = intersectionPoint.y - quadPos.y; // vertical in text local

	if (localX >= -halfWidth && localX <= halfWidth &&
		localY >= -halfHeight && localY <= halfHeight)
	{
		// debug markers
		draw_line(gomo, (vec3_t){intersectionPoint.x - 0.1f, intersectionPoint.y, intersectionPoint.z}, (vec3_t){intersectionPoint.x + 0.1f, intersectionPoint.y, intersectionPoint.z}, (vec3_t){1.0f, 0.0f, 0.0f});
		draw_line(gomo, (vec3_t){intersectionPoint.x, intersectionPoint.y - 0.1f, intersectionPoint.z}, (vec3_t){intersectionPoint.x, intersectionPoint.y + 0.1f, intersectionPoint.z}, (vec3_t){1.0f, 0.0f, 0.0f});
		draw_line(gomo, (vec3_t){intersectionPoint.x, intersectionPoint.y, intersectionPoint.z - 0.1f}, (vec3_t){intersectionPoint.x, intersectionPoint.y, intersectionPoint.z + 0.1f}, (vec3_t){1.0f, 0.0f, 0.0f});
		// draw normal as a line starting from intersectionPoint in world space
		vec3_t normalEnd = add_vec3(intersectionPoint, prod_vec3(quadNormal, 0.5f));
		draw_line(gomo, intersectionPoint, normalEnd, (vec3_t){0.0f, 1.0f, 0.0f});
		return 1;
	}

	return 0;
}

int intersectText(gomo_t *gomo, ray_t ray, hit_t *intersection)
{
	for (int i = 0; i < NB_TEXT; i++)
	{
		text_t *text = &gomo->text[i];
		if (!text->id || !text->text || text->proj != 1 || text->id == 2)
			continue;

		float textWidth = calculate_text_width(gomo, text->font, text->text, text->scale);
		float textHeight = 48.0f * text->scale;
		float halfWidth = textWidth * 0.5f;
		float halfHeight = textHeight * 0.5f;

		// Use stored text->pos as center; ray_intersects_quad will undo shader rotation and test in text-local XY
		if (ray_intersects_quad(gomo, ray, text->pos, halfWidth, halfHeight))
		{
			// recompute t for hit point
			float angle = gomo->camera->ah;
			vec3_t quadNormal = (vec3_t){ sinf(angle), 0.0f, cosf(angle) };
			float denom = dot_vec3(ray.direction, quadNormal);
			if (fabsf(denom) < 1e-6f)
				continue;
			vec3_t p0_minus_o = (vec3_t){ text->pos.x - ray.origin.x, text->pos.y - ray.origin.y, text->pos.z - ray.origin.z };
			float t = dot_vec3(p0_minus_o, quadNormal) / denom;
			if (t < 0)
				continue;

			intersection->hit = text->id;
			intersection->point.x = ray.origin.x + t * ray.direction.x;
			intersection->point.y = ray.origin.y + t * ray.direction.y;
			intersection->point.z = ray.origin.z + t * ray.direction.z;
			intersection->normal = quadNormal;
			return 1;
		}
	}
	intersection->hit = -1;
	return 0;
}

int intersectBoard(ray_t ray, hit_t *intersection)
{
    float dDotN = dot_vec3(ray.direction, (vec3_t){0.0f, 1.0f, 0.0f});

    // Use epsilon for floating-point comparison
    if (fabsf(dDotN) < 1e-6f)
        return 0;

    float t = (0.43f - ray.origin.y) / ray.direction.y;

    // Ignore intersections behind the ray origin
    if (t < 0)
        return 0;

    intersection->point.x = ray.origin.x + t * ray.direction.x;
    intersection->point.y = -5.904f; // Goban height
    intersection->point.z = ray.origin.z + t * ray.direction.z;


    return 1;
}

ray_t createRay(gomo_t *gomo, double xpos, double ypos)
{
    ray_t ray;

    // Convert screen coordinates to normalized device coordinates (NDC)
    float ndcX = (2.0f * (float)xpos) / (float)WIDTH - 1.0f;
    float ndcY = 1.0f - (2.0f * (float)ypos) / (float)HEIGHT;
	vec4_t ray_clip = {ndcX, ndcY, -1.0f, 1.0f};
	float *inv_proj = inv_mat4(gomo->camera->projection);
	float *inv_view = inv_mat4(gomo->camera->view);
	vec4_t ray_eye = mulv_mat4(inv_proj, ray_clip);
	ray_eye = (vec4_t){ray_eye.x, ray_eye.y, -1.0f, 0.0f};
	vec4_t ray_world = mulv_mat4(inv_view, ray_eye);
	ray.direction = norm_vec3((vec3_t){ray_world.x, ray_world.y, ray_world.z});

	free(inv_proj);
	free(inv_view);

    ray.origin = gomo->camera->eye;
	ray.direction = norm_vec3(ray.direction); // Normalize the direction vector

    return ray;
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
	(void)mods;
	if (button == GLFW_MOUSE_BUTTON_RIGHT && (action == GLFW_PRESS || action == GLFW_RELEASE))
	{
		gomo_t *gomo = glfwGetWindowUserPointer(window);
		if (ROTATION)
			return ;
		if (action == GLFW_PRESS && gomo->nb_stones < 361)
		{
			if (gomo->tmp_stone && (!gomo->board[gomo->tmp_stone - 1].state || gomo->board[gomo->tmp_stone - 1].state == 2)) {
				gomo->board[gomo->tmp_stone - 1].state = 1;
				gomo->board[gomo->tmp_stone - 1].color = gomo->nb_stones % 2 ? (vec3_t){0.0f, 0.0f, 0.0f} : (vec3_t){1.0f, 1.0f, 1.0f};
				gomo->tmp_stone = 0;
			}
		}
	} else if (button == GLFW_MOUSE_BUTTON_LEFT && (action == GLFW_PRESS || action == GLFW_RELEASE))
	{
		gomo_t *gomo = glfwGetWindowUserPointer(window);
		if (ROTATION)
			return ;
		if (!(gomo->camera->options >> 3 & 1))
		{
			gomo->camera->options ^= 1 << 2;
			if (action == GLFW_PRESS)
			{
				glfwSetInputMode(gomo->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
				glfwSetCursorPos(gomo->window, gomo->camera->ah / MSPEED, gomo->camera->av / MSPEED);
			}
			else if (action == GLFW_RELEASE)
			{
				glfwSetInputMode(gomo->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}
		}
	}
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
	(void)xoffset;
	if (yoffset)
	{
		gomo_t *gomo = glfwGetWindowUserPointer(window);
		if (!(TOP_VIEW) && !(ANIMATE) && !(ROTATION))
		{
			float zoom = yoffset * (log(gomo->camera->dist + 7.5) - 2);
			if (gomo->camera->dist - zoom >= 1.0f && gomo->camera->dist - zoom <= 3.4f)
				gomo->camera->dist -= zoom;
		}
	}
}

void mouse_move_callback(GLFWwindow *window, double xpos, double ypos)
{
	gomo_t *gomo = glfwGetWindowUserPointer(window);
	ray_t ray;
	hit_t res;
	ray = createRay(gomo, xpos, ypos);
	gomo->obj = gomo->obj->first;
	if (!(ROTATION) && gomo->nb_stones < 361 && intersectBoard(ray, &res))
	{
		gomo->obj = gomo->obj->next;
		int closest_case = find_closest_case(gomo, res.point);
		if (closest_case >= 0 && gomo->board[closest_case].state == 0)
		{
			if (gomo->tmp_stone && gomo->tmp_stone - 1 != closest_case) {
				gomo->board[gomo->tmp_stone - 1].state = 0;
				gomo->board[gomo->tmp_stone - 1].color = (vec3_t){1.0f, 0.0f, 1.0f};
			}
			gomo->tmp_stone = closest_case + 1;
			gomo->board[closest_case].state = 2;
			gomo->board[closest_case].color = gomo->nb_stones % 2 ? (vec3_t){0.5f, 0.5f, 0.5f} : (vec3_t){0.8f, 0.8f, 0.8f};
		} else if (closest_case < 0 && gomo->tmp_stone) {
			gomo->board[gomo->tmp_stone - 1].state = 0;
			gomo->board[gomo->tmp_stone - 1].color = (vec3_t){1.0f, 0.0f, 1.0f};
			gomo->tmp_stone = 0;
		}
		updateData(gomo);
	} else if (ROTATION && intersectText(gomo, ray, &res)) {
		if (res.hit == 3) {
			add_text_to_render(gomo, "font_text2", "Human VS Human", (vec3_t){0.0f, 1.00f, 0.0f}, 0.003f, (vec3_t){1.0f, 1.0f, 1.0f}, 1, 3);
		} else if (res.hit == 4) {
			add_text_to_render(gomo, "font_text2", "Human VS IA", (vec3_t){0.0f, 0.80f, 0.0f}, 0.003f, (vec3_t){1.0f, 1.0f, 1.0f}, 1, 4);
		}
	}
	if (res.hit == -1) {
		if (gomo->text[3].color.x == 1.0f && gomo->text[3].color.y == 1.0f && gomo->text[3].color.z == 1.0f)
			add_text_to_render(gomo, "font_text2", "Human VS Human", (vec3_t){0.0f, 1.00f, 0.0f}, 0.003f, (vec3_t){1.0f, 0.5f, 0.5f}, 1, 3);
		if (gomo->text[4].color.x == 1.0f && gomo->text[4].color.y == 1.0f && gomo->text[4].color.z == 1.0f)
			add_text_to_render(gomo, "font_text2", "Human VS IA", (vec3_t){0.0f, 0.80f, 0.0f}, 0.003f, (vec3_t){1.0f, 0.5f, 0.5f}, 1, 4);
	}
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	(void)mods;
	(void)scancode;
	if (action == GLFW_PRESS)
	{
		gomo_t *gomo = glfwGetWindowUserPointer(window);
		if (key == GLFW_KEY_ESCAPE)
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		else if (key == GLFW_KEY_H)
			gomo->camera->options ^= 1 << 0;
		else if (key == GLFW_KEY_W)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else if (key == GLFW_KEY_F)
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		else if (key == GLFW_KEY_P)
			glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		else if (!(ROTATION) && key == GLFW_KEY_V)
		{
			if (TOP_VIEW && !(ANIMATE)) gomo->camera->options ^= 1 << 1; // animate
			else if (!(TOP_VIEW) && ANIMATE) gomo->camera->options ^= 1 << 1; // animate
			gomo->camera->options ^= 1 << 3; // free view
		}
	}
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
	(void)window;
	glViewport(0, 0, width, height);
}
