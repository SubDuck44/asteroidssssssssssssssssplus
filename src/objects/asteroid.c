#pragma once

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#if __INCLUDE_LEVEL__ == 0 /////////////////////////////////////////////////////

#include "../engine.c"
#include "../utils.c"

#endif

struct Entity_asteroid {
	Transform tf;
	Vector2f  vel;
	double    ang_vel;
	ColRect   hitbox;
	bool      hit_something; // TODO: Move this to collision system
};
// -----------------------------------------------------------------------------
bool Entity_asteroid_create(Vector2l position);
void Entity_asteroid_update(struct Entity_asteroid* self, size_t index);
void Entity_asteroid_collide(void* self, void* other, uint32_t typeof_other);

#if __INCLUDE_LEVEL__ == 0 /////////////////////////////////////////////////////

#include <SDL3_gfx/SDL3_gfxPrimitives.h>
#include <endian.h>
#include <stdio.h>
#include <stdlib.h>

#include "../res.c"

bool Entity_asteroid_create(Vector2l position) {
	DynArrPush(
		&entities_asteroids, ((struct Entity_asteroid) {
								 .ang_vel = (SDL_randf() * 4) * -(SDL_randf()),
								 .vel     = Vec2f_force(3.0, SDL_randf() * 360),
								 .tf      = (Transform) {
										  .pos  = position,
										  .size = {100, 100},
										  .ctr  = {50, 50},
										  .rot  = SDL_randf() * 360,
                                 },
							 })
	);

	struct Entity_asteroid* self =
		&entities_asteroids.arr[entities_asteroids.len - 1];

	Eng_make_hitbox(
		&self->hitbox, self, Entity_asteroid_collide, ENTITY_ASTEROID,
		self->tf.pos.x, self->tf.pos.y, self->tf.size.x, self->tf.size.y
	);

	return true;
}

void Entity_asteroid_collide(void* data, void* other, uint32_t typeof_other) {
	(void) other;
	(void) typeof_other;
	struct Entity_asteroid* self = data;

	SDL_Log("I hit something!");
	self->hit_something = true;
}

void Entity_asteroid_update(struct Entity_asteroid* self, size_t index) {
	(void) index;

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
}

#endif
