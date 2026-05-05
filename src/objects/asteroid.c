#pragma once

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#if __INCLUDE_LEVEL__ == 0 /////////////////////////////////////////////////////

#include "../engine.c"
#include "../gameobjects.c"
#include "../utils.c"

#endif

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

#include "../res.c"

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
