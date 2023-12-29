/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   object.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncoursol <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/04/14 13:55:17 by ncoursol          #+#    #+#             */
/*   Updated: 2022/04/14 13:55:58 by ncoursol         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/gomo.h"

void	new_vertex(gomo_t *gomo, int f) {
	data_t *new;
	if (!(new = (data_t*)malloc(sizeof(data_t))))
		exit_callback(gomo, 17, "new data malloc failed");
	gomo->obj->faces[f]->next = new;
	gomo->obj->faces[f]->next->first = gomo->obj->faces[f]->first;
	gomo->obj->faces[f] = gomo->obj->faces[f]->next;
	gomo->obj->faces[f]->vertex = 0;
	gomo->obj->faces[f]->texture = 0;
	gomo->obj->faces[f]->normal = 0;
	gomo->obj->faces[f]->next = NULL;
}

void	new_obj(gomo_t *gomo, char *line) {
	obj_t *new;
	obj_t *first;

	first = gomo->obj->first;
	if (!(new = (obj_t*)malloc(sizeof(obj_t))))
		exit_callback(gomo, 10, "new object malloc failed");
	new->id = gomo->obj->id + 1;
	gomo->obj->next = new;
	gomo->obj = gomo->obj->next;
	init_obj(gomo);
	gomo->obj->name = string_copy(gomo, gomo->obj->name, line);
	gomo->obj->first = first;
}

vec3_t	create_normal(gomo_t *gomo, int k, int i, vec3_t norm) {
	vec3_t	a, b;
	if (!gomo->obj->faces[i]->normal) {
		a = (vec3_t){
			gomo->obj->obj[k + 5] - gomo->obj->obj[k - 3],
			gomo->obj->obj[k + 6] - gomo->obj->obj[k - 2],
			gomo->obj->obj[k + 7] - gomo->obj->obj[k - 1],
		};
		b = (vec3_t){
			gomo->obj->obj[k + 13] - gomo->obj->obj[k - 3],
			gomo->obj->obj[k + 14] - gomo->obj->obj[k - 2],
			gomo->obj->obj[k + 15] - gomo->obj->obj[k - 1],
		};
		norm = cross_vec3(a, b);
	} else {
		norm = (vec3_t){
			gomo->normals[(gomo->obj->faces[i]->normal - 1) * 3],
			gomo->normals[(gomo->obj->faces[i]->normal - 1) * 3 + 1],
			gomo->normals[(gomo->obj->faces[i]->normal - 1) * 3 + 2]
		};
	}
	return (norm);
}

void	set_texture_uvrgb(gomo_t *gomo, int *k, int i, vec3_t norm)
{
	int j = 0;
	vec3_t 	tmp;
	float		sc;

	while(gomo->obj->faces[i] != NULL && j < 3) {
		// Set UV
		if (gomo->obj->texCoord) {
			gomo->obj->obj[*k + 0] = gomo->textures[(gomo->obj->faces[i]->texture - 1) * 2];
			gomo->obj->obj[*k + 1] = gomo->textures[(gomo->obj->faces[i]->texture - 1) * 2 + 1];
		} else {
			tmp.x = sqrt(powf(gomo->obj->max[0] - gomo->obj->min[0], 2)) / 2;
			tmp.y = sqrt(powf(gomo->obj->max[1] - gomo->obj->min[1], 2)) / 2;
			tmp.z = sqrt(powf(gomo->obj->max[2] - gomo->obj->min[2], 2)) / 2;
			if (ABS(norm.x) > ABS(norm.z) && ABS(norm.x) > ABS(norm.z)) {
				sc = tmp.x > tmp.y ? tmp.x : tmp.y;
				gomo->obj->obj[*k + 0] = gomo->vertices[(gomo->obj->faces[i]->vertex - 1) * 3 + 1] / sc;
				gomo->obj->obj[*k + 1] = gomo->vertices[(gomo->obj->faces[i]->vertex - 1) * 3 + 2] / sc;
			} else if (ABS(norm.y) > ABS(norm.x) && ABS(norm.y) > ABS(norm.z)) {
				sc = tmp.y > tmp.z ? tmp.y : tmp.z;
				gomo->obj->obj[*k + 0] = gomo->vertices[(gomo->obj->faces[i]->vertex - 1) * 3] / sc;
				gomo->obj->obj[*k + 1] = gomo->vertices[(gomo->obj->faces[i]->vertex - 1) * 3 + 2] / sc;
			} else {
				sc = tmp.x > tmp.z ? tmp.x : tmp.z;
				gomo->obj->obj[*k + 0] = gomo->vertices[(gomo->obj->faces[i]->vertex - 1) * 3] / sc;
				gomo->obj->obj[*k + 1] = gomo->vertices[(gomo->obj->faces[i]->vertex - 1) * 3 + 1] / sc;
			}
		}
		*k += 2;
		// Set RGB
		gomo->obj->obj[*k + 0] = (float)i / (float)gomo->obj->nb_faces;
		gomo->obj->obj[*k + 1] = (float)i / (float)gomo->obj->nb_faces;
		gomo->obj->obj[*k + 2] = (float)i / (float)gomo->obj->nb_faces;
		*k += 6;
		if (gomo->obj->faces[i]->next == NULL)
			break;
		gomo->obj->faces[i] = gomo->obj->faces[i]->next;
		j++;
	}
}

void	set_texture_xyz(gomo_t *gomo, int *k, int i)
{
	int j = 0;

	while(gomo->obj->faces[i] != NULL && j < 3) {
		gomo->obj->obj[*k + 0] = gomo->vertices[(gomo->obj->faces[i]->vertex - 1) * 3];
		gomo->obj->obj[*k + 1] = gomo->vertices[(gomo->obj->faces[i]->vertex - 1) * 3 + 1];
		gomo->obj->obj[*k + 2] = gomo->vertices[(gomo->obj->faces[i]->vertex - 1) * 3 + 2];
		if (gomo->obj->id != 0) {
			gomo->obj->obj[*k + 0] += 2.14 * (gomo->obj->id - 1);
			gomo->obj->obj[*k + 1] += 6.35;
			gomo->obj->obj[*k + 2] += 2.14 * (gomo->obj->id - 1);
		}
		*k += 8;
		if (gomo->obj->faces[i]->next == NULL)
			break;
		gomo->obj->faces[i] = gomo->obj->faces[i]->next;
		j++;
	}
}

/* Object Buffer (for 1 triangle):
		00 01 02 03 04 05 06 07|08 09 10 11 12 13 14 15|16 17 18 19 20 21 22 23
		x1 y1 z1 U1 V1 r1 g1 b1|x2 y2 z2 U2 V2 r2 g2 b2|x3 y3 z3 U3 V3 r3 g3 b3
		        Vertex 1       |        Vertex 2       |        Vertex 3
*/
void	create_obj(gomo_t *gomo)
{
	int			k;
	vec3_t  norm;

	gomo->obj = gomo->obj->first;
	while (gomo->obj != NULL) {
		if (!(gomo->obj->obj = (float*)malloc(sizeof(float) * gomo->obj->nb_triangles * 8)))
		exit_callback(gomo, 23, "obj realloc failed");
		k = 0;
		for (int i = 0; i < gomo->obj->nb_faces; i++) {
			gomo->obj->faces[i] = gomo->obj->faces[i]->first;
			// first set XYZ for each vertices
			set_texture_xyz(gomo, &k, i);
			// Back to the vertex U1 (see above) k = 24
			k -= 21;
			// create (or set if already define) the triangle normal
			norm = create_normal(gomo, k, i, norm);
			gomo->obj->faces[i] = gomo->obj->faces[i]->first;
			// Set UV and RGB for each vertices
			set_texture_uvrgb(gomo, &k, i, norm);
			// Set indice k to 24 (0 for the next triangle) k = 27
			k -= 3;
		}
		if (gomo->obj->next == NULL)
			break;
		gomo->obj = gomo->obj->next;
	}
}
