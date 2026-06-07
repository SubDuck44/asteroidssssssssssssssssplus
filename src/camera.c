#pragma once

#include <la.h>

#define FIXED_POINT 2147483648L // 1 << 31 but as a long literal (fuck C)

typedef struct {
	V2i64  pos;
	V2i64  ctr;
	V2d    siz;
	double scl;
} Camera;

extern Camera cam;

void centerCamera(V2i64 ctr);
void zoomCamera(double factor);

#define cam2wld(src)                                                           \
	v2i64(                                                                     \
		src.x* FIXED_POINT / cam.scl + cam.pos.x,                              \
		src.y * FIXED_POINT / cam.scl + cam.pos.y                              \
	)

#define wld2cam(src)                                                           \
	v2d((src.x - cam.pos.x) * cam.scl / FIXED_POINT,                           \
	    (src.y - cam.pos.y) * cam.scl / FIXED_POINT)

#if __INCLUDE_LEVEL__ == 0 /////////////////////////////////////////////////////

#include "utils.c"

Camera cam = {.scl = 1};

static void __attribute__((constructor)) init(void) {
	cam.siz = v2d(WINDOW_WIDTH, WINDOW_HEIGHT);
	centerCamera(v2i64i64(0));
}

static void recenter(void) {
	cam.pos.x = cam.ctr.x - cam.siz.x / 2 / cam.scl * FIXED_POINT;
	cam.pos.y = cam.ctr.y - cam.siz.y / 2 / cam.scl * FIXED_POINT;
}

void centerCamera(V2i64 ctr) {
	cam.ctr = ctr;
	recenter();
}

void zoomCamera(double factor) {
	cam.scl *= factor;
	recenter();
}

#endif
