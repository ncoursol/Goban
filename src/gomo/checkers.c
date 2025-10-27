#include "../../include/gomo.h"

int can_place_stone(gomo_t *gomo)
{
	if (MENU || !gomo->game)
		return 0;
	if (!gomo->tmp_stone || gomo->nb_stones >= 361)
		return 0;
	if (gomo->game->mode == 2 || (gomo->game->mode == 1 && !gomo->game->players[gomo->game->swap2_player].is_human))
		return 0;
	return 1;
}

void display_swap2_info(gomo_t *gomo)
{
	char buffer[32];
	snprintf(buffer, sizeof(buffer), "%s play as", gomo->game->players[gomo->game->current_player].name);

	// if we are at the swap2 color choice step
	if ((gomo->game->swap2_step == 1 && gomo->game->move_count == 3) ||
		(gomo->game->swap2_step == 3 && gomo->game->move_count == 5))
	{
		add_text_to_render(gomo, "font_text2", buffer, (vec3_t){0.0f, 1.5f, 0.0f}, (vec3_t){0.0f, 0.0f, 0.0f}, 1, 0, 0.004f, (vec3_t){1.0f, 0.5f, 0.5f}, 1, 5);
		add_text_to_render(gomo, "font_text2", "Black", (vec3_t){0.0f, 1.3f, 0.0f}, (vec3_t){0.0f, 0.0f, 0.0f}, 1, 1, 0.003f, (vec3_t){1.0f, 0.5f, 0.5f}, 1, 6);
		add_text_to_render(gomo, "font_text2", "White", (vec3_t){0.0f, 1.1f, 0.0f}, (vec3_t){0.0f, 0.0f, 0.0f}, 1, 1, 0.003f, (vec3_t){1.0f, 0.5f, 0.5f}, 1, 7);
		if (gomo->game->swap2_step == 1)
			add_text_to_render(gomo, "font_text2", "Place 2 stones", (vec3_t){0.0f, 0.9f, 0.0f}, (vec3_t){0.0f, 0.0f, 0.0f}, 1, 1, 0.003f, (vec3_t){1.0f, 0.5f, 0.5f}, 1, 8);
	}
}

void change_camera_target(gomo_t *gomo)
{
	if (gomo->game->mode != 0)
		return;
	if ((gomo->game->swap2_step == 0 && floor(gomo->camera->targetPos.y) != floor(PI)) ||
		(gomo->game->swap2_step == 3 && floor(gomo->camera->targetPos.y) != floor(PI)))
	{
		if (!(ANIMATE))
			C_ANIMATE;
		gomo->camera->targetPos = P1_POS;
	} 
	else if ((gomo->game->swap2_step == 1 && floor(gomo->camera->targetPos.y) != 0.0f))
	{
		if (!(ANIMATE))
			C_ANIMATE;
		gomo->camera->targetPos = P2_POS;
	} 
	else if (gomo->game->swap2_step == 4)
	{
		if (!(ANIMATE))
			C_ANIMATE;
		gomo->camera->targetPos = gomo->game->current_player == 0 ? P1_POS : P2_POS;
	}
}