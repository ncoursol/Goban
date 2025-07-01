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

void new_vertex(gomo_t *gomo, int f)
{
	data_t *new;
	if (!(new = (data_t *)malloc(sizeof(data_t))))
		exit_callback(gomo, 17, "new data malloc failed");
	gomo->obj->faces[f]->next = new;
	gomo->obj->faces[f]->next->first = gomo->obj->faces[f]->first;
	gomo->obj->faces[f] = gomo->obj->faces[f]->next;
	gomo->obj->faces[f]->vertex = 0;
	gomo->obj->faces[f]->texture = 0;
	gomo->obj->faces[f]->normal = 0;
	gomo->obj->faces[f]->next = NULL;
}

vec3_t create_normal(gomo_t *gomo, float *normals, int k, int i)
{
	vec3_t a, b, norm;
	if (!gomo->obj->faces[i]->normal)
	{
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
	}
	else
	{
		norm = (vec3_t){
			normals[(gomo->obj->faces[i]->normal - 1) * 3],
			normals[(gomo->obj->faces[i]->normal - 1) * 3 + 1],
			normals[(gomo->obj->faces[i]->normal - 1) * 3 + 2]};
	}
	return (norm);
}

void set_texture_uv(gomo_t *gomo, float *textures, int *k, int i)
{
	int j = 0;

	while (gomo->obj->faces[i] != NULL && j < 3)
	{
		// Set UV
		gomo->obj->obj[*k + 0] = textures[(gomo->obj->faces[i]->texture - 1) * 2];
		gomo->obj->obj[*k + 1] = textures[(gomo->obj->faces[i]->texture - 1) * 2 + 1];
		*k += 6;
		if (gomo->obj->faces[i]->next == NULL)
			break;
		gomo->obj->faces[i] = gomo->obj->faces[i]->next;
		j++;
	}
}

void set_texture_xyz(gomo_t *gomo, float *vertices, int *k, int i)
{
	int j = 0;

	while (gomo->obj->faces[i] != NULL && j < 3)
	{
		gomo->obj->obj[*k + 0] = vertices[(gomo->obj->faces[i]->vertex - 1) * 3];
		gomo->obj->obj[*k + 1] = vertices[(gomo->obj->faces[i]->vertex - 1) * 3 + 1];
		gomo->obj->obj[*k + 2] = vertices[(gomo->obj->faces[i]->vertex - 1) * 3 + 2];
		if (gomo->obj->id != 0)
		{
			gomo->obj->obj[*k + 0] += 2.14 * (gomo->obj->id - 1);
			gomo->obj->obj[*k + 1] += 6.35;
			gomo->obj->obj[*k + 2] += 2.14 * (gomo->obj->id - 1);
		}
		*k += 6;
		if (gomo->obj->faces[i]->next == NULL)
			break;
		gomo->obj->faces[i] = gomo->obj->faces[i]->next;
		j++;
	}
}

void set_texture_id(gomo_t *gomo, int i, int *k)
{
	int id = 0;
	material_t *tmp;

	tmp = gomo->obj->materials;

	if (gomo->obj->materials_ids != NULL && gomo->obj->materials->name != NULL) {
		while (gomo->obj->materials_ids[id] != -2 && gomo->obj->materials_ids[id + 1] != -2)
		{
			if (gomo->obj->materials_ids[id] <= i && gomo->obj->materials_ids[id + 1] > i)
				break;
			id++;
		}

		while (tmp && tmp->next && tmp->id != (unsigned int)gomo->obj->materials_ids[id]) {
			tmp = tmp->next;
		}
	}
	id = 0;
	if (tmp->texture) {
		for (int l = 0; l < NB_TEXTURES; l++) {
			if (strstr(textures_path[l].path, tmp->texture) != NULL) {
				id = l;
				break;
			}
		}
	}

	for (int j = 0; j < 3; j++) {
		gomo->obj->obj[*k] = (float)id;
		*k += 6;
	}
}

/* Object Buffer (for 1 triangle):
		00 01 02 03 04 05|06 07 08 09 10 11|12 13 14 15 16 17
		x1 y1 z1 U1 V1 i1|x2 y2 z2 U2 V2 i2|x3 y3 z3 U3 V3 i3
			 Vertex 1    |     Vertex 2    |     Vertex 3
*/
void create_obj(gomo_t *gomo, float *vertices, float *textures, float *normals)
{
	int k = 0;
	vec3_t norm;

	if (!(gomo->obj->obj = (float *)malloc(sizeof(float) * gomo->obj->nb_vertices * 8)))
		exit_callback(gomo, 23, "obj realloc failed");
	for (int i = 0; i < gomo->obj->nb_faces; i++)
	{
		gomo->obj->faces[i] = gomo->obj->faces[i]->first;
		// first set XYZ for each vertices
		set_texture_xyz(gomo, vertices, &k, i);
		// Back to the vertex U1 (see above) k = 18
		k -= 15;

		// create (or set if already define) the triangle normal
		norm = create_normal(gomo, normals, k, i);
		gomo->obj->faces[i] = gomo->obj->faces[i]->first;
		// Set UV for each vertices
		set_texture_uv(gomo, textures, &k, i);
		k -= 16;

		set_texture_id(gomo, i, &k);
		// Set indice k to 24 (0 for the next triangle) k = 27
		k -= 5;
	}
}
