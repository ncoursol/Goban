/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   vec3.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncoursol <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/04/04 16:59:08 by ncoursol          #+#    #+#             */
/*   Updated: 2022/04/17 18:00:44 by ncoursol         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/scop.h"

float		dist_btw_two_vec3(vec3_t a, vec3_t b)
{
	return (sqrtf(powf(a.x - b.x, 2) + powf(a.y - b.y, 2) + powf(a.z - b.z, 2)));
}

vec3_t	prod_vec3(vec3_t a, float b)
{
	vec3_t ret;

	ret.x = a.x * b;
	ret.y = a.y * b;
	ret.z = a.z * b;
	return (ret);
}

vec3_t	add_vec3(vec3_t a, vec3_t b)
{
	vec3_t ret;

	ret.x = a.x + b.x;
	ret.y = a.y + b.y;
	ret.z = a.z + b.z;
	return (ret);
}

vec3_t	sub_vec3(vec3_t a, vec3_t b)
{
	vec3_t ret;

	ret.x = a.x - b.x;
	ret.y = a.y - b.y;
	ret.z = a.z - b.z;
	return (ret);
}

vec3_t	norm_vec3(vec3_t a)
{
	float w;

	w = sqrtf(a.x * a.x + a.y * a.y + a.z * a.z);
	a.x /= w;
	a.y /= w;
	a.z /= w;
	return (a);
}

vec3_t	cross_vec3(vec3_t a, vec3_t b)
{
	vec3_t ret;

	ret.x = a.y * b.z - b.y * a.z;
	ret.y = b.x * a.z - a.x * b.z;
	ret.z = a.x * b.y - b.x * a.y;
	return (ret);
}

GLfloat	dot_vec3(vec3_t a, vec3_t b)
{
  return (a.x * b.x + a.y * b.y + a.z * b.z);
}
