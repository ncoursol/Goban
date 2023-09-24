/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   triangulation.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncoursol <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/04/14 13:55:17 by ncoursol          #+#    #+#             */
/*   Updated: 2023/01/27 22:40:24 by ncoursol         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/scop.h"

/*
	Basic ear-clipping program:
	don't work with concave, non-coplanar and holes polygons..
	face:  0->1->2->3->4->5->null
	tri2   0->2->3->null
	tri3	 0->3->4->null
	tri4	 0->4->5->null
*/

int			ear_clipping(scop_t *scop, int *f, int nb)
{
	int 		i = 0;
	int			k;
	data_t	*tmp;

	scop->obj->faces[*f] = scop->obj->faces[*f]->first;
	tmp = scop->obj->faces[*f];
	while (tmp != NULL) {
		k = 0;
		if (i == 0) {
			k++;
			while (k < nb - 2) {
				scop->obj->faces[*f + k] = data_copy(scop->obj->faces[*f + k], tmp);
				scop->obj->faces[*f + k]->first = scop->obj->faces[*f + k];
				k++;
			}
			tmp = tmp->next->next;
		} else {
			while (k < 2) {
				new_vertex(scop, *f + i);
				scop->obj->faces[*f + i] = data_copy(scop->obj->faces[*f + i], tmp);
				if (tmp->next == NULL)
					break;
				if (k == 0)
					tmp = tmp->next;
				k++;
			}
		}
		if (tmp->next == NULL)
			break;
		i++;
	}
	*f += nb - 3;
	return (1);
}

void    triangulate(scop_t *scop, int *f, int *b_f, int nb)
{
	if (*f + (nb - 3) >= V_BUFF_SIZE * *b_f) {
		*b_f += 1;
		if (!(scop->obj->faces = (data_t**)realloc(scop->obj->faces, sizeof(data_t*) * (V_BUFF_SIZE * *b_f))))
			exit_callback(scop, 18, "faces realloc failed");
		scop->obj->faces_size = V_BUFF_SIZE * *b_f;
		init_face_data(scop, *b_f);
	}
	ear_clipping(scop, f, nb);
	scop->obj->nb_faces += nb - 3;
}
