/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncoursol <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/04/14 13:55:17 by ncoursol          #+#    #+#             */
/*   Updated: 2023/01/27 22:43:39 by ncoursol         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/gomo.h"

void	load_face(gomo_t *gomo, char *line, int *f, int *b_f)
{
	char	*tmp = &line[1];
	int		nb = 0;
	if (*f >= V_BUFF_SIZE * *b_f) {
		*b_f += 1;
		if (!(gomo->obj->faces = (data_t**)realloc(gomo->obj->faces, sizeof(data_t*) * (V_BUFF_SIZE * *b_f))))
			exit_callback(gomo, 16, "faces realloc failed");
		gomo->obj->faces_size = V_BUFF_SIZE * *b_f;
		init_face_data(gomo, *b_f);
	}
	while (tmp[0] && tmp[0] == ' ') {
		if (gomo->obj->faces[*f]->vertex)
			new_vertex(gomo, *f);
		if (!gomo->obj->faces[*f]->vertex)
			gomo->obj->faces[*f]->vertex = strtof(tmp, &tmp) + gomo->tmp_id.x;
		if (tmp[0] == '/' && !gomo->obj->faces[*f]->texture)
			gomo->obj->faces[*f]->texture = strtof(++tmp, &tmp) + gomo->tmp_id.y;
		if (tmp[0] == '/' && !gomo->obj->faces[*f]->normal)
			gomo->obj->faces[*f]->normal = strtof(++tmp, &tmp) + gomo->tmp_id.z;
		while(tmp[0] == ' ' && tmp[1] && (tmp[1] < '0' || tmp[1] > '9'))
			tmp = &tmp[1];
		nb++;
	}
	gomo->obj->nb_faces++;
	if (nb > 3)
		triangulate(gomo, f, b_f, nb);
	else if (nb < 3)
		exit_callback(gomo, 19, "faces realloc failed");
	*f += 1;
}

void	load_vertex(gomo_t *gomo, char *line, size_t read, int *v, int *b_v, int type)
{
	if (count_space(line) != 3 + type)
		exit_callback(gomo, 11, "wrong .obj file");
	gomo->vertices = float_copy(gomo, gomo->vertices, read, &line[1 + type], v, b_v);
	gomo->nb_vertices++;
	if (*v >= 2) {
		for (int i = 0; i < 3; i++) {
			if (gomo->vertices[*v - i - 1] > gomo->obj->max[2 - i])
				gomo->obj->max[2 - i] = gomo->vertices[*v - i - 1];
			if (gomo->vertices[*v - i - 1] < gomo->obj->min[2 - i])
				gomo->obj->min[2 - i] = gomo->vertices[*v - i - 1];
		}
	}
}

void	load_new_object(gomo_t *gomo, char *line, int *f, int *b_f)
{
	if (gomo->obj->name == NULL)
		gomo->obj->name = string_copy(gomo, gomo->obj->name, &line[2]);
	else {
		gomo->obj->nb_triangles = gomo->obj->nb_faces * 3;
		new_obj(gomo, &line[2]);
		*f = 0;
		*b_f = 1;
	}
}

void	load_obj(gomo_t *gomo, char *argv)
{
	FILE 		*fp;
	char 		*line = NULL;
	size_t 	len = 0;
	size_t 	read;
	int			v = gomo->nb_vertices * 3, vt = gomo->nb_textures * 2, vn = gomo->nb_normals * 3, f = 0;
	int			b_v = 1, b_vt = 1, b_vn = 1, b_f = 1;

	if(!(fp = fopen(argv, "r")))
		exit_callback(gomo, 8, "wrong .obj file, fopen failed");

	if (gomo->nb_vertices || gomo->nb_textures || gomo->nb_normals)
	{
		if (!(gomo->vertices = (float*)realloc(gomo->vertices, sizeof(float) * (v + V_BUFF_SIZE))))
			exit_callback(gomo, 1, "vertices malloc failed");
		if (!(gomo->textures = (float*)realloc(gomo->textures, sizeof(float) * (vt + V_BUFF_SIZE))))
			exit_callback(gomo, 2, "textures malloc failed");
		if (!(gomo->normals = (float*)realloc(gomo->normals, sizeof(float) * (vn + V_BUFF_SIZE))))
			exit_callback(gomo, 3, "normals malloc failed");
	}

	while ((read = getline(&line, &len, fp)) != (size_t)(-1)) {
		if (line != NULL && line[0] != '#') {
			if (!strncmp(line, "mtllib ", 7) && gomo->obj->matName == NULL) {
				gomo->obj->matName = string_copy(gomo, gomo->obj->matName, &line[7]);
			} else if (!strncmp(line, "o ", 2)) {
				load_new_object(gomo, line, &f, &b_f);
			} else if (!strncmp(line, "v  ", 3)) {
				load_vertex(gomo, line, read, &v, &b_v, 1);
			} else if (!strncmp(line, "v ", 2)) {
				load_vertex(gomo, line, read, &v, &b_v, 0);
			} else if (!strncmp(line, "vt ", 3)) {
				gomo->textures = float_copy(gomo, gomo->textures, read, &line[2], &vt, &b_vt);
				gomo->obj->texCoord++;
				gomo->nb_textures++;
			} else if (!strncmp(line, "vn ", 3)) {
				gomo->normals = float_copy(gomo, gomo->normals, read, &line[2], &vn, &b_vn);
				gomo->nb_normals++;
			} else if (gomo->obj->matDef == NULL && !strncmp(line, "usemtl ", 7)) {
				gomo->obj->matDef = string_copy(gomo, gomo->obj->matDef, &line[7]);
			} else if (!gomo->obj->smooth && !strncmp(line, "s on", 4)) {
				gomo->obj->smooth = 1;
			} else if (!strncmp(line, "f ", 2))
				load_face(gomo, line, &f, &b_f);
		}
		free(line);
		line = NULL;
	}
	free_null((void*)line);
	fclose(fp);
	if (v)
		if (!(gomo->vertices = (float*)realloc(gomo->vertices, sizeof(float) * v)))
			exit_callback(gomo, 20, "vertex realloc failed");
	if (vt)
		if (!(gomo->textures = (float*)realloc(gomo->textures, sizeof(float) * vt)))
			exit_callback(gomo, 21, "textures realloc failed");
	if (vn)
		if (!(gomo->normals = (float*)realloc(gomo->normals, sizeof(float) * vn)))
			exit_callback(gomo, 22, "normals realloc failed");
	gomo->tmp_id.x = gomo->nb_vertices;
	gomo->tmp_id.y = gomo->nb_textures;
	gomo->tmp_id.z = gomo->nb_normals;
	gomo->obj->nb_triangles = gomo->obj->nb_faces * 3;
	create_obj(gomo);
}
