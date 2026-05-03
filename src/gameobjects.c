#pragma once

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#if __INCLUDE_LEVEL__ == 0 /////////////////////////////////////////////////////
#include <SDL3/SDL.h>

#include "engine.c"
#include "utils.c"
#endif

// Types
enum GameObject_Types : uint32_t {
	GAMEOBJECT_NONE,
	GAMEOBJECT_PLAYER,
	GAMEOBJECT_ASTEROID,
	GAMEOBJECT_FPS_DISPLAY,
	GAMEOBJECT_GAMEOVERLAY,
	GAMEOBJECT_NUM
};

// Player
enum PlayermModules : uint8_t {
	PLAYERMODULE_SOLAR,
	PLAYERMODULE_ANTENNA,
	PLAYERMODULE_CLAW,
};
// -----------------------------------------------------------------------------
struct GameObject_Player {
	bool      alive;
	Transform tf;
	Vector2f  vel;
	float     ang_vel;
	float     force_main_thruster;
	float     force_rcs_thrusters;
	float     force_rot;
	ColRect   hitbox;
	uint8_t   modules;
};
// -----------------------------------------------------------------------------
Error GameObject_player_create(void);
Error GameObject_player_update(void* data, uint32_t index_of_self);

// Asteroid
struct GameObject_Asteroid {
	Transform tf;
	Vector2f  vel;
	double    ang_vel;
	ColRect*  hitbox;
};
// -----------------------------------------------------------------------------
Error GameObject_asteroid_create(struct GameObject_Asteroid* override);
Error GameObject_asteroid_update(void* data, uint32_t index_of_self);

#if __INCLUDE_LEVEL__ == 0 /////////////////////////////////////////////////////

#include <SDL3_gfx/SDL3_gfxPrimitives.h>
#include <endian.h>
#include <stdio.h>

#include "res.c"

// Player ======================================================================
Error GameObject_player_create(void) {
	struct GameObject_Player self = {
		.alive               = true,
		.vel                 = {0},
		.ang_vel             = 0.0,
		.force_rot           = 0.2,
		.force_main_thruster = 0.25,
		.force_rcs_thrusters = 0.1,
		.modules             = 3
	};
	self.tf = (Transform) {
		.pos  = (Vector2l) {0, 0},
		.size = (SDL_FPoint) {100, 100},
		.ctr  = (SDL_FPoint) {50, 50},
		.rot  = 0,
	};
	struct GameObject_Player* new = NULL;

	ASSERT_PREDICATE(
		Eng_create_object(
			&self, (void*) &new, sizeof(struct GameObject_Player),
			GAMEOBJECT_PLAYER
		),
		return false;
		, CODE_SUCCESS "INFO: Successfully created GameObject: player" CODE_END,
		CODE_ERROR "FATAL: Failed to create GameObject player" CODE_END
	);
	ASSERT_PREDICATE(
		Eng_hook_update(GameObject_player_update, new), return false;
		,
		CODE_SUCCESS "INFO: Successfully hooked update callback for GameObject "
					 "player" CODE_END,
		CODE_ERROR "FATAL: Failed to hook update callback for GameObject "
				   "player" CODE_END
	);

	Eng_create_hitbox(&new->hitbox, &new->tf);

	return true;
}

Error GameObject_player_update(void* data, uint32_t index_of_self) {
	(void) index_of_self;
	struct GameObject_Player* self = data;

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
			Cam_world_to_screen(self->tf.pos, &Eng_std_camera), Eng_mouse_pos
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
	Vector2l new_pos = Vec2l_add_Vec2f(
		self->tf.pos, Vec2f_scale(self->vel, DEFAULT_FIXED_POINT)
	);
	SET_TRANS_POS_BY_CTR(self->tf, new_pos);
	self->tf.rot          = WRAP_COMPASS((int) (self->tf.rot + self->ang_vel));
	Eng_std_camera.target = self->tf.pos;

	// Set hitbox
	Eng_set_hitbox_pos(&self->hitbox, self->tf.pos);

	SDL_FPoint player_origin = {0, 0};
	SDL_FRect  player_rect =
		Cam_transform_rect(&self->tf, &Eng_std_camera, NULL);
	SDL_FPoint player_ctr = FPoint_add(
		Cam_world_to_screen(self->tf.pos, &Eng_std_camera),
		(SDL_FPoint) {(int32_t) (self->tf.size.x / 2),
	                  (int32_t) (self->tf.size.y / 2)}
	);

	// Draw player
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	SDL_RenderTextureRotated(
		renderer, TEX_PLAYER.tex, NULL, &player_rect, self->tf.rot,
		&player_origin, SDL_FLIP_NONE
	);

	// Draw hitbox
	Eng_draw_hitbox(&self->hitbox);

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
		SDL_FPoint prog_pos =
			FPoint_add(player_ctr, Vec2f_to_FPoint(self->vel));
		SDL_FPoint retro_pos =
			FPoint_add(player_ctr, Vec2f_to_FPoint(Vec2f_invert(self->vel)));
		SDL_FRect prog_dest  = {prog_pos.x - 12.5, prog_pos.y - 12.5, 25, 25};
		SDL_FRect retro_dest = {retro_pos.x - 12.5, retro_pos.y - 12.5, 25, 25};

		SDL_SetTextureAlphaMod(
			TEX_PROGRADE.tex, clampi(0, 255, vector_strength * 10)
		);
		SDL_SetTextureAlphaMod(
			TEX_RETROGRADE.tex, clampi(0, 255, vector_strength * 10)
		);
		SDL_RenderTexture(renderer, TEX_PROGRADE.tex, NULL, &prog_dest);
		SDL_RenderTexture(renderer, TEX_RETROGRADE.tex, NULL, &retro_dest);

		thickLineRGBA(
			renderer, player_ctr.x, player_ctr.y, prog_pos.x, prog_pos.y, 5,
			148, 222, 10, clampi(0, 255, vector_strength * 10)
		);
		thickLineRGBA(
			renderer, player_ctr.x, player_ctr.y, retro_pos.x, retro_pos.y, 5,
			210, 219, 39, clampi(0, 255, vector_strength * 10)
		);
	}

	return true;
}

// Asteroid ====================================================================
Error GameObject_asteroid_create(struct GameObject_Asteroid* override) {
	struct GameObject_Asteroid self;
	if(!override) {
		self = (struct GameObject_Asteroid) {
			.ang_vel = 4.0f, .vel = Vec2f_force(3.0, SDL_randf() * 360)
		};
		self.tf = (Transform) {
			.pos  = {0, 0},
			.size = {100, 100},
			.ctr  = {50, 50},
			.rot  = 0,
		};
	}

	struct GameObject_Asteroid* new = NULL;

	ASSERT_PREDICATE(Eng_create_object(
						 (override) ? override : &self, (void*) &new,
						 sizeof(struct GameObject_Asteroid), GAMEOBJECT_ASTEROID
					 ),
	                 return false;
	                 ,
	                 CODE_SUCCESS
	                 "INFO: Successfully created GameObject asteroid" CODE_END,
	                 CODE_ERROR
	                 "FATAL: Failed to create GameObject player" CODE_END);

	ASSERT_PREDICATE(
		Eng_hook_update(GameObject_asteroid_update, new), return false;
		,
		CODE_SUCCESS "INFO: Successfully hooked update callback for GameObject "
					 "asteroid" CODE_END,
		CODE_ERROR "FATAL: Failed to hook update callback for GameObject "
				   "asteroid" CODE_END
	);
	return true;
}

Error GameObject_asteroid_update(void* data, uint32_t index_of_self) {
	(void) index_of_self;
	struct GameObject_Asteroid* self = data;

	// Update
	double deltatime = Eng_get_deltatime_factor();

	Vector2l new_pos = Vec2l_add_Vec2f(
		self->tf.pos, self->vel // TODO switch out for new int based system
	);
	SET_TRANS_POS_BY_CTR(self->tf, new_pos);
	self->tf.rot += self->ang_vel * deltatime;

	SDL_FRect dest = Cam_transform_rect(&self->tf, &Eng_std_camera, NULL);

	// Draw
	SDL_RenderTextureRotated(
		renderer, TEX_ASTEROID.tex, NULL, &dest, self->tf.rot, NULL,
		SDL_FLIP_NONE
	);

	return true;
}

#endif
