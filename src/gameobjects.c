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
Error GameObject_player_update(GameObject* parent, uint32_t index_of_self);
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
Error GameObject_fps_display_update(GameObject* parent, uint32_t index_of_self);
Error GameObject_fps_display_create(SDL_FPoint pos);

#if __INCLUDE_LEVEL__ == 0 /////////////////////////////////////////////////////

// Function definitions
Error GameObject_player_update(GameObject* parent, uint32_t index_of_self) {
	(void) index_of_self;
	struct GameObject_Player* self = parent->data;

	SDL_FRect rect = (SDL_FRect) {0.0, 0.0, 50.0, 50.0};
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderRect(renderer, &rect);

	if(Eng_get_key_pressed(KEY_A)) self->ang_vel -= self->force_rot;
	if(Eng_get_key_pressed(KEY_D)) self->ang_vel += self->force_rot;
	if(Eng_get_key_pressed(KEY_W)) {
		self->vel = pointf_add(
			pointf_force(
				self->force_main_thruster * get_deltatime_factor(), self->rot
			),
			self->vel
		);
	}
	if(Eng_get_key_pressed(KEY_I))
		self->vel = pointf_add(
			pointf_force(self->force_main_thruster, self->rot), self->vel
		);
	if(Eng_get_key_pressed(KEY_J))
		self->vel = pointf_add(
			pointf_force(
				self->force_main_thruster, WRAP_COMPASS((int) self->rot - 90)
			),
			self->vel
		);

	if(Eng_get_key_pressed(KEY_K))
		self->vel = pointf_add(
			pointf_force(
				self->force_main_thruster, WRAP_COMPASS((int) self->rot + 180)
			),
			self->vel
		);
	if(Eng_get_key_pressed(KEY_L))
		self->vel = pointf_add(
			pointf_force(
				self->force_main_thruster, WRAP_COMPASS((int) self->rot + 90)
			),
			self->vel
		);
	if(Eng_get_key_pressed(KEY_MOUSE_LEFT)) {
		self->rot = pointf_bearing(self->pos, Eng_mouse_pos);
	}

	self->pos = pointf_add(self->pos, self->vel);
	self->rot = WRAP_COMPASS((int) (self->rot + self->ang_vel));

	// TODO Switch these to modfs (or just if it)
	self->pos.x = PROPER_MOD((int) self->pos.x, Eng_screensize.x);
	self->pos.y = PROPER_MOD((int) self->pos.y, Eng_screensize.y);

	SDL_FRect  player_rect = {player.pos.x, player.pos.y, 50, 50};
	SDL_FPoint player_off  = {25, 25};
	SDL_FPoint player_ctr  = pointf_add(player.pos, player_off);
	// Grey background
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	// Draw player
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	SDL_RenderTextureRotated(
		renderer, TEX_PLAYER.tex, NULL, &player_rect, player.rot, &player_off,
		SDL_FLIP_NONE
	);

	// Draw lateral movement guides
	if(GET_KEY_PRESSED(KEY_LALT)) {
		const SDL_FPoint bow =
			pointf_add(pointf_force(75, player.rot), player_ctr);
		const SDL_FPoint stern = pointf_add(
			pointf_force(75, PROPER_MOD((int) player.rot + 180, 360)),
			player_ctr
		);
		const SDL_FPoint port = pointf_add(
			pointf_force(75, PROPER_MOD((int) player.rot - 90, 360)), player_ctr
		);
		const SDL_FPoint starboard = pointf_add(
			pointf_force(75, PROPER_MOD((int) player.rot + 90, 360)), player_ctr
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
	SDL_FPoint dest  = pointf_add(player_ctr, pointf_scale(player.vel, 10));
	SDL_FPoint dest2 = pointf_add(player_ctr, pointf_scale(player.vel, -10));
	SDL_FRect  dest_icantbefucked = {dest.x - 12.5f, dest.y - 12.5f, 25, 25};
	SDL_FRect  dest_icantbefucked2eletricboogaloo = {
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

Error GameObject_fps_display_update(
	GameObject* parent, uint32_t index_of_self
) {
	(void) index_of_self;
	struct GaneObject_FPS_Display* self = parent->data;

	char fps_string[64] = {0};
	snprintf(fps_string, sizeof(fps_string), "FPS: %d", engine.current_fps);
	TTF_SetTextString(self->display, fps_string, sizeof(fps_string));

	SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
	TTF_DrawRendererText(self->display, self->pos.x, self->pos.y);
	return ERR_PASS;
}
Error GameObject_fps_display_create(SDL_FPoint pos) {
	struct GaneObject_FPS_Display self = {
		.display = nullptr,
		.pos     = pos,
	};
	self.display =
		TTF_CreateText(engine.text_engine, engine.font, "hewo :3", 0);
	struct GameObject_FPS_Display* new = NULL;
	if(Eng_create_object(
		   &self, (void*) &new, sizeof(struct GaneObject_FPS_Display),
		   GAMEOBJECT_FPS_DISPLAY
	   ) == ERR_FATAL)
		return ERR_FATAL;
	SDL_Log("Got: %p", new);
	Eng_hook_update(GameObject_fps_display_update, new);
	return ERR_PASS;
}

#endif
