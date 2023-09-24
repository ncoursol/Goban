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

#include "../../include/scop.h"

GLfloat		*new_mat4(void)
{
	size_t	i;
	GLfloat		*mat;

	if (!(mat = (float*)malloc(sizeof(float) * 16)))
		return (NULL);
	i = 0;
	while (i < 16)
	{
		mat[i] = 0.f;
		i++;
	}
	return(mat);
}

GLfloat		*new_mat4_model(void)
{
	GLfloat	*mat;

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
	for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
					ret[4 * j + i] = 0;
					for (int k = 0; k < 4; k++) {
							ret[4 * j + i] += a[4 * j + k] * b[i + k * 4];
					}
			}
  }
	return(ret);
}
