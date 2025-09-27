#include "../../include/gomo.h"

void    load_new_material(gomo_t *gomo, char *name)
{
    material_t *new_material;

    if (!(new_material = (material_t *)malloc(sizeof(material_t))))
        exit_callback(gomo, 201, "new material malloc failed");
    new_material->name = string_copy(gomo, new_material->name, name);
    new_material->ambient = (vec3_t){0.0f, 0.0f, 0.0f};
    new_material->diffuse = (vec3_t){0.0f, 0.0f, 0.0f};
    new_material->specular = (vec3_t){0.0f, 0.0f, 0.0f};
    new_material->emission = (vec3_t){0.0f, 0.0f, 0.0f};
    new_material->specular_exp = 1.0f;
    new_material->dissolve = 1.0f;
    new_material->refract_index = 1.0f;
    new_material->illum = 0;
    new_material->texture = NULL;
    new_material->id = 0;
    new_material->next = NULL;
    if (gomo->obj->materials->name == NULL) {
        new_material->first = new_material;
        gomo->obj->materials = new_material;
    } else {
        new_material->first = gomo->obj->materials->first;
        gomo->obj->materials->next = new_material;
        gomo->obj->materials = gomo->obj->materials->next;
    }
}

vec3_t  vec3_copy(gomo_t *gomo, char *line)
{
    vec3_t vec;

    if (count_space(line) != 2)
        exit_callback(gomo, 202, "wrong .mtl file");
    vec.x = strtof(line, &line);
    vec.y = strtof(line, &line);
    vec.z = strtof(line, &line);
    if (vec.x < 0.0f || vec.y < 0.0f || vec.z < 0.0f)
        exit_callback(gomo, 203, "negative value in .mtl file");
    return vec;
}

void    load_material(gomo_t *gomo, char *name)
{
    FILE *fp;
	char *line = NULL;
	size_t len = 0;
	size_t read;
    unsigned int i = 0;

	char path[256];
    size_t name_len = strlen(name);
    if (name_len > 0 && name[name_len - 1] == '\n') {
        name[name_len - 1] = '\0';
    }
    snprintf(path, sizeof(path), "resources/%s", name);
	if (!(fp = fopen(path, "r")))
        exit_callback(gomo, 200, "wrong .mtl file, fopen failed");

	while ((read = getline(&line, &len, fp)) != (size_t)(-1))
	{
        if (line != NULL && line[0] != '#')
		{
			if (!strncmp(line, "newmtl ", 7))
			{
				load_new_material(gomo, &line[7]);
                i++;
			}
			else if (!strncmp(line, "Ka ", 3))
			{
				gomo->obj->materials->ambient = vec3_copy(gomo, &line[3]);
			}
			else if (!strncmp(line, "Kd ", 3))
			{
				gomo->obj->materials->diffuse = vec3_copy(gomo, &line[3]);
			}
			else if (!strncmp(line, "Ks ", 3))
			{
				gomo->obj->materials->specular = vec3_copy(gomo, &line[3]);
			}
            else if (!strncmp(line, "Ke ", 3))
            {
                gomo->obj->materials->emission = vec3_copy(gomo, &line[3]);
            }
            else if (!strncmp(line, "d ", 2))
            {
                gomo->obj->materials->dissolve = atof(&line[2]);
            }
            else if (!strncmp(line, "Ni ", 3))
            {
                gomo->obj->materials->refract_index = atof(&line[3]);
            }
            else if (!strncmp(line, "illum ", 6))
            {
                gomo->obj->materials->illum = atoi(&line[6]);
            }
            else if (!strncmp(line, "map_Kd ", 7))
            {
                gomo->obj->materials->texture = string_copy(gomo, gomo->obj->materials->texture, &line[7]);
            }
			else if (!strncmp(line, "Ns ", 3))
			{
				gomo->obj->materials->specular_exp = atof(&line[3]);
			}
		}
		free(line);
		line = NULL;
	}
	free_null((void *)line);
	fclose(fp);
    if (i > 0) {
        if (!(gomo->obj->materials_ids = (int *)malloc(sizeof(int) * (i + 1))))
            exit_callback(gomo, 56, "materials_ids malloc failed");
        for (unsigned int j = 0; j < i; j++)
        {
            gomo->obj->materials_ids[j] = -1;
        }
        gomo->obj->materials_ids[i] = -2;
    }
    /*
    gomo->obj->materials = gomo->obj->materials->first;
    while (gomo->obj->materials != NULL)
    {
        printf("Material: %s\n", gomo->obj->materials->name);
        printf("  Ambient: %f %f %f\n", gomo->obj->materials->ambient.x, gomo->obj->materials->ambient.y, gomo->obj->materials->ambient.z);
        printf("  Diffuse: %f %f %f\n", gomo->obj->materials->diffuse.x, gomo->obj->materials->diffuse.y, gomo->obj->materials->diffuse.z);
        printf("  Specular: %f %f %f\n", gomo->obj->materials->specular.x, gomo->obj->materials->specular.y, gomo->obj->materials->specular.z);
        printf("  Emission: %f %f %f\n", gomo->obj->materials->emission.x, gomo->obj->materials->emission.y, gomo->obj->materials->emission.z);
        printf("  Specular Exp: %f\n", gomo->obj->materials->specular_exp);
        printf("  Dissolve: %f\n", gomo->obj->materials->dissolve);
        printf("  Refractive Index: %f\n", gomo->obj->materials->refract_index);
        printf("  Illumination Model: %d\n", gomo->obj->materials->illum);
        printf("  Texture: %s\n", gomo->obj->materials->texture ? gomo->obj->materials->texture : "None");
        gomo->obj->materials = gomo->obj->materials->next;
    }*/
}