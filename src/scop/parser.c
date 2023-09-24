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

#include "../../include/scop.h"

void	load_face(scop_t *scop, char *line, int *f, int *b_f)
{
	char	*tmp = &line[1];
	int		nb = 0;
	if (*f >= V_BUFF_SIZE * *b_f) {
		*b_f += 1;
		if (!(scop->obj->faces = (data_t**)realloc(scop->obj->faces, sizeof(data_t*) * (V_BUFF_SIZE * *b_f))))
			exit_callback(scop, 16, "faces realloc failed");
		scop->obj->faces_size = V_BUFF_SIZE * *b_f;
		init_face_data(scop, *b_f);
	}
	while (tmp[0] && tmp[0] == ' ') {
		if (scop->obj->faces[*f]->vertex)
			new_vertex(scop, *f);
		if (!scop->obj->faces[*f]->vertex)
			scop->obj->faces[*f]->vertex = strtof(tmp, &tmp);
		if (tmp[0] == '/' && !scop->obj->faces[*f]->texture)
			scop->obj->faces[*f]->texture = strtof(++tmp, &tmp);
		if (tmp[0] == '/' && !scop->obj->faces[*f]->normal)
			scop->obj->faces[*f]->normal = strtof(++tmp, &tmp);
		while(tmp[0] == ' ' && tmp[1] && (tmp[1] < '0' || tmp[1] > '9'))
			tmp = &tmp[1];
		nb++;
	}
	scop->obj->nb_faces++;
	if (nb > 3)
		triangulate(scop, f, b_f, nb);
	else if (nb < 3)
		exit_callback(scop, 19, "faces realloc failed");
	*f += 1;
}

void	load_vertex(scop_t *scop, char *line, size_t read, int *v, int *b_v, int type)
{
	if (count_space(line) != 3 + type)
		exit_callback(scop, 11, "wrong .obj file");
	scop->vertices = float_copy(scop, scop->vertices, read, &line[1 + type], v, b_v);
	scop->nb_vertices++;
	if (*v >= 2) {
		for (int i = 0; i < 3; i++) {
			if (scop->vertices[*v - i - 1] > scop->obj->max[2 - i])
				scop->obj->max[2 - i] = scop->vertices[*v - i - 1];
			if (scop->vertices[*v - i - 1] < scop->obj->min[2 - i])
				scop->obj->min[2 - i] = scop->vertices[*v - i - 1];
		}
	}
}

void	load_new_object(scop_t *scop, char *line, int *f, int *b_f)
{
	if (scop->obj->name == NULL)
		scop->obj->name = string_copy(scop, scop->obj->name, &line[2]);
	else {
		scop->obj->nb_triangles = scop->obj->nb_faces * 3;
		new_obj(scop, &line[2]);
		*f = 0;
		*b_f = 1;
	}
}

void	load_obj(scop_t *scop, char **argv)
{
	FILE 		*fp;
	char 		*line = NULL;
	size_t 	len = 0;
	size_t 	read;
	int			v = 0, vt = 0, vn = 0, f = 0;
	int			b_v = 1, b_vt = 1, b_vn = 1, b_f = 1;

	if(!(fp = fopen(argv[1], "r")))
		exit_callback(scop, 8, "wrong .obj file, fopen failed");

	while ((read = getline(&line, &len, fp)) != (size_t)(-1)) {
		if (line != NULL && line[0] != '#') {
			if (!strncmp(line, "mtllib ", 7) && scop->obj->matName == NULL) {
				scop->obj->matName = string_copy(scop, scop->obj->matName, &line[7]);
			} else if (!strncmp(line, "o ", 2)) {
				load_new_object(scop, line, &f, &b_f);
			} else if (!strncmp(line, "v  ", 3)) {
				load_vertex(scop, line, read, &v, &b_v, 1);
			} else if (!strncmp(line, "v ", 2)) {
				load_vertex(scop, line, read, &v, &b_v, 0);
			} else if (!strncmp(line, "vt ", 3)) {
				scop->textures = float_copy(scop, scop->textures, read, &line[2], &vt, &b_vt);
				scop->obj->texCoord++;
				scop->nb_textures++;
			} else if (!strncmp(line, "vn ", 3)) {
				scop->normals = float_copy(scop, scop->normals, read, &line[2], &vn, &b_vn);
				scop->nb_normals++;
			} else if (scop->obj->matDef == NULL && !strncmp(line, "usemtl ", 7)) {
				scop->obj->matDef = string_copy(scop, scop->obj->matDef, &line[7]);
			} else if (!scop->obj->smooth && !strncmp(line, "s on", 4)) {
				scop->obj->smooth = 1;
			} else if (!strncmp(line, "f ", 2))
				load_face(scop, line, &f, &b_f);
		}
		free(line);
		line = NULL;
	}
	free_null((void*)line);
	fclose(fp);
	if (v)
		if (!(scop->vertices = (float*)realloc(scop->vertices, sizeof(float) * v)))
			exit_callback(scop, 20, "vertex realloc failed");
	if (vt)
		if (!(scop->textures = (float*)realloc(scop->textures, sizeof(float) * vt)))
			exit_callback(scop, 21, "textures realloc failed");
	if (vn)
		if (!(scop->normals = (float*)realloc(scop->normals, sizeof(float) * vn)))
			exit_callback(scop, 22, "normals realloc failed");
	scop->obj->nb_triangles = scop->obj->nb_faces * 3;
	create_obj(scop);
}
