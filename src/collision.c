#pragma once

#include "utils.c"

#define AXES 2
#define VERTICES (AXES * 2)

struct Hitbox {
	// TODO MVP collision system - only one centered rect
	double hw, hh;
	V2d    vertices[VERTICES]; // internal
};

typedef struct Hitbox Hitbox;

////////////////////////////////////////////////////////////////////////////////

bool intersect(Hitbox h1, Hitbox h2);

#define hitboxUpdate(obj) _hitboxUpdate(obj->pos, obj->rot, &obj->hit);
void _hitboxUpdate(V2i64 pos, double rot, Hitbox* hit);

#define hitboxRender(obj) _hitboxRender(obj->hit)
void _hitboxRender(Hitbox hit);

#if __INCLUDE_LEVEL__ == 0 /////////////////////////////////////////////////////

#include "camera.c"
#include <SDL3_gfx/SDL3_gfxPrimitives.h>

////////////////////////////////////////////////////////////////////////////////

typedef V2d Axis;

typedef struct {
	double min;
	double max;
} Proj;

static void getAxes(Hitbox hit, Axis axes[AXES]) {
	for(size_t i = 0; i < AXES; i++) {
		V2d p1  = hit.vertices[i];
		V2d p2  = hit.vertices[i + 1];
		axes[i] = v2d_sub(p1, p2);
	}
}

static Proj project(Hitbox hit, Axis axis) {
	double min = +INFINITY;
	double max = -INFINITY;

	for(size_t i = 0; i < VERTICES; i++) {
		double p = v2d_dot(axis, hit.vertices[i]);

		if(p < min) min = p;
		if(p > max) max = p;
	}

	return (Proj) {
		.min = min,
		.max = max,
	};
}

static bool overlap(Proj p1, Proj p2) {
	return (p1.min < p2.max && p1.max > p2.min) ||
	       (p2.min < p1.max && p2.max > p1.min);
}

bool intersect(Hitbox hit1, Hitbox hit2) {
	Axis axes[AXES * 2] = {0};
	getAxes(hit1, &axes[AXES * 0]);
	getAxes(hit2, &axes[AXES * 1]);

	for(size_t i = 0; i < ARRLEN(axes); i++) {
		Axis axis = axes[i];

		Proj p1 = project(hit1, axis);
		Proj p2 = project(hit2, axis);

		if(!overlap(p1, p2)) return false;
	}

	return true;
}

#define rotate(vector) m2d_mul_vec(m2d_rot((rot) * DEG2RAD), vector)

void _hitboxUpdate(V2i64 _pos, double rot, Hitbox* hit) {
	double hw = hit->hw * FIXED_POINT;
	double hh = hit->hh * FIXED_POINT;

	V2d pos = v2i64_2d(_pos);

	hit->vertices[0] = v2d_add(pos, rotate(v2d(+hw, +hh))); // bottom right
	hit->vertices[1] = v2d_add(pos, rotate(v2d(-hw, +hh))); // bottom left
	hit->vertices[2] = v2d_add(pos, rotate(v2d(-hw, -hh))); // top left
	hit->vertices[3] = v2d_add(pos, rotate(v2d(+hw, -hh))); // top right
}

void _hitboxRender(Hitbox hit) {
	V2d br = wld2cam(hit.vertices[0]);
	V2d bl = wld2cam(hit.vertices[1]);
	V2d tl = wld2cam(hit.vertices[2]);
	V2d tr = wld2cam(hit.vertices[3]);

	float xs[] = {tl.x, tr.x, br.x, bl.x};
	float ys[] = {tl.y, tr.y, br.y, bl.y};

	polygonRGBA(renderer, xs, ys, 4, RED);

	filledCircleRGBA(renderer, V2d_Arg(tl), 3, RED);
	filledCircleRGBA(renderer, V2d_Arg(tr), 3, GRN);
	filledCircleRGBA(renderer, V2d_Arg(br), 3, BLU);
	filledCircleRGBA(renderer, V2d_Arg(bl), 3, MAG);
}

#endif
