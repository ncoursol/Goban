/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncoursol <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/04/17 19:51:32 by ncoursol          #+#    #+#             */
/*   Updated: 2022/04/17 19:53:54 by ncoursol         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/scop.h"

void	free_null(void *a)
{
	if (a != NULL) {
		free(a);
		a = NULL;
	}
}

int			count_space(char *a)
{
	int ret = 0;
	int i = 0;

	if (a != NULL) {
		while (a[i]) {
			if (a[i] == ' ')
				ret++;
			i++;
		}
	}
	return (ret);
}

char		*strjoin(const char *s1, const char *s2)
{
	char	*str;
	size_t	i;
	size_t	j;
	size_t	k;

	str = NULL;
	i = 0;
	j = 0;
	if (s1)
		i = strlen(s1);
	if (s2)
		j = strlen(s2);
	if (!(str = (char*)malloc(sizeof(char) * (i + j + 1))))
		return (NULL);
	k = 0;
	while (s1 && s1[k] && k < i)
	{
		str[k] = s1[k];
		k++;
	}
	i = 0;
	while (s2 && s2[i] && i < j)
	{
		str[k + i] = s2[i];
		i++;
	}
	str[k + i] = '\0';
	return (str);
}

float		*float_copy(scop_t *scop, float *dest, int len, char *line, int *nb, int *nb_buff)
{
	char 	*tmp;

	tmp = line;
	if (*nb + len >= V_BUFF_SIZE * *nb_buff) {
		*nb_buff += 1;
		if (!(dest = (float*)realloc(dest, sizeof(float) * (V_BUFF_SIZE * *nb_buff))))
			exit_callback(scop, 12, "float_copy realloc failed");
		for (int i = (V_BUFF_SIZE * *nb_buff) - V_BUFF_SIZE; i < V_BUFF_SIZE * *nb_buff; i++) {
			dest[i] = 0;
		}
	}
	while (tmp[0] == ' ') {
		if (!tmp[1])
			exit_callback(scop, 13, "wrong .obj file");
		if (tmp[1] == '+' || tmp[1] == '-') {
			if (!tmp[2] || tmp[2] < '0' || tmp[2] > '9')
			exit_callback(scop, 14, "wrong .obj file");
		}
		else if (tmp[1] < '0' || tmp[1] > '9')
			exit_callback(scop, 15, "wrong .obj file");
		dest[*nb] = strtof(tmp, &tmp);
		*nb += 1;
	}
	return (dest);
}

char	*string_copy(scop_t *scop, char *dest, char *line)
{
	size_t 	len;

	len = strlen(line);
	if (!(dest = (char*)malloc(sizeof(char) * len)))
		exit_callback(scop, 9, "string_copy malloc failed");
	memcpy(dest, line, len);
	dest[len - 1] = '\0';
	return (dest);
}

data_t	*data_copy(data_t *a, data_t *b) {
	a->vertex = b->vertex;
	a->texture = b->texture;
	a->normal = b->normal;
	a->next = NULL;
	return (a);
}
