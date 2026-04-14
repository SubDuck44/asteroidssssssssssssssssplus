#pragma once

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#if __INCLUDE_LEVEL__ == 0
#include <SDL3/SDL.h>

#include "engine.c"
#include "utils.c"
#endif

enum GameObject_Types : uint32_t {
	GAMEOBJECT_PLAYER,
	GAMEOBJECT_ASTEROID,
	GAMEOBJECT_FPS_DISPLAY,
	GAMEOBJECT_NUM
};

// Declarations
struct GameObject_Player {
	bool     alive;
	Position pos;
	double   rot;
	Vector2f vel;
	float    ang_vel;
	float    force_main_thruster;
	float    force_rcs_thrusters;
	float    force_rot;
	ColRect* hitbox;
};
Error GameObject_player_update(void* data, uint32_t index_of_self);
Error GameObject_player_create(void);

struct GameObject_Asteroid {
	Position pos;
	float    rot;
	Vector2f vel;
	double   ang_vel;
	ColRect* hitbox;
};
Error GameObject_asteroid_create(struct GameObject_Asteroid* override);
Error GameObject_asteroid_update(void* data, uint32_t index_of_self);

struct GaneObject_FPS_Display {
	TTF_Text*  display;
	SDL_FPoint pos;
};
Error GameObject_fps_display_update(void* data, uint32_t index_of_self);
Error GameObject_fps_display_create(SDL_FPoint pos);

#if __INCLUDE_LEVEL__ == 0 /////////////////////////////////////////////////////

#include <SDL3_gfx/SDL3_gfxPrimitives.h>
#include <endian.h>
#include <stdio.h>

#include "res.c"

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
		self->vel = Vec2f_add(Vec2f_force(thrust_main, self->rot), self->vel);
	}
	if(Eng_get_key_pressed(KEY_I))
		self->vel = Vec2f_add(Vec2f_force(thrust_rcs, self->rot), self->vel);
	if(Eng_get_key_pressed(KEY_J))
		self->vel = Vec2f_add(
			Vec2f_force(thrust_rcs, WRAP_COMPASS((int) self->rot - 90)),
			self->vel
		);

	if(Eng_get_key_pressed(KEY_K))
		self->vel = Vec2f_add(
			Vec2f_force(thrust_rcs, WRAP_COMPASS((int) self->rot + 180)),
			self->vel
		);
	if(Eng_get_key_pressed(KEY_L))
		self->vel = Vec2f_add(
			Vec2f_force(thrust_rcs, WRAP_COMPASS((int) self->rot + 90)),
			self->vel
		);
	if(Eng_get_key_pressed(KEY_MOUSE_LEFT)) {
		self->rot = FPoint_angle_to(
			Pos_world_to_screen(self->pos, &Eng_std_camera), Eng_mouse_pos
		);
	}

	self->pos             = Pos_add_Vec2f(self->pos, self->vel);
	self->rot             = WRAP_COMPASS((int) (self->rot + self->ang_vel));
	Eng_std_camera.target = self->pos;

	SDL_FRect  player_rect   = (SDL_FRect) {0, 0, 50, 50};
	SDL_FPoint player_origin = {25, 25};
	SDL_FPoint player_ctr    = {0};
	Pos_cam_transform(
		&self->pos, &player_ctr, &player_rect, &player_origin, &Eng_std_camera
	);

	if(Eng_update_hitbox(self->hitbox, &self->pos, NULL) == ERR_FATAL)
		return ERR_FATAL;

	// Draw player
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	SDL_RenderTextureRotated(
		renderer, TEX_PLAYER.tex, NULL, &player_rect, self->rot, &player_origin,
		SDL_FLIP_NONE
	);

	// Draw lateral movement guides
	if(Eng_get_key_pressed(KEY_LALT)) {
		const SDL_FPoint bow =
			FPoint_add(FPoint_force(75, self->rot), player_ctr);
		const SDL_FPoint stern = FPoint_add(
			FPoint_force(75, PROPER_MOD((int) self->rot + 180, 360)), player_ctr
		);
		const SDL_FPoint port = FPoint_add(
			FPoint_force(75, PROPER_MOD((int) self->rot - 90, 360)), player_ctr
		);
		const SDL_FPoint starboard = FPoint_add(
			FPoint_force(75, PROPER_MOD((int) self->rot + 90, 360)), player_ctr
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
	Position prog_world_pos = Pos_add_Vec2f(
		self->pos, Vec2f_scale(self->vel, 10 * Eng_std_camera.zoom)
	);
	Position retro_world_pos = Pos_add_Vec2f(
		self->pos, Vec2f_scale(self->vel, -10 * Eng_std_camera.zoom)
	);
	SDL_FPoint prog_pos     = {0};
	SDL_FPoint retro_pos    = {0};
	SDL_FRect  prog_dest    = {0, 0, 25, 25};
	SDL_FRect  retro_dest   = {0, 0, 25, 25};
	SDL_FPoint prog_origin  = {12.5, 12.5};
	SDL_FPoint retro_origin = {12.5, 12.5};

	Pos_cam_transform(
		&prog_world_pos, &prog_pos, &prog_dest, &prog_origin, &Eng_std_camera
	);
	Pos_cam_transform(
		&retro_world_pos, &retro_pos, &retro_dest, &retro_origin,
		&Eng_std_camera
	);

	SDL_RenderTexture(renderer, TEX_PROGRADE.tex, NULL, &prog_dest);
	SDL_RenderTexture(renderer, TEX_RETROGRADE.tex, NULL, &retro_dest);

	thickLineColor(
		renderer, player_ctr.x, player_ctr.y, prog_pos.x, prog_pos.y, 3,
		htobe32(0x94DE0AFF)
	);
	thickLineColor(
		renderer, player_ctr.x, player_ctr.y, retro_pos.x, retro_pos.y, 3,
		htobe32(0xD2DB27FF)
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
	Error failed                  = Eng_register_hitbox(
        self.pos, (Vector2f) {50, 50}, new, GAMEOBJECT_PLAYER, &self.hitbox,
        &Eng_std_collision_tree
    );
	ASSERT_PREDICATE(
		failed == ERR_PASS, return ERR_FATAL;
		,
		CODE_SUCCESS
		"INFO: Successfully registered hitbox for GameObject player" CODE_END,
		CODE_ERROR
		"FATAL: Failed to register hitbox for GameObject player" CODE_END
	);
	ASSERT_PREDICATE(
		Eng_create_object(
			&self, (void*) &new, sizeof(struct GameObject_Player),
			GAMEOBJECT_PLAYER
		),
		return ERR_FATAL;
		, CODE_SUCCESS "INFO: Successfully created GameObject: player" CODE_END,
		CODE_ERROR "FATAL: Failed to create GameObject player" CODE_END
	);
	ASSERT_PREDICATE(
		Eng_hook_update(GameObject_player_update, new), return ERR_FATAL;
		,
		CODE_SUCCESS "INFO: Successfully hooked update callback for GameObject "
					 "player" CODE_END,
		CODE_ERROR
		"FATAL: Failed to hook update callback for GameObject player" CODE_END
	);
	return ERR_PASS;
}

Error GameObject_asteroid_create(struct GameObject_Asteroid* override) {
	struct GameObject_Asteroid self;
	if(!override) {
		float rot = 0;
		self      = (struct GameObject_Asteroid) {
				 .ang_vel = 4.0f,
				 .pos =
                Pos_add_Vec2f(Eng_std_camera.target, (Vector2f) {0.0, -50.0}),
				 .rot = rot,
				 .vel = Vec2f_force(3.0, SDL_randf() * 360)
        };
	}

	struct GameObject_Asteroid* new = NULL;
	Error failed                    = Eng_register_hitbox(
        self.pos, (Vector2f) {100, 100}, new, GAMEOBJECT_ASTEROID, &self.hitbox,
        &Eng_std_collision_tree
    );
	ASSERT_PREDICATE(
		failed, return ERR_FATAL;
		,
		CODE_SUCCESS
		"INFO: Successfully registered hitbox for GameObject asteroid" CODE_END,
		CODE_ERROR
		"FATAL: Failed to register hitbox for GameObject asteroid" CODE_END
	);
	ASSERT_PREDICATE(Eng_create_object(
						 (override) ? override : &self, (void*) &new,
						 sizeof(struct GameObject_Asteroid), GAMEOBJECT_ASTEROID
					 ),
	                 return ERR_FATAL;
	                 ,
	                 CODE_SUCCESS
	                 "INFO: Successfully created GameObject asteroid" CODE_END,
	                 CODE_ERROR
	                 "FATAL: Failed to create GameObject player" CODE_END);

	ASSERT_PREDICATE(
		Eng_hook_update(GameObject_asteroid_update, new), return ERR_FATAL;
		,
		CODE_SUCCESS "INFO: Successfully hooked update callback for GameObject "
					 "asteroid" CODE_END,
		CODE_ERROR
		"FATAL: Failed to hook update callback for GameObject asteroid" CODE_END
	);
	return ERR_PASS;
}

Error GameObject_asteroid_update(void* data, uint32_t index_of_self) {
	(void) index_of_self;
	struct GameObject_Asteroid* self = data;

	// Update
	double deltatime = Eng_get_deltatime_factor();

	self->pos = Pos_add_Vec2f(
		self->pos, self->vel // TODO switch out for new int based system
	);
	self->rot += self->ang_vel * deltatime;

	ColInfo col = Eng_get_collision(self->hitbox, &Eng_std_collision_tree);
	if(col.collided && col.typeof_owner == GAMEOBJECT_ASTEROID) {
		self->vel = Vec2f_rotate(
			self->vel,
			Vec2f_angle_to(
				FPoint_to_Vec2f(
					Pos_world_to_screen(col.collider->pos, &Eng_std_camera)
				),
				FPoint_to_Vec2f(Pos_world_to_screen(self->pos, &Eng_std_camera))
			)
		);
	}

	SDL_FPoint screen_pos = {0};
	SDL_FRect  dest       = {0, 0, 100, 100};
	SDL_FPoint origin     = {50.0f, 50.0f};

	Pos_cam_transform(&self->pos, &screen_pos, &dest, &origin, &Eng_std_camera);

	Vector2f size = (Vector2f) {100, 100};
	Eng_update_hitbox(self->hitbox, &self->pos, &size);

	// Draw
	SDL_RenderTextureRotated(
		renderer, TEX_ASTEROID.tex, NULL, &dest, self->rot, &origin,
		SDL_FLIP_NONE
	);

	return ERR_PASS;
}

Error GameObject_fps_display_update(void* data, uint32_t index_of_self) {
	(void) index_of_self;
	if(Eng_debug_vis) {
		struct GaneObject_FPS_Display* self = data;

		char fps_string[64] = {0};
		snprintf(fps_string, sizeof(fps_string), "FPS: %d", Eng_current_fps);
		TTF_SetTextString(self->display, fps_string, sizeof(fps_string));

		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		TTF_DrawRendererText(self->display, self->pos.x, self->pos.y);
	}
	return ERR_PASS;
}
Error GameObject_fps_display_create(SDL_FPoint pos) {
	struct GaneObject_FPS_Display self = {
		.display = nullptr,
		.pos     = pos,
	};
	ASSERT_PREDICATE_SDL(
		(self.display =
	         TTF_CreateText(Eng_text_engine, Eng_font, "hewo :3", 0)),
		return ERR_FATAL;
		,
		CODE_SUCCESS "INFO: Successfully created TTF_Text object for "
					 "GameObject fps_display" CODE_END,
		CODE_ERROR "FATAL: Failed to create TTF_Text object for GameObject "
				   "fps_display" CODE_END
	);
	struct GameObject_FPS_Display* new = NULL;
	ASSERT_PREDICATE(
		Eng_create_object(
			&self, (void*) &new, sizeof(struct GaneObject_FPS_Display),
			GAMEOBJECT_FPS_DISPLAY
		),
		return ERR_FATAL;
		,
		CODE_SUCCESS
		"INFO: Successfully created GameObject fps_display" CODE_END,
		CODE_ERROR "FATAL: Failed to create GameObject fps_display" CODE_END
	);
	ASSERT_PREDICATE(Eng_hook_update(GameObject_fps_display_update, new),
	                 return ERR_FATAL;
	                 ,
	                 CODE_SUCCESS "INFO: Successfully hooked update callback "
	                              "for GameObject fps_display" CODE_END,
	                 CODE_ERROR "FATAL: Failed to hook update callback for "
	                            "GameObject fps_display" CODE_END);
	return ERR_PASS;
}

#endif
