#include "../../include/gomo.h"

int ray_intersects_quad(gomo_t *gomo, ray_t ray, vec3_t quadPos, float halfWidth, float halfHeight)
{
	// Compute plane normal: use camera->eye and quadPos like the shader does
	vec3_t toCamera = sub_vec3(gomo->camera->eye, quadPos);
	toCamera = norm_vec3(toCamera); // normalize to match shader's normalize()
	float angle = atan2f(toCamera.x, toCamera.z); // matches shader: atan(toCamera.x, toCamera.z)
	float cosA = cosf(angle);
	float sinA = sinf(angle);
	vec3_t quadNormal = (vec3_t){ sinA, 0.0f, cosA };
	// quadNormal is already unit length (sin^2+cos^2==1)

	// Denominator for ray-plane intersection
	float denom = dot_vec3(ray.direction, quadNormal);
	if (fabsf(denom) < 1e-6f)
		return 0; // Ray parallel to quad plane

	// Compute t using general plane intersection: t = dot(P0 - O, N) / dot(D, N)
	vec3_t p0_minus_o = (vec3_t){ quadPos.x - ray.origin.x, quadPos.y - ray.origin.y, quadPos.z - ray.origin.z };
	float t = dot_vec3(p0_minus_o, quadNormal) / denom;
	if (t < 0)
		return 0; // intersection behind origin

	// Intersection point in world space
	vec3_t intersectionPoint;
	intersectionPoint.x = ray.origin.x + t * ray.direction.x;
	intersectionPoint.y = ray.origin.y + t * ray.direction.y;
	intersectionPoint.z = ray.origin.z + t * ray.direction.z;

	// Undo shader rotation: rotate intersection point by -angle around Y to text-local frame
	float dx = intersectionPoint.x - quadPos.x;
	float dz = intersectionPoint.z - quadPos.z;
	float localX = dx * cosA - dz * sinA; // Inverse rotation: R(-angle)
	float localY = intersectionPoint.y - quadPos.y; // vertical in text local

	if (localX >= -halfWidth && localX <= halfWidth &&
		localY >= -halfHeight && localY <= halfHeight)
	{
		// debug markers
		add_line_to_render(gomo, (vec3_t){intersectionPoint.x - 0.1f, intersectionPoint.y, intersectionPoint.z}, (vec3_t){intersectionPoint.x + 0.1f, intersectionPoint.y, intersectionPoint.z}, (vec3_t){1.0f, 0.0f, 0.0f}, 371);
		add_line_to_render(gomo, (vec3_t){intersectionPoint.x, intersectionPoint.y - 0.1f, intersectionPoint.z}, (vec3_t){intersectionPoint.x, intersectionPoint.y + 0.1f, intersectionPoint.z}, (vec3_t){1.0f, 0.0f, 0.0f}, 372);
		add_line_to_render(gomo, (vec3_t){intersectionPoint.x, intersectionPoint.y, intersectionPoint.z - 0.1f}, (vec3_t){intersectionPoint.x, intersectionPoint.y, intersectionPoint.z + 0.1f}, (vec3_t){1.0f, 0.0f, 0.0f}, 373);
		// draw normal as a line starting from intersectionPoint in world space
		vec3_t normalEnd = add_vec3(intersectionPoint, prod_vec3(quadNormal, 0.5f));
		add_line_to_render(gomo, intersectionPoint, normalEnd, (vec3_t){0.12f, 1.0f, 0.0f}, 374);
		return 1;
	}

	return 0;
}

int intersectText(gomo_t *gomo, ray_t ray, hit_t *intersection)
{
	for (int i = 0; i < NB_TEXT; i++)
	{
		text_t *text = &gomo->text[i];
		if (!text->id || !text->text || text->proj != 1 || !text->clickable)
			continue;

		float textWidth = calculate_text_width(gomo, text->font, text->text, text->scale);
		float textHeight = 48.0f * text->scale;
		float halfWidth = textWidth * 0.5f;
		float halfHeight = textHeight * 0.5f;

		// Use stored text->pos as center; ray_intersects_quad will undo shader rotation and test in text-local XY
		if (ray_intersects_quad(gomo, ray, text->pos, halfWidth, halfHeight))
		{
			// recompute t for hit point using same angle calculation as ray_intersects_quad
			vec3_t toCamera = sub_vec3(gomo->camera->eye, text->pos);
			toCamera = norm_vec3(toCamera);
			float angle = atan2f(toCamera.x, toCamera.z);
			float cosA = cosf(angle);
			float sinA = sinf(angle);
			vec3_t quadNormal = (vec3_t){ sinA, 0.0f, cosA };
			
			float denom = dot_vec3(ray.direction, quadNormal);
			if (fabsf(denom) < 1e-6f)
				continue;
			vec3_t p0_minus_o = (vec3_t){ text->pos.x - ray.origin.x, text->pos.y - ray.origin.y, text->pos.z - ray.origin.z };
			float t = dot_vec3(p0_minus_o, quadNormal) / denom;
			if (t < 0)
				continue;

			intersection->hit = text->id;
			intersection->point.x = ray.origin.x + t * ray.direction.x;
			intersection->point.y = ray.origin.y + t * ray.direction.y;
			intersection->point.z = ray.origin.z + t * ray.direction.z;
			intersection->normal = quadNormal;
			return 1;
		}
	}
	intersection->hit = -1;
	return 0;
}

int intersectBoard(ray_t ray, hit_t *intersection)
{
    float dDotN = dot_vec3(ray.direction, (vec3_t){0.0f, 1.0f, 0.0f});

    // Use epsilon for floating-point comparison
    if (fabsf(dDotN) < 1e-6f)
        return 0;

    float t = (0.43f - ray.origin.y) / ray.direction.y;

    // Ignore intersections behind the ray origin
    if (t < 0)
        return 0;

    intersection->point.x = ray.origin.x + t * ray.direction.x;
    intersection->point.y = -5.904f; // Goban height
    intersection->point.z = ray.origin.z + t * ray.direction.z;


    return 1;
}

ray_t createRay(gomo_t *gomo, double xpos, double ypos)
{
    ray_t ray;

    // Convert screen coordinates to normalized device coordinates (NDC)
    float ndcX = (2.0f * (float)xpos) / (float)WIDTH - 1.0f;
    float ndcY = 1.0f - (2.0f * (float)ypos) / (float)HEIGHT;
	vec4_t ray_clip = {ndcX, ndcY, -1.0f, 1.0f};
	float *inv_proj = inv_mat4(gomo->camera->projection);
	float *inv_view = inv_mat4(gomo->camera->view);
	vec4_t ray_eye = mulv_mat4(inv_proj, ray_clip);
	ray_eye = (vec4_t){ray_eye.x, ray_eye.y, -1.0f, 0.0f};
	vec4_t ray_world = mulv_mat4(inv_view, ray_eye);
	ray.direction = norm_vec3((vec3_t){ray_world.x, ray_world.y, ray_world.z});

	free(inv_proj);
	free(inv_view);

    ray.origin = gomo->camera->eye;
	ray.direction = norm_vec3(ray.direction); // Normalize the direction vector

    return ray;
}