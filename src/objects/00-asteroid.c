#include "object.c"

static bool is_occupied(Hitbox hit) {
	PoolLoop(asteroidPool, {
		if(intersect(hit, it->hit)) return true;
	});

	return false;
}

static V2i64 roll_spawn(void) {
	if(playerPool.used.len == 0) {
		printf("asteroid failed to find player, spawning at 0,0\n");
		return v2i64(0, 0);
	} else {
		const double distance = max(2500 * SDL_randf(), 500);
		const double rotation = SDL_rand(360);

		return v2i64_add(
			playerPool.used.ptr[0].pos, //
			v2i642d(rotate(
				v2d(distance * FIXED_POINT, 0), //
				rotation                        //
			))
		);
	}
}

void _asteroidCreate(Asteroid* self) {
#define f (1.0 / 16)
#define v (SDL_randf() * f - f / 2) * FIXED_POINT

	*self = (Asteroid) {
		.vel = v2i64(v, v),
		.rot = SDL_rand(360),
		.rvl = (SDL_randf() * 2 - 1) * (f / 3),
		.scl = SDL_randf() * 2 + 5,
		.hit = (Hitbox) {
			.hw = TEX_ASTEROID.w / 2 * 0.8,
			.hh = TEX_ASTEROID.h / 2 * 0.8,
		},
	};

	do {
		self->pos = roll_spawn();
		hitboxUpdate(self);
	} while(is_occupied(self->hit));
}

void asteroidUpdate(Asteroid* self) {
	update(self);
}

void asteroidRender(Asteroid* self) {
	render(TEX_ASTEROID);
}
