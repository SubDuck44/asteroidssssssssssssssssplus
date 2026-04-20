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
	bool     alive;
	Vector2l pos;
	double   rot;
	Vector2f vel;
	float    ang_vel;
	float    force_main_thruster;
	float    force_rcs_thrusters;
	float    force_rot;
	ColRect* hitbox;
	uint8_t  modules;
};
// -----------------------------------------------------------------------------
Error GameObject_player_create(void);
Error GameObject_player_update(void* data, uint32_t index_of_self);

// Asteroid
struct GameObject_Asteroid {
	Vector2l pos;
	float    rot;
	Vector2f vel;
	double   ang_vel;
	ColRect* hitbox;
};
// -----------------------------------------------------------------------------
Error GameObject_asteroid_create(struct GameObject_Asteroid* override);
Error GameObject_asteroid_update(void* data, uint32_t index_of_self);

// Game overlay
struct GameObject_GameOverlay {
	TTF_Text* toast;
};
// -----------------------------------------------------------------------------
Error GameObject_gameoverlay_create(void);
Error GameObject_gameoverlay_update(void* data, uint32_t index_of_self);

#if __INCLUDE_LEVEL__ == 0 /////////////////////////////////////////////////////

#include <SDL3_gfx/SDL3_gfxPrimitives.h>
#include <endian.h>
#include <stdio.h>

#include "res.c"

// Player ======================================================================
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
		.modules             = 3
	};
	struct GameObject_Player* new = NULL;
	Error failed                  = Eng_register_hitbox(
        self.pos, (Vector2f) {50, 50}, new, GAMEOBJECT_PLAYER, &self.hitbox,
        &Eng_std_collision_tree
    );
	ASSERT_PREDICATE(
		failed == true, return false;
		,
		CODE_SUCCESS "INFO: Successfully registered hitbox for GameObject "
					 "player" CODE_END,
		CODE_ERROR
		"FATAL: Failed to register hitbox for GameObject player" CODE_END
	);
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
	return true;
}

Error GameObject_player_update(void* data, uint32_t index_of_self) {
	(void) index_of_self;
	struct GameObject_Player* self = data;

	// TODO Fix float inaccuracy in player movement controller
	double delta_time  = Eng_get_deltatime_factor();
	double thrust_main = self->force_main_thruster * delta_time;
	double thrust_rcs  = self->force_rcs_thrusters * delta_time;
	double force_rot   = self->force_rot * Eng_get_deltatime_factor();
	if(Eng_get_key_down(KEY_A)) self->ang_vel -= force_rot;
	if(Eng_get_key_down(KEY_D)) self->ang_vel += force_rot;
	if(Eng_get_key_down(KEY_W)) {
		self->vel = Vec2f_add(Vec2f_force(thrust_main, self->rot), self->vel);
	}
	if(Eng_get_key_down(KEY_I))
		self->vel = Vec2f_add(Vec2f_force(thrust_rcs, self->rot), self->vel);
	if(Eng_get_key_down(KEY_J))
		self->vel = Vec2f_add(
			Vec2f_force(thrust_rcs, WRAP_COMPASS((int) self->rot - 90)),
			self->vel
		);

	if(Eng_get_key_down(KEY_K))
		self->vel = Vec2f_add(
			Vec2f_force(thrust_rcs, WRAP_COMPASS((int) self->rot + 180)),
			self->vel
		);
	if(Eng_get_key_down(KEY_L))
		self->vel = Vec2f_add(
			Vec2f_force(thrust_rcs, WRAP_COMPASS((int) self->rot + 90)),
			self->vel
		);
	if(Eng_get_key_down(KEY_MOUSE_LEFT)) {
		self->rot = FPoint_angle_to(
			Cam_world_to_screen(self->pos, &Eng_std_camera), Eng_mouse_pos
		);
	}
	if(Eng_get_key_pressed(KEY_1)) self->modules ^= (1 << PLAYERMODULE_SOLAR);
	if(Eng_get_key_pressed(KEY_2) &&
	   (self->modules ^ (1 << PLAYERMODULE_CLAW)) > self->modules)
		self->modules ^= (1 << PLAYERMODULE_ANTENNA);
	if(Eng_get_key_pressed(KEY_3) &&
	   (self->modules ^ (1 << PLAYERMODULE_ANTENNA)) > self->modules)
		self->modules ^= (1 << PLAYERMODULE_CLAW);

	self->pos =
		Vec2l_add_Vec2f(self->pos, Vec2f_scale(self->vel, DEFAULT_FIXED_POINT));
	self->rot             = WRAP_COMPASS((int) (self->rot + self->ang_vel));
	Eng_std_camera.target = self->pos;

	SDL_FRect  player_rect   = (SDL_FRect) {0, 0, 50, 50};
	SDL_FPoint player_origin = {25, 25};
	SDL_FPoint player_ctr    = {0};
	Cam_transform(
		&self->pos, &player_ctr, &player_rect, &player_origin, &Eng_std_camera
	);

	if(Eng_update_hitbox(self->hitbox, &self->pos, NULL) == false) return false;

	// Draw player
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	SDL_RenderTextureRotated(
		renderer, TEX_PLAYER.tex, NULL, &player_rect, self->rot, &player_origin,
		SDL_FLIP_NONE
	);

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
			renderer, TEX_SOLAR_PANELS.tex, NULL, &solar_dest, self->rot,
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
			renderer, TEX_ANTENNA.tex, NULL, &antenna_dest, self->rot,
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
			renderer, TEX_CLAW.tex, NULL, &claw_dest, self->rot, &claw_origin,
			SDL_FLIP_NONE
		);
	}

	// Draw lateral movement guides
	if(Eng_get_key_down(KEY_LALT)) {
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
	float vector_strength = Vec2f_length(self->vel);
	if(vector_strength > 2) {
		Vector2l prog_world_pos = Vec2l_add_Vec2f(
			self->pos,
			Vec2f_scale(
				self->vel, 10 * Eng_std_camera.zoom * DEFAULT_FIXED_POINT
			)
		);
		Vector2l retro_world_pos = Vec2l_add_Vec2f(
			self->pos,
			Vec2f_scale(
				self->vel, -10 * Eng_std_camera.zoom * DEFAULT_FIXED_POINT
			)
		);
		SDL_FPoint prog_pos     = {0};
		SDL_FPoint retro_pos    = {0};
		SDL_FRect  prog_dest    = {0, 0, 25, 25};
		SDL_FRect  retro_dest   = {0, 0, 25, 25};
		SDL_FPoint prog_origin  = {12.5, 12.5};
		SDL_FPoint retro_origin = {12.5, 12.5};

		Cam_transform(
			&prog_world_pos, &prog_pos, &prog_dest, &prog_origin,
			&Eng_std_camera
		);
		Cam_transform(
			&retro_world_pos, &retro_pos, &retro_dest, &retro_origin,
			&Eng_std_camera
		);

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
		float rot = 0;
		self      = (struct GameObject_Asteroid) {
				 .ang_vel = 4.0f,
				 .pos =
                Vec2l_add_Vec2f(Eng_std_camera.target, (Vector2f) {0.0, -50.0}),
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
		failed, return false;
		,
		CODE_SUCCESS "INFO: Successfully registered hitbox for GameObject "
					 "asteroid" CODE_END,
		CODE_ERROR
		"FATAL: Failed to register hitbox for GameObject asteroid" CODE_END
	);
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

	self->pos = Vec2l_add_Vec2f(
		self->pos, self->vel // TODO switch out for new int based system
	);
	self->rot += self->ang_vel * deltatime;

	ColInfo col = Eng_get_collision(self->hitbox, &Eng_std_collision_tree);
	if(col.collided && col.typeof_owner == GAMEOBJECT_ASTEROID) {
		self->vel = Vec2f_rotate(
			self->vel,
			Vec2f_angle_to(
				FPoint_to_Vec2f(
					Cam_world_to_screen(col.collider->pos, &Eng_std_camera)
				),
				FPoint_to_Vec2f(Cam_world_to_screen(self->pos, &Eng_std_camera))
			)
		);
	}

	SDL_FPoint screen_pos = {0};
	SDL_FRect  dest       = {0, 0, 100, 100};
	SDL_FPoint origin     = {50.0f, 50.0f};

	Cam_transform(&self->pos, &screen_pos, &dest, &origin, &Eng_std_camera);

	Vector2f size = (Vector2f) {100, 100};
	Eng_update_hitbox(self->hitbox, &self->pos, &size);

	// Draw
	SDL_RenderTextureRotated(
		renderer, TEX_ASTEROID.tex, NULL, &dest, self->rot, &origin,
		SDL_FLIP_NONE
	);

	return true;
}

// Game overlay ================================================================
Error GameObject_gameoverlay_create(void) {
	struct GameObject_GameOverlay data = {
		.toast = NULL,
	};

	struct GameObject_GameOverlay* new = NULL;

	ASSERT_PREDICATE((data.toast =
	                      TTF_CreateText(Eng_text_engine, Eng_font, "none", 0)),
	                 return false;
	                 ,
	                 CODE_SUCCESS "INFO: Successfully created text object for "
	                              "GameObject_GameOverlay" CODE_END,
	                 CODE_ERROR "FATAL: Failed to create text object for "
	                            "GameObject_GameOverlay" CODE_END);

	TTF_SetTextWrapWidth(data.toast, 160);

	ASSERT_PREDICATE(
		Eng_create_object(
			&data, (void**) &new, sizeof(Toast), GAMEOBJECT_GAMEOVERLAY
		),
		TTF_DestroyText(data.toast);
		return false;
		,
		CODE_SUCCESS
		"INFO: Successfully created GameObject gameoverlay" CODE_END,
		CODE_ERROR "FATAL: Failed to create GameObject game overlay" CODE_END
	);

	return true;
}

Error GameObject_gameoverlay_update(void* data, uint32_t index_of_self) {
	(void) index_of_self;
	struct GameObject_GameOverlay* self = data;

	if(Eng_toast_queue.len > 0) {
		const float width   = 200;
		const float height  = 100;
		SDL_FRect   draw_at = {
            ((float) Eng_std_camera.screensize.x / 2) - (width / 2), 20, width,
            height
        };

		for(size_t i = 0; i < Eng_toast_queue.len; i++) {
			SDL_Color color;
			switch(Eng_toast_queue.arr[i].type) {
			case TOAST_WARN:
				color = (SDL_Color) {250, 189, 47, 255};
				break;
			case TOAST_CRITICAL:
				color = (SDL_Color) {204, 36, 29, 255};
				break;
			default:
				color = (SDL_Color) {69, 133, 136, 255};
				break;
			}
			SDL_SetRenderDrawColor(
				renderer, color.r, color.g, color.b, color.a
			);
			SDL_RenderFillRect(renderer, &draw_at);
			TTF_SetTextString(
				self->toast, Eng_toast_queue.arr[i].content, Eng_toast_queue.len
			);
			TTF_SetFontWrapAlignment(Eng_font, TTF_HORIZONTAL_ALIGN_CENTER);
			TTF_DrawRendererText(self->toast, draw_at.x + 20, draw_at.y + 20);
			draw_at.y += height + 20;

			if(Eng_toast_queue.arr[i].timestamp + 5000 < SDL_GetTicks()) {
				Eng_pop_toast(NULL);
			}
		}
	}

	return true;
}

#endif
