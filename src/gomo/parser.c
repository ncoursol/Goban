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

void load_face(gomo_t *gomo, char *line, int nb_faces, int *b_f)
{
	char *tmp = &line[1];
	if (nb_faces >= V_BUFF_SIZE * *b_f)
	{
		*b_f += 1;
		if (!(gomo->obj->faces = (data_t **)realloc(gomo->obj->faces, sizeof(data_t *) * (V_BUFF_SIZE * *b_f))))
			exit_callback(gomo, 16, "faces realloc failed");
		gomo->obj->faces_size = V_BUFF_SIZE * *b_f;
		init_face_data(gomo, *b_f);
	}
	while (tmp[0] && tmp[0] == ' ')
	{
		if (gomo->obj->faces[nb_faces]->vertex)
			new_vertex(gomo, nb_faces);
		if (!gomo->obj->faces[nb_faces]->vertex)
			gomo->obj->faces[nb_faces]->vertex = strtof(tmp, &tmp);
		if (tmp[0] == '/' && !gomo->obj->faces[nb_faces]->texture)
			gomo->obj->faces[nb_faces]->texture = strtof(++tmp, &tmp);
		if (tmp[0] == '/' && !gomo->obj->faces[nb_faces]->normal)
			gomo->obj->faces[nb_faces]->normal = strtof(++tmp, &tmp);
		while (tmp[0] == ' ' && tmp[1] && (tmp[1] < '0' || tmp[1] > '9'))
			tmp = &tmp[1];
	}
}

void load_vertex(gomo_t *gomo, float *vertices, char *line, size_t read, int *nb_vertices, int *b_v)
{
	if (count_space(line) != 3)
		exit_callback(gomo, 11, "wrong .obj file");
	vertices = float_copy(gomo, vertices, read, &line[1], nb_vertices, b_v);
	if (*nb_vertices >= 2)
	{
		for (int i = 0; i < 3; i++)
		{
			if (vertices[*nb_vertices - i - 1] > gomo->obj->max[2 - i])
				gomo->obj->max[2 - i] = vertices[*nb_vertices - i - 1];
			if (vertices[*nb_vertices - i - 1] < gomo->obj->min[2 - i])
				gomo->obj->min[2 - i] = vertices[*nb_vertices - i - 1];
		}
	}
}

void load_obj(gomo_t *gomo, char *argv)
{
	FILE *fp;
	char *line = NULL;
	size_t len = 0;
	size_t read;
	int nb_vertices = 0, nb_textures = 0, nb_normals = 0, nb_faces = 0;
	int b_v = 1, b_vt = 1, b_vn = 1, b_f = 1; // b_v = buff_vertices, b_vt = buff_textures, b_vn = buff_normals, b_f = buff_faces
	float *vertices, *textures, *normals;

	if (!(fp = fopen(argv, "r")))
		exit_callback(gomo, 8, "wrong .obj file, fopen failed");

	if (!(vertices = (float *)malloc(sizeof(float) * V_BUFF_SIZE)))
		exit_callback(gomo, 1, "vertices malloc failed");
	if (!(textures = (float *)malloc(sizeof(float) * V_BUFF_SIZE)))
		exit_callback(gomo, 2, "textures malloc failed");
	if (!(normals = (float *)malloc(sizeof(float) * V_BUFF_SIZE)))
		exit_callback(gomo, 3, "normals malloc failed");

	while ((read = getline(&line, &len, fp)) != (size_t)(-1))
	{
		if (line != NULL && line[0] != '#')
		{
			if (!strncmp(line, "v ", 2))
			{
				load_vertex(gomo, vertices, line, read, &nb_vertices, &b_v);
			}
			else if (!strncmp(line, "vt ", 3))
			{
				textures = float_copy(gomo, textures, read, &line[2], &nb_textures, &b_vt);
				gomo->obj->texCoord++;
			}
			else if (!strncmp(line, "vn ", 3))
			{
				normals = float_copy(gomo, normals, read, &line[2], &nb_normals, &b_vn);
			}
			else if (!strncmp(line, "f ", 2))
			{
				load_face(gomo, line, nb_faces, &b_f);
				nb_faces++;
			}
		}
		free(line);
		line = NULL;
	}
	free_null((void *)line);
	fclose(fp);
	if (nb_vertices)
		if (!(vertices = (float *)realloc(vertices, sizeof(float) * nb_vertices)))
			exit_callback(gomo, 20, "vertex realloc failed");
	if (nb_textures)
		if (!(textures = (float *)realloc(textures, sizeof(float) * nb_textures)))
			exit_callback(gomo, 21, "textures realloc failed");
	if (nb_normals)
		if (!(normals = (float *)realloc(normals, sizeof(float) * nb_normals)))
			exit_callback(gomo, 22, "normals realloc failed");
	gomo->obj->nb_faces = nb_faces;
	gomo->obj->nb_vertices = gomo->obj->nb_faces * 3;
	create_obj(gomo, vertices, textures, normals);
	free_null((void *)vertices);
	free_null((void *)textures);
	free_null((void *)normals);
}
