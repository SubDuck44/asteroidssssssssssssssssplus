#include "object.c"

void _asteroidCreate(Asteroid* self) {
#define f (1.0 / 16)
#define v (SDL_randf() * f - f / 2) * FIXED_POINT

	*self = (Asteroid) {
		.pos = cam2wld(v2d(SDL_rand(WINDOW_WIDTH), SDL_rand(WINDOW_HEIGHT))),

		.vel = v2i64(v, v),

		.rot = SDL_rand(360),
		.rvl = (SDL_randf() * 2 - 1) * f,

		.scl = SDL_randf() * 1.5 + 0.5,
	};

	self->hit.hw = TEX_ASTEROID.w / 2 * self->scl * 0.8;
	self->hit.hh = TEX_ASTEROID.h / 2 * self->scl * 0.8;
}

void asteroidUpdate(Asteroid* self) {
	update(self);
}

void asteroidRender(Asteroid* self) {
	render(TEX_ASTEROID);
}
