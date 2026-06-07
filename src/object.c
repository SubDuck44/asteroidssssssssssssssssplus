#pragma once

#include "objects.c"

ObjID getOID(void* obj);

#define rotate(vector, angle)                                                  \
	m2d_mul_vec(m2d_rot((self->rot + angle) * DEG2RAD), vector)

#define accel(force, angle)                                                    \
	self->vel = v2i64_add(                                                     \
		self->vel, v2i642d(rotate(v2d(force * FIXED_POINT, 0), angle))         \
	)

#define update(obj) _update(&obj->pos, obj->vel, &obj->rot, obj->rvl)
void _update(V2i64* pos, V2i64 vel, double* rot, double rvl);

typedef struct {
	V2i64 pos; // for internal use

	double x, y;
	double rot;
	double scl;
} RenderArgs;

#define render(tex, ...)                                                       \
	_render(                                                                   \
		tex,                                                                   \
		(RenderArgs) {                                                         \
			.pos = self->pos,                                                  \
			.rot = self->rot,                                                  \
			.scl = self->scl,                                                  \
		},                                                                     \
		(RenderArgs) {__VA_ARGS__}                                             \
	);

void _render(Texture tex, RenderArgs obj, RenderArgs off);

#if __INCLUDE_LEVEL__ == 0 /////////////////////////////////////////////////////

ObjID getOID(void* obj) {
	return *(ObjID*) obj;
}

void _update(V2i64* pos, V2i64 vel, double* rot, double rvl) {
	*pos = v2i64_add(*pos, v2i64_mul(vel, v2i64i64(dtf / MIL)));
	*rot = mod(*rot + rvl * dtf / MIL, 360);
}

void _render(Texture tex, RenderArgs obj, RenderArgs off) {
	if(off.scl == 0) off.scl = 1;

	double rot = obj.rot + 90; // so that texture top is positive x
	V2d    scl = v2dd(obj.scl * cam.scl);

	// midpoint of the texture
	V2d mid = v2d_add(
		wld2cam(obj.pos),
		m2d_mul_vec(m2d_rot(rot * DEG2RAD), v2d_mul(v2d(off.x, off.y), scl))
	);

	// scaled lengths of the texture
	V2d len = v2d_mul(v2d_mul(v2d(tex.w, tex.h), scl), v2dd(off.scl));

	// half length = difference between midpoint and corners
	V2d dif = v2d_div(len, v2dd(2));

	// top left corner
	V2d top = v2d_sub(mid, dif);

	SDL_FRect dst = {
		.x = top.x,
		.y = top.y,
		.w = len.x,
		.h = len.y,
	};

	SDL_FPoint ctr = {
		.x = dif.x,
		.y = dif.y,
	};

	SDL_RenderTextureRotated(
		renderer,      //
		tex.tex,       //
		NULL,          //
		&dst,          //
		rot + off.rot, //
		&ctr,          //
		SDL_FLIP_NONE  //
	);
}

#endif
