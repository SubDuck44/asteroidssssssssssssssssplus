#include "object.c"

const uint64_t lifetime = 3L * BIL; // 3 seconds fuse timeout

FIELDS({
	uint64_t timestamp; //
})

void _bulletCreate(Bullet* self) {
	*self = (Bullet) {
		.scl       = 0.5,
		.timestamp = SDL_GetTicksNS(),
		.hit       = (Hitbox) {
				  .hw = TEX_PENETRATOR.h / 2.0,
				  .hh = 8,
        },
	};
}

void bulletUpdate(Bullet* self) {
	if(SDL_GetTicksNS() > self->timestamp + lifetime) {
		bulletDelete(self);
		return;
	}

	update(self);

	PoolLoop(asteroidPool, {
		if(intersect(self->hit, it->hit)) {

			// Split/die
			double new_scl = it->scl / 2;
			if(new_scl > 0.50) {
				Asteroid* as1 = asteroidCreate();
				Asteroid* as2 = asteroidCreate();

				as1->pos = as2->pos = it->pos;
				as1->scl = as2->scl = new_scl;

				as1->vel =
					v2i64_imp(it->vel, FIXED_POINT / 10.0, self->rot + 45);
				as2->vel =
					v2i64_imp(it->vel, FIXED_POINT / 10.0, self->rot - 45);
			}

			asteroidDelete(it);
			bulletDelete(self);

			break;
		}
	});
}

void bulletRender(Bullet* self) {
	render(TEX_PENETRATOR);
}
