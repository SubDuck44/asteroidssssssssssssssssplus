#include "object.c"

const uint64_t lifetime = 5L * BIL; // 3 seconds fuse timeout

FIELDS({
	uint64_t timestamp; //
})

void _bulletCreate(Bullet* self) {
	*self = (Bullet) {
		.scl       = 0.25,
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

			score += 1;

			V2i64  pos = it->pos;
			V2i64  vel = it->vel;
			double scl = it->scl * 0.5;

			// Split/die
			asteroidDelete(it);
			bulletDelete(self);

			if(scl > 0.50) {
				/* I AM GOING TO FUCKING KILL MYSELF
				  "Oh yeah, let me just use ints for the coords instead of the
				   industry standard thats there for a GOOD FUCKING REASON"
				   Its all the fault of the float gang */
				const double ugly_hack_to_save_me =
					v2d_len(v2d2i64(self->vel)) * 0.1;

				Asteroid* as1 = asteroidCreate();
				as1->pos      = pos;
				as1->scl      = scl;
				as1->vel = v2i64_imp(vel, ugly_hack_to_save_me, self->rot + 45);

				Asteroid* as2 = asteroidCreate();
				as2->pos      = pos;
				as2->scl      = scl;
				as2->vel = v2i64_imp(vel, ugly_hack_to_save_me, self->rot - 45);
			}

			break;
		}
	});
}

void bulletRender(Bullet* self) {
	render(TEX_PENETRATOR);
}
