#include "object.c"

FIELDS({
	uint8_t modules; //
})

#define FORCE_FWD (0.25 / 16)
#define FORCE_RCS (0.1 / 16)
#define FORCE_ROT (0.1 / 16)

enum {
	MOD_SOLAR,
	MOD_ANTENNA,
	MOD_CLAW,
};

#define MOD(x) (1 << MOD_##x)

void _playerCreate(Player* self) {
	*self = (Player) {
		.pos     = cam2wld(v2d(WINDOW_WIDTH / 2.0, WINDOW_HEIGHT / 2.0)),
		.rot     = 0,
		.scl     = 2,
		.modules = MOD(SOLAR) | MOD(ANTENNA),
	};

	self->hit.hw = (TEX_PLAYER.w / 2 - 9) * self->scl;
	self->hit.hh = (TEX_PLAYER.h / 2 + TEX_SOLAR_PANELS.h - 1) * self->scl;
}

void playerUpdate(Player* self) {
	if(DOWN(W)) accel(FORCE_FWD, 0);

	if(DOWN(I)) accel(FORCE_RCS, 0);
	if(DOWN(L)) accel(FORCE_RCS, 90);
	if(DOWN(K)) accel(FORCE_RCS, 180);
	if(DOWN(J)) accel(FORCE_RCS, 270);

	bool dampRot = true;

	if(DOWN(A)) {
		dampRot = false;
		self->rvl -= FORCE_ROT;
	}
	if(DOWN(D)) {
		dampRot = false;
		self->rvl += FORCE_ROT;
	}

	if(dampRot) self->rvl *= 0.97;

	if(DOWN(0)) {
		self->vel = v2d_2i64(v2d_mul(v2i64_2d(self->vel), v2dd(0.97)));
		/* self->vel = v2i64i64(0); */
		/* self->rvl = 0; */
	}

	/* if(PRES(1)) self->modules ^= MOD(SOLAR); */

	/* if(PRES(2)) { */
	/* 	self->modules ^= MOD(ANTENNA); */
	/* 	self->modules &= ~MOD(CLAW); */
	/* } */

	/* if(PRES(3)) { */
	/* 	self->modules ^= MOD(CLAW); */
	/* 	self->modules &= ~MOD(ANTENNA); */
	/* } */

	update(self);
	centerCamera(self->pos);

	PoolLoop(asteroidPool, {
		if(intersect(self->hit, it->hit)) asteroidDelete(it); //
	});
}

void playerRender(Player* self) {
	render(TEX_PLAYER);

	V2d pos = wld2cam(self->pos);
	V2d vel = v2d_div(v2d2i64(self->vel), v2dd(FIXED_POINT / 16.0));

	// draw modules
	if(self->modules & MOD(SOLAR)) render(TEX_SOLAR_PANELS);
	if(self->modules & MOD(ANTENNA))
		render(TEX_ANTENNA, .y = -TEX_PLAYER.h / 2);
	if(self->modules & MOD(CLAW)) render(TEX_CLAW, .y = -TEX_PLAYER.h / 2);

	// draw movement guides
	if(DOWN(LALT)) {
		V2d len = v2d(75, 0);

		V2d bow = v2d_add(pos, rotate(len, 0));   // bow, front
		V2d stb = v2d_add(pos, rotate(len, 90));  // starboard, right
		V2d str = v2d_add(pos, rotate(len, 180)); // stern, back
		V2d prt = v2d_add(pos, rotate(len, 270)); // port, left

		thickLineRGBA(renderer, V2d_Arg(pos), V2d_Arg(bow), 3, PROG + 255);
		thickLineRGBA(renderer, V2d_Arg(pos), V2d_Arg(stb), 3, GRN);
		thickLineRGBA(renderer, V2d_Arg(pos), V2d_Arg(str), 3, RETR + 255);
		thickLineRGBA(renderer, V2d_Arg(pos), V2d_Arg(prt), 3, RED);
	}

	// draw velocity vector
	double mag = v2d_len(vel);
	if(mag > 2) {
		V2d prog = v2d_add(pos, v2d_mul(vel, v2dd(10 * cam.scl)));
		V2d retr = v2d_sub(pos, v2d_mul(vel, v2dd(10 * cam.scl)));

		double size = 25;

		SDL_FRect progDst = {
			.x = prog.x - size / 2,
			.y = prog.y - size / 2,
			.w = size,
			.h = size,
		};

		SDL_FRect retrDst = {
			.x = retr.x - size / 2,
			.y = retr.y - size / 2,
			.w = size,
			.h = size,
		};

		double alpha = clampd(0, 255, mag * 10);

		thickLineRGBA(renderer, V2d_Arg(pos), V2d_Arg(prog), 5, PROG + alpha);
		thickLineRGBA(renderer, V2d_Arg(pos), V2d_Arg(retr), 5, RETR + alpha);

		SDL_SetTextureAlphaMod(TEX_PROGRADE.tex, alpha);
		SDL_SetTextureAlphaMod(TEX_RETROGRADE.tex, alpha);

		SDL_RenderTexture(renderer, TEX_PROGRADE.tex, NULL, &progDst);
		SDL_RenderTexture(renderer, TEX_RETROGRADE.tex, NULL, &retrDst);
	}

	hitboxRender(self);
}
