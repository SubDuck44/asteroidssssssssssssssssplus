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
	ColRect   hitbox;
	bool      hit_something;
};
// -----------------------------------------------------------------------------
Result GameObject_asteroid_create(Vector2l position);
Result GameObject_asteroid_update(void* data, uint32_t index_of_self);
void   GameObject_asteroid_collide(
	  void* self, void* other, uint32_t typeof_other
  );

#if __INCLUDE_LEVEL__ == 0 /////////////////////////////////////////////////////

#include <SDL3_gfx/SDL3_gfxPrimitives.h>
#include <endian.h>
#include <stdio.h>

#include "../res.c"

Result GameObject_asteroid_create(Vector2l position) {
	struct GameObject_Asteroid self = {
		.ang_vel = (SDL_randf() * 4) * -(SDL_randf()),
		.vel     = Vec2f_force(3.0, SDL_randf() * 360),
	};
	self.tf = (Transform) {
		.pos  = position,
		.size = {100, 100},
		.ctr  = {50, 50},
		.rot  = SDL_randf() * 360,
	};

	struct GameObject_Asteroid* new = NULL;

	ASSERT_PREDICATE(Eng_create_object(
						 &self, (void*) &new,
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
	Eng_make_hitbox(
		&new->hitbox, new, GameObject_asteroid_collide, GAMEOBJECT_ASTEROID,
		self.tf.pos.x, self.tf.pos.y, self.tf.size.x, self.tf.size.y
	);
	return true;
}

void GameObject_asteroid_collide(
	void* data, void* other, uint32_t typeof_other
) {
	(void) other;
	(void) typeof_other;
	struct GameObject_Asteroid* self = data;

	SDL_Log("I hit something!");
	self->hit_something = true;
}

Result GameObject_asteroid_update(void* data, uint32_t index_of_self) {
	(void) index_of_self;
	struct GameObject_Asteroid* self = data;

	// Update
	double deltatime = Eng_get_deltatime_factor();

	self->tf.pos = Vec2l_add_Vec2f(
		self->tf.pos, Vec2f_scale(self->vel, DEFAULT_FIXED_POINT)
	);
	self->tf.rot += self->ang_vel * deltatime;

	Eng_set_hitbox(&self->hitbox, self->tf.pos);
	Eng_update_hitbox(&self->hitbox);
	Eng_draw_hitbox(&self->hitbox, self->hit_something);
	self->hit_something = false;

	SDL_FRect dest = Cam_transform_rect(&self->tf, &Eng_camera, NULL);

	// Draw
	SDL_RenderTextureRotated(
		renderer, TEX_ASTEROID.tex, NULL, &dest, self->tf.rot, NULL,
		SDL_FLIP_NONE
	);

	return true;
}

#endif
