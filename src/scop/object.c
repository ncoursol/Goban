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

#include "../../include/scop.h"

void	new_vertex(scop_t *scop, int f) {
	data_t *new;
	if (!(new = (data_t*)malloc(sizeof(data_t))))
		exit_callback(scop, 17, "new data malloc failed");
	scop->obj->faces[f]->next = new;
	scop->obj->faces[f]->next->first = scop->obj->faces[f]->first;
	scop->obj->faces[f] = scop->obj->faces[f]->next;
	scop->obj->faces[f]->vertex = 0;
	scop->obj->faces[f]->texture = 0;
	scop->obj->faces[f]->normal = 0;
	scop->obj->faces[f]->next = NULL;
}

void	new_obj(scop_t *scop, char *line) {
	obj_t *new;
	obj_t *first;

	first = scop->obj->first;
	if (!(new = (obj_t*)malloc(sizeof(obj_t))))
		exit_callback(scop, 10, "new object malloc failed");
	new->id = scop->obj->id + 1;
	scop->obj->next = new;
	scop->obj = scop->obj->next;
	init_obj(scop);
	scop->obj->name = string_copy(scop, scop->obj->name, line);
	scop->obj->first = first;
}

vec3_t	create_normal(scop_t *scop, int k, int i, vec3_t norm) {
	vec3_t	a, b;
	if (!scop->obj->faces[i]->normal) {
		a = (vec3_t){
			scop->obj->obj[k + 5] - scop->obj->obj[k - 3],
			scop->obj->obj[k + 6] - scop->obj->obj[k - 2],
			scop->obj->obj[k + 7] - scop->obj->obj[k - 1],
		};
		b = (vec3_t){
			scop->obj->obj[k + 13] - scop->obj->obj[k - 3],
			scop->obj->obj[k + 14] - scop->obj->obj[k - 2],
			scop->obj->obj[k + 15] - scop->obj->obj[k - 1],
		};
		norm = cross_vec3(a, b);
	} else {
		norm = (vec3_t){
			scop->normals[(scop->obj->faces[i]->normal - 1) * 3],
			scop->normals[(scop->obj->faces[i]->normal - 1) * 3 + 1],
			scop->normals[(scop->obj->faces[i]->normal - 1) * 3 + 2]
		};
	}
	return (norm);
}

void	set_texture_uvrgb(scop_t *scop, int *k, int i, vec3_t norm)
{
	int j = 0;
	vec3_t 	tmp;
	float		sc;

	while(scop->obj->faces[i] != NULL && j < 3) {
		// Set UV
		if (scop->obj->texCoord) {
			scop->obj->obj[*k + 0] = scop->textures[(scop->obj->faces[i]->texture - 1) * 2];
			scop->obj->obj[*k + 1] = scop->textures[(scop->obj->faces[i]->texture - 1) * 2 + 1];
		} else {
			tmp.x = sqrt(powf(scop->obj->max[0] - scop->obj->min[0], 2)) / 2;
			tmp.y = sqrt(powf(scop->obj->max[1] - scop->obj->min[1], 2)) / 2;
			tmp.z = sqrt(powf(scop->obj->max[2] - scop->obj->min[2], 2)) / 2;
			if (ABS(norm.x) > ABS(norm.z) && ABS(norm.x) > ABS(norm.z)) {
				sc = tmp.x > tmp.y ? tmp.x : tmp.y;
				scop->obj->obj[*k + 0] = scop->vertices[(scop->obj->faces[i]->vertex - 1) * 3 + 1] / sc;
				scop->obj->obj[*k + 1] = scop->vertices[(scop->obj->faces[i]->vertex - 1) * 3 + 2] / sc;
			} else if (ABS(norm.y) > ABS(norm.x) && ABS(norm.y) > ABS(norm.z)) {
				sc = tmp.y > tmp.z ? tmp.y : tmp.z;
				scop->obj->obj[*k + 0] = scop->vertices[(scop->obj->faces[i]->vertex - 1) * 3] / sc;
				scop->obj->obj[*k + 1] = scop->vertices[(scop->obj->faces[i]->vertex - 1) * 3 + 2] / sc;
			} else {
				sc = tmp.x > tmp.z ? tmp.x : tmp.z;
				scop->obj->obj[*k + 0] = scop->vertices[(scop->obj->faces[i]->vertex - 1) * 3] / sc;
				scop->obj->obj[*k + 1] = scop->vertices[(scop->obj->faces[i]->vertex - 1) * 3 + 1] / sc;
			}
		}
		*k += 2;
		// Set RGB
		scop->obj->obj[*k + 0] = (float)i / (float)scop->obj->nb_faces;
		scop->obj->obj[*k + 1] = (float)i / (float)scop->obj->nb_faces;
		scop->obj->obj[*k + 2] = (float)i / (float)scop->obj->nb_faces;
		*k += 6;
		if (scop->obj->faces[i]->next == NULL)
			break;
		scop->obj->faces[i] = scop->obj->faces[i]->next;
		j++;
	}
}

void	set_texture_xyz(scop_t *scop, int *k, int i)
{
	int j = 0;

	while(scop->obj->faces[i] != NULL && j < 3) {
		scop->obj->obj[*k + 0] = scop->vertices[(scop->obj->faces[i]->vertex - 1) * 3];
		scop->obj->obj[*k + 1] = scop->vertices[(scop->obj->faces[i]->vertex - 1) * 3 + 1];
		scop->obj->obj[*k + 2] = scop->vertices[(scop->obj->faces[i]->vertex - 1) * 3 + 2];
		*k += 8;
		if (scop->obj->faces[i]->next == NULL)
			break;
		scop->obj->faces[i] = scop->obj->faces[i]->next;
		j++;
	}
}

/* Object Buffer (for 1 triangle):
		00 01 02 03 04 05 06 07|08 09 10 11 12 13 14 15|16 17 18 19 20 21 22 23
		x1 y1 z1 U1 V1 r1 g1 b1|x2 y2 z2 U2 V2 r2 g2 b2|x3 y3 z3 U3 V3 r3 g3 b3
		        Vertex 1       |        Vertex 2       |        Vertex 3
*/
void	create_obj(scop_t *scop)
{
	int			k;
	vec3_t  norm;

	scop->obj = scop->obj->first;
	while (scop->obj != NULL) {
		if (!(scop->obj->obj = (float*)malloc(sizeof(float) * scop->obj->nb_triangles * 8)))
		exit_callback(scop, 23, "obj realloc failed");
		k = 0;
		for (int i = 0; i < scop->obj->nb_faces; i++) {
			scop->obj->faces[i] = scop->obj->faces[i]->first;
			// first set XYZ for each vertices
			set_texture_xyz(scop, &k, i);
			// Back to the vertex U1 (see above) k = 24
			k -= 21;
			// create (or set if already define) the triangle normal
			norm = create_normal(scop, k, i, norm);
			scop->obj->faces[i] = scop->obj->faces[i]->first;
			// Set UV and RGB for each vertices
			set_texture_uvrgb(scop, &k, i, norm);
			// Set indice k to 24 (0 for the next triangle) k = 27
			k -= 3;
		}
		if (scop->obj->next == NULL)
			break;
		scop->obj = scop->obj->next;
	}
}
