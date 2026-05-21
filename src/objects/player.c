#pragma once

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#if __INCLUDE_LEVEL__ == 0 /////////////////////////////////////////////////////

#include "../engine.c"
#include "../utils.c"
#include <stdlib.h>

#endif

// Player
enum PlayermModules : uint8_t {
	PLAYERMODULE_SOLAR,
	PLAYERMODULE_ANTENNA,
	PLAYERMODULE_CLAW,
};
// -----------------------------------------------------------------------------
struct Entity_player {
	bool      alive;
	Transform tf;
	Vector2f  vel;
	float     ang_vel;
	float     force_main_thruster;
	float     force_rcs_thrusters;
	float     force_rot;
	ColRect   hitbox;
	bool      hit_something; // TODO: Move this to collision system
	uint8_t   modules;
};
// -----------------------------------------------------------------------------
bool Entity_player_create(void);
void Entity_player_update(struct Entity_player* self, size_t index);
void Entity_player_collide(void* data, void* other, uint32_t typeof_other);

#if __INCLUDE_LEVEL__ == 0 /////////////////////////////////////////////////////

#include <SDL3_gfx/SDL3_gfxPrimitives.h>
#include <endian.h>
#include <stdio.h>

#include "../res.c"

bool Entity_player_create(void) {
	DynArrPush(
		&entities_players, ((struct Entity_player) {
							   .alive               = true,
							   .vel                 = {0},
							   .ang_vel             = 0.0,
							   .force_rot           = 0.2,
							   .force_main_thruster = 0.25,
							   .force_rcs_thrusters = 0.1,
							   .modules             = 3,
							   .tf                  = (Transform) {
													.pos  = (Vector2l) {0, 0},
													.size = (SDL_FPoint) {100, 100},
													.ctr  = (SDL_FPoint) {50, 50},
													.rot  = 0,
                               },
						   })
	);

	struct Entity_player* self =
		&entities_players.arr[entities_players.len - 1];

	Eng_make_hitbox(
		&self->hitbox, (void*) self, Entity_player_collide, ENTITY_PLAYER,
		self->tf.pos.x, self->tf.pos.y, self->tf.size.x, self->tf.size.y
	);

	return true;
}

void Entity_player_update(struct Entity_player* self, size_t index) {
	(void) index;

	double delta_time  = Eng_get_deltatime_factor();
	double thrust_main = self->force_main_thruster * delta_time;
	double thrust_rcs  = self->force_rcs_thrusters * delta_time;
	double force_rot   = self->force_rot * Eng_get_deltatime_factor();
	if(Eng_get_key_down(KEY_A)) self->ang_vel -= force_rot;
	if(Eng_get_key_down(KEY_D)) self->ang_vel += force_rot;
	if(Eng_get_key_down(KEY_W)) {
		self->vel =
			Vec2f_add(Vec2f_force(thrust_main, self->tf.rot), self->vel);
	}
	if(Eng_get_key_down(KEY_I))
		self->vel = Vec2f_add(Vec2f_force(thrust_rcs, self->tf.rot), self->vel);
	if(Eng_get_key_down(KEY_J))
		self->vel = Vec2f_add(
			Vec2f_force(thrust_rcs, WRAP_COMPASS((int) self->tf.rot - 90)),
			self->vel
		);

	if(Eng_get_key_down(KEY_K))
		self->vel = Vec2f_add(
			Vec2f_force(thrust_rcs, WRAP_COMPASS((int) self->tf.rot + 180)),
			self->vel
		);
	if(Eng_get_key_down(KEY_L))
		self->vel = Vec2f_add(
			Vec2f_force(thrust_rcs, WRAP_COMPASS((int) self->tf.rot + 90)),
			self->vel
		);
	if(Eng_get_key_down(KEY_MOUSE_LEFT)) {
		self->tf.rot = FPoint_angle_to(
			Cam_world_to_screen(self->tf.pos, &Eng_camera), Eng_mouse_pos
		);
	}
	if(Eng_get_key_pressed(KEY_1)) self->modules ^= (1 << PLAYERMODULE_SOLAR);
	if(Eng_get_key_pressed(KEY_2) &&
	   (self->modules ^ (1 << PLAYERMODULE_CLAW)) > self->modules)
		self->modules ^= (1 << PLAYERMODULE_ANTENNA);
	if(Eng_get_key_pressed(KEY_3) &&
	   (self->modules ^ (1 << PLAYERMODULE_ANTENNA)) > self->modules)
		self->modules ^= (1 << PLAYERMODULE_CLAW);

	// Set own position
	printf("Pos was: %ld, %ld\n", self->tf.pos.x, self->tf.pos.y);
	self->tf.pos = Vec2l_add_Vec2f(
		self->tf.pos, Vec2f_scale(self->vel, DEFAULT_FIXED_POINT)
	);
	self->tf.rot      = WRAP_COMPASS((int) (self->tf.rot + self->ang_vel));
	Eng_camera.target = self->tf.pos;
	printf("Pos is: %ld, %ld\n", self->tf.pos.x, self->tf.pos.y);

	// Update hitbox
	Eng_set_hitbox(&self->hitbox, self->tf.pos);
	Eng_update_hitbox(&self->hitbox);

	SDL_FPoint player_origin = {50 * Eng_camera.zoom, 50 * Eng_camera.zoom};
	SDL_FRect  player_rect   = Cam_transform_rect(&self->tf, &Eng_camera, NULL);
	SDL_FPoint player_ctr    = Cam_world_to_screen(self->tf.pos, &Eng_camera);

	// Draw
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	SDL_RenderTextureRotated(
		renderer, TEX_PLAYER.tex, NULL, &player_rect, self->tf.rot,
		&player_origin, SDL_FLIP_NONE
	);

	// Draw hitbox
	Eng_draw_hitbox(&self->hitbox, self->hit_something);
	self->hit_something = false;

	// Draw player modules
	if(self->modules & (1 << PLAYERMODULE_SOLAR)) {
		SDL_FRect solar_dest = {
			player_rect.x - player_rect.w, player_rect.y, player_rect.w * 3,
			player_rect.h
		};
		SDL_FPoint solar_origin = {
			player_origin.x + player_rect.w, player_origin.y
		};
		SDL_RenderTextureRotated(
			renderer, TEX_SOLAR_PANELS.tex, NULL, &solar_dest, self->tf.rot,
			&solar_origin, SDL_FLIP_NONE
		);
	}
	if(self->modules & (1 << PLAYERMODULE_ANTENNA)) {
		SDL_FRect antenna_dest = {
			player_rect.x, player_rect.y - player_rect.h, player_rect.w,
			player_rect.h * 2
		};
		SDL_FPoint antenna_origin = {
			player_origin.x, player_origin.y + player_rect.h
		};
		SDL_RenderTextureRotated(
			renderer, TEX_ANTENNA.tex, NULL, &antenna_dest, self->tf.rot,
			&antenna_origin, SDL_FLIP_NONE
		);
	}
	if(self->modules & (1 << PLAYERMODULE_CLAW)) {
		SDL_FRect claw_dest = {
			player_rect.x, player_rect.y - player_rect.h, player_rect.w,
			player_rect.h * 2
		};
		SDL_FPoint claw_origin = {
			player_origin.x, player_origin.y + player_rect.h
		};
		SDL_RenderTextureRotated(
			renderer, TEX_CLAW.tex, NULL, &claw_dest, self->tf.rot,
			&claw_origin, SDL_FLIP_NONE
		);
	}

	// Draw lateral movement guides
	if(Eng_get_key_down(KEY_LALT)) {
		const SDL_FPoint bow =
			FPoint_add(FPoint_force(75, self->tf.rot), player_ctr);
		const SDL_FPoint stern = FPoint_add(
			FPoint_force(75, PROPER_MOD((int) self->tf.rot + 180, 360)),
			player_ctr
		);
		const SDL_FPoint port = FPoint_add(
			FPoint_force(75, PROPER_MOD((int) self->tf.rot - 90, 360)),
			player_ctr
		);
		const SDL_FPoint starboard = FPoint_add(
			FPoint_force(75, PROPER_MOD((int) self->tf.rot + 90, 360)),
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
	float vector_strength = Vec2f_length(self->vel);
	if(vector_strength > 2) {
		SDL_FPoint prog_pos = FPoint_add(
			player_ctr, FPoint_scale(Vec2f_to_FPoint(self->vel), 10)
		);
		SDL_FPoint retro_pos = FPoint_add(
			player_ctr,
			FPoint_scale(Vec2f_to_FPoint(Vec2f_invert(self->vel)), 10)
		);
		SDL_FRect prog_dest  = {prog_pos.x - 12.5, prog_pos.y - 12.5, 25, 25};
		SDL_FRect retro_dest = {retro_pos.x - 12.5, retro_pos.y - 12.5, 25, 25};

		SDL_SetTextureAlphaMod(
			TEX_PROGRADE.tex, CLAMP(0, 255, vector_strength * 10)
		);
		SDL_SetTextureAlphaMod(
			TEX_RETROGRADE.tex, CLAMP(0, 255, vector_strength * 10)
		);
		SDL_RenderTexture(renderer, TEX_PROGRADE.tex, NULL, &prog_dest);
		SDL_RenderTexture(renderer, TEX_RETROGRADE.tex, NULL, &retro_dest);

		thickLineRGBA(
			renderer, player_ctr.x, player_ctr.y, prog_pos.x, prog_pos.y, 5,
			148, 222, 10, CLAMP(0, 255, vector_strength * 10)
		);
		thickLineRGBA(
			renderer, player_ctr.x, player_ctr.y, retro_pos.x, retro_pos.y, 5,
			210, 219, 39, CLAMP(0, 255, vector_strength * 10)
		);
	}
}

void Entity_player_collide(void* data, void* other, uint32_t typeof_other) {
	(void) other;
	(void) typeof_other;
	struct Entity_player* self = data;

	SDL_Log("Player hit something!");
	self->hit_something = true;
}

#endif
