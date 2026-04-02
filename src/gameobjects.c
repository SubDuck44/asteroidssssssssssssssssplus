#pragma once

#include "engine.c"
#include "main.c"

#include <SDL3/SDL.h>

enum GameObject_Types : uint32_t {
	GAMEOBJECT_PLAYER,
	GAMEOBJECT_ASTEROID,
	GAMEOBJECT_FPS_DISPLAY,
	GAMEOBJECT_NUM
};

// Declarations
struct GameObject_Player {
	bool       alive;
	SDL_FPoint pos;
	double     rot;
	SDL_FPoint vel;
	float      ang_vel;
	float      force_main_thruster;
	float      force_rcs_thrusters;
	float      force_rot;
};
Error GameObject_player_update(void* data, uint32_t index_of_self);
Error GameObject_player_create(void);

struct GameObject_Asteroid {
	SDL_FPoint pos;
	float      rot;
	SDL_FPoint vel;
	double     ang_vel;
};

struct GaneObject_FPS_Display {
	TTF_Text*  display;
	SDL_FPoint pos;
};
Error GameObject_fps_display_update(void* data, uint32_t index_of_self);
Error GameObject_fps_display_create(SDL_FPoint pos);

#if __INCLUDE_LEVEL__ == 0 /////////////////////////////////////////////////////

// Function definitions
Error GameObject_player_update(void* data, uint32_t index_of_self) {
	(void) index_of_self;
	struct GameObject_Player* self = data;

	// TODO Fix float inaccuracy in player movement controller
	double delta_time  = Eng_get_deltatime_factor();
	double thrust_main = self->force_main_thruster * delta_time;
	double thrust_rcs  = self->force_rcs_thrusters * delta_time;
	double force_rot   = self->force_rot * Eng_get_deltatime_factor();
	if(Eng_get_key_pressed(KEY_A)) self->ang_vel -= force_rot;
	if(Eng_get_key_pressed(KEY_D)) self->ang_vel += force_rot;
	if(Eng_get_key_pressed(KEY_W)) {
		self->vel =
			Eng_pointf_add(Eng_pointf_force(thrust_main, self->rot), self->vel);
	}
	if(Eng_get_key_pressed(KEY_I))
		self->vel =
			Eng_pointf_add(Eng_pointf_force(thrust_rcs, self->rot), self->vel);
	if(Eng_get_key_pressed(KEY_J))
		self->vel = Eng_pointf_add(
			Eng_pointf_force(thrust_rcs, WRAP_COMPASS((int) self->rot - 90)),
			self->vel
		);

	if(Eng_get_key_pressed(KEY_K))
		self->vel = Eng_pointf_add(
			Eng_pointf_force(thrust_rcs, WRAP_COMPASS((int) self->rot + 180)),
			self->vel
		);
	if(Eng_get_key_pressed(KEY_L))
		self->vel = Eng_pointf_add(
			Eng_pointf_force(thrust_rcs, WRAP_COMPASS((int) self->rot + 90)),
			self->vel
		);
	if(Eng_get_key_pressed(KEY_MOUSE_LEFT)) {
		self->rot = Eng_pointf_bearing(self->pos, Eng_mouse_pos);
	}

	self->pos = Eng_pointf_add(self->pos, self->vel);
	self->rot = WRAP_COMPASS((int) (self->rot + self->ang_vel));

	// TODO Switch these to modfs (or just if it)
	self->pos.x = PROPER_MOD((int) self->pos.x, Eng_screensize.x);
	self->pos.y = PROPER_MOD((int) self->pos.y, Eng_screensize.y);

	SDL_FRect  player_rect = {self->pos.x, self->pos.y, 50, 50};
	SDL_FPoint player_off  = {25, 25};
	SDL_FPoint player_ctr  = Eng_pointf_add(self->pos, player_off);

	// Draw player
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	SDL_RenderTextureRotated(
		renderer, TEX_PLAYER.tex, NULL, &player_rect, self->rot, &player_off,
		SDL_FLIP_NONE
	);

	// Draw lateral movement guides
	if(Eng_get_key_pressed(KEY_LALT)) {
		const SDL_FPoint bow =
			Eng_pointf_add(Eng_pointf_force(75, self->rot), player_ctr);
		const SDL_FPoint stern = Eng_pointf_add(
			Eng_pointf_force(75, PROPER_MOD((int) self->rot + 180, 360)),
			player_ctr
		);
		const SDL_FPoint port = Eng_pointf_add(
			Eng_pointf_force(75, PROPER_MOD((int) self->rot - 90, 360)),
			player_ctr
		);
		const SDL_FPoint starboard = Eng_pointf_add(
			Eng_pointf_force(75, PROPER_MOD((int) self->rot + 90, 360)),
			player_ctr
		);
		thickLineRGBA(
			renderer, bow.x, bow.y, stern.x, stern.y, 3, 25, 60, 165, 255
		);
		thickLineRGBA(
			renderer, port.x, port.y, starboard.x, starboard.y, 3, 25, 60, 165,
			255
		);
	}

	// Draw velocity vector
	SDL_FPoint dest =
		Eng_pointf_add(player_ctr, Eng_pointf_scale(self->vel, 10));
	SDL_FPoint dest2 =
		Eng_pointf_add(player_ctr, Eng_pointf_scale(self->vel, -10));
	SDL_FRect dest_icantbefucked = {dest.x - 12.5f, dest.y - 12.5f, 25, 25};
	SDL_FRect dest_icantbefucked2eletricboogaloo = {
		dest2.x - 12.5f, dest2.y - 12.5f, 25, 25
	};
	thickLineColor(
		renderer, player_ctr.x, player_ctr.y, dest.x, dest.y, 3,
		htobe32(0x94DE0AFF)
	);
	SDL_RenderTexture(renderer, TEX_PROGRADE.tex, NULL, &dest_icantbefucked);
	thickLineColor(
		renderer, player_ctr.x, player_ctr.y, dest2.x, dest2.y, 3,
		htobe32(0xD2DB27FF)
	);
	SDL_RenderTexture(
		renderer, TEX_RETROGRADE.tex, NULL, &dest_icantbefucked2eletricboogaloo
	);

	return ERR_PASS;
}

Error GameObject_player_create(void) {
	struct GameObject_Player self = {
		.alive               = true,
		.pos                 = {0},
		.rot                 = 0.0,
		.vel                 = {0},
		.ang_vel             = 0.0,
		.force_rot           = 0.2,
		.force_main_thruster = 0.25,
		.force_rcs_thrusters = 0.1,
	};
	struct GameObject_Player* new = NULL;
	Eng_create_object(
		&self, (void*) &new, sizeof(struct GameObject_Player), GAMEOBJECT_PLAYER
	);
	Eng_hook_update(GameObject_player_update, new);
	return ERR_PASS;
}

Error GameObject_fps_display_update(void* data, uint32_t index_of_self) {
	(void) index_of_self;
	struct GaneObject_FPS_Display* self = data;

	char fps_string[64] = {0};
	snprintf(fps_string, sizeof(fps_string), "FPS: %d", Eng_current_fps);
	TTF_SetTextString(self->display, fps_string, sizeof(fps_string));

	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	TTF_DrawRendererText(self->display, self->pos.x, self->pos.y);
	return ERR_PASS;
}
Error GameObject_fps_display_create(SDL_FPoint pos) {
	struct GaneObject_FPS_Display self = {
		.display = nullptr,
		.pos     = pos,
	};
	self.display = TTF_CreateText(Eng_text_engine, Eng_font, "hewo :3", 0);
	struct GameObject_FPS_Display* new = NULL;
	if(Eng_create_object(
		   &self, (void*) &new, sizeof(struct GaneObject_FPS_Display),
		   GAMEOBJECT_FPS_DISPLAY
	   ) == ERR_FATAL)
		return ERR_FATAL;
	Eng_hook_update(GameObject_fps_display_update, new);
	return ERR_PASS;
}

#endif
