/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mat4.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncoursol <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/04/14 13:55:17 by ncoursol          #+#    #+#             */
/*   Updated: 2022/04/14 13:55:58 by ncoursol         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/gomo.h"

GLfloat *new_mat4(void)
{
	size_t i;
	GLfloat *mat;

	if (!(mat = (float *)malloc(sizeof(float) * 16)))
		return (NULL);
	i = 0;
	while (i < 16)
	{
		mat[i] = 0.f;
		i++;
	}
	return (mat);
}

GLfloat *new_mat4_model(void)
{
	GLfloat *mat;

	if (!(mat = new_mat4()))
		return (NULL);
	mat[0] = 1.0f;
	mat[5] = 1.0f;
	mat[10] = 1.0f;
	mat[15] = 1.0f;
	return (mat);
}

GLfloat *prod_mat4(float *a, float *b)
{
	float *ret;
	ret = new_mat4();
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			ret[4 * j + i] = 0;
			for (int k = 0; k < 4; k++)
			{
				ret[4 * j + i] += a[4 * j + k] * b[i + k * 4];
			}
		}
	}
	return (ret);
}

vec4_t mulv_mat4(float *m, vec4_t v)
{
	vec4_t result;

	result.x = m[0] * v.x + m[4] * v.y + m[8] * v.z + m[12] * v.w;
	result.y = m[1] * v.x + m[5] * v.y + m[9] * v.z + m[13] * v.w;
	result.z = m[2] * v.x + m[6] * v.y + m[10] * v.z + m[14] * v.w;
	result.w = m[3] * v.x + m[7] * v.y + m[11] * v.z + m[15] * v.w;
	return result;
}

vec4_t mult_mat4_vec4(float *m, vec4_t v)
{
	vec4_t result;

	result.x = m[0] * v.x + m[1] * v.y + m[2] * v.z + m[3] * v.w;
	result.y = m[4] * v.x + m[5] * v.y + m[6] * v.z + m[7] * v.w;
	result.z = m[8] * v.x + m[9] * v.y + m[10] * v.z + m[11] * v.w;
	result.w = m[12] * v.x + m[13] * v.y + m[14] * v.z + m[15] * v.w;
	return result;
}

vec3_t mult_mat4_vec3(float *m, vec3_t v)
{
	vec3_t result;

	result.x = m[0] * v.x + m[1] * v.y + m[2] * v.z + m[3];
	result.y = m[4] * v.x + m[5] * v.y + m[6] * v.z + m[7];
	result.z = m[8] * v.x + m[9] * v.y + m[10] * v.z + m[11];
	return result;
}

void print_mat4(float *m)
{
	printf("Matrix:\n");
	printf("%f %f %f %f\n", m[0], m[1], m[2], m[3]);
	printf("%f %f %f %f\n", m[4], m[5], m[6], m[7]);
	printf("%f %f %f %f\n", m[8], m[9], m[10], m[11]);
	printf("%f %f %f %f\n", m[12], m[13], m[14], m[15]);
}

float *inv_mat4(float *m)
{
	float tmp[12]; // temp array for pairs
	float src[16]; // array of transpose source matrix
	float det;	   // determinant
	float *result; // destination matrix

	result = new_mat4();

	// transpose matrix
	for (int i = 0; i < 4; i++)
	{
		src[i] = m[i * 4];
		src[i + 4] = m[i * 4 + 1];
		src[i + 8] = m[i * 4 + 2];
		src[i + 12] = m[i * 4 + 3];
	}

	// calculate pairs for first 8 elements (cofactors)
	tmp[0] = src[10] * src[15];
	tmp[1] = src[11] * src[14];
	tmp[2] = src[9] * src[15];
	tmp[3] = src[11] * src[13];
	tmp[4] = src[9] * src[14];
	tmp[5] = src[10] * src[13];
	tmp[6] = src[8] * src[15];
	tmp[7] = src[11] * src[12];
	tmp[8] = src[8] * src[14];
	tmp[9] = src[10] * src[12];
	tmp[10] = src[8] * src[13];
	tmp[11] = src[9] * src[12];

	// calculate first 8 elements (cofactors)
	result[0] = tmp[0] * src[5] + tmp[3] * src[6] + tmp[4] * src[7];
	result[0] -= tmp[1] * src[5] + tmp[2] * src[6] + tmp[5] * src[7];
	result[1] = tmp[1] * src[4] + tmp[6] * src[6] + tmp[9] * src[7];
	result[1] -= tmp[0] * src[4] + tmp[7] * src[6] + tmp[8] * src[7];
	result[2] = tmp[2] * src[4] + tmp[7] * src[5] + tmp[10] * src[7];
	result[2] -= tmp[3] * src[4] + tmp[6] * src[5] + tmp[11] * src[7];
	result[3] = tmp[5] * src[4] + tmp[8] * src[5] + tmp[11] * src[6];
	result[3] -= tmp[4] * src[4] + tmp[9] * src[5] + tmp[10] * src[6];
	result[4] = tmp[1] * src[1] + tmp[2] * src[2] + tmp[5] * src[3];
	result[4] -= tmp[0] * src[1] + tmp[3] * src[2] + tmp[4] * src[3];
	result[5] = tmp[0] * src[0] + tmp[7] * src[2] + tmp[8] * src[3];
	result[5] -= tmp[1] * src[0] + tmp[6] * src[2] + tmp[9] * src[3];
	result[6] = tmp[3] * src[0] + tmp[6] * src[1] + tmp[11] * src[3];
	result[6] -= tmp[2] * src[0] + tmp[7] * src[1] + tmp[10] * src[3];
	result[7] = tmp[4] * src[0] + tmp[9] * src[1] + tmp[10] * src[2];
	result[7] -= tmp[5] * src[0] + tmp[8] * src[1] + tmp[11] * src[2];

	// calculate pairs for second 8 elements (cofactors)
	tmp[0] = src[2] * src[7];
	tmp[1] = src[3] * src[6];
	tmp[2] = src[1] * src[7];
	tmp[3] = src[3] * src[5];
	tmp[4] = src[1] * src[6];
	tmp[5] = src[2] * src[5];
	tmp[6] = src[0] * src[7];
	tmp[7] = src[3] * src[4];
	tmp[8] = src[0] * src[6];
	tmp[9] = src[2] * src[4];
	tmp[10] = src[0] * src[5];
	tmp[11] = src[1] * src[4];

	// calculate second 8 elements (cofactors)
	result[8] = tmp[0] * src[13] + tmp[3] * src[14] + tmp[4] * src[15];
	result[8] -= tmp[1] * src[13] + tmp[2] * src[14] + tmp[5] * src[15];
	result[9] = tmp[1] * src[12] + tmp[6] * src[14] + tmp[9] * src[15];
	result[9] -= tmp[0] * src[12] + tmp[7] * src[14] + tmp[8] * src[15];
	result[10] = tmp[2] * src[12] + tmp[7] * src[13] + tmp[10] * src[15];
	result[10] -= tmp[3] * src[12] + tmp[6] * src[13] + tmp[11] * src[15];
	result[11] = tmp[5] * src[12] + tmp[8] * src[13] + tmp[11] * src[14];
	result[11] -= tmp[4] * src[12] + tmp[9] * src[13] + tmp[10] * src[14];
	result[12] = tmp[2] * src[10] + tmp[5] * src[11] + tmp[1] * src[9];
	result[12] -= tmp[4] * src[11] + tmp[0] * src[9] + tmp[3] * src[10];
	result[13] = tmp[8] * src[11] + tmp[0] * src[8] + tmp[7] * src[10];
	result[13] -= tmp[6] * src[10] + tmp[9] * src[11] + tmp[1] * src[8];
	result[14] = tmp[6] * src[9] + tmp[11] * src[11] + tmp[3] * src[8];
	result[14] -= tmp[10] * src[11] + tmp[2] * src[8] + tmp[7] * src[9];
	result[15] = tmp[10] * src[10] + tmp[4] * src[8] + tmp[9] * src[9];
	result[15] -= tmp[8] * src[9] + tmp[11] * src[10] + tmp[5] * src[8];

	// calculate determinant
	det = src[0] * result[0] + src[1] * result[1] + src[2] * result[2] + src[3] * result[3];

	// calculate matrix inverse
	det = 1.0f / det;
	for (int i = 0; i < 16; i++)
	{
		result[i] *= det;
	}

	return result;
}