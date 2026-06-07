#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <err.h>
#include <la.h>
#include <stdlib.h>

////////////////////////////////////////////////////////////////////////////////

#define ARRLEN(x) (sizeof(x) / sizeof(*(x)))

#define BREAK __asm__("int3");

////////////////////////////////////////////////////////////////////////////////

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

#define RAD2DEG (180 / M_PI)
#define DEG2RAD (1 / RAD2DEG)

#define sind(x) (sin((x) * DEG2RAD))
#define cosd(x) (cos((x) * DEG2RAD))

V2d rotate(V2d vec, double rot);

double mod(double a, double b);

/** Applies a force with magnitude m and direction r to a vector base */
V2i64 v2i64_imp(V2i64 base, double m, double r);

////////////////////////////////////////////////////////////////////////////////

#define SDL_Die(msg) errx(1, msg ": %s", SDL_GetError())

#define WINDOW_TITLE "Asteroidssssssssssssssss+"
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

extern SDL_Window*   window;
extern SDL_Renderer* renderer;

#define FONT_SIZE 12

extern TTF_Font*       font;
extern TTF_TextEngine* textEngine;

#define MIL 1'000'000
#define BIL 1'000'000'000
#define FPS 60
#define DFT (BIL / FPS)

extern int64_t dtf; // delta frame
extern double  fps;

extern bool debugVisible;

////////////////////////////////////////////////////////////////////////////////

#define Color(color)                                                           \
	((color >> (8 * 3)) & 0xFF), ((color >> (8 * 2)) & 0xFF),                  \
		((color >> (8 * 1)) & 0xFF), ((color >> (8 * 0)) & 0xFF)

#define RED Color(0xFF0000FF)
#define GRN Color(0x00FF00FF)
#define BLU Color(0x0000FFFF)
#define YEL Color(0xFFFF00FF)
#define MAG Color(0xFF00FFFF)
#define CYA Color(0x00FFFFFF)

// add alpha as required
#define PROG Color(0xFFFFFF00)
#define RETR Color(0xD2DB2700)

// background color
#define GRAY Color(0x27242BFF)

////////////////////////////////////////////////////////////////////////////////

#define FAIL(str) ("[1;31m" str "[m")
#define OKAY(str) ("[1;32m" str "[m")
#define WARN(str) ("[1;33m" str "[m")

////////////////////////////////////////////////////////////////////////////////

#define ArrayN(t, n)                                                           \
	typedef struct {                                                           \
		t*     ptr;                                                            \
		size_t len;                                                            \
		size_t cap;                                                            \
	} n

#define Array(t) ArrayN(t, t##s)

#define ArrayExtend(arr)                                                       \
	((arr).len >= (arr).cap                                                    \
	     ? (arr).ptr = reallocarray(                                           \
			   (arr).ptr, ((arr).cap = max((arr).cap << 1, 16)),               \
			   sizeof(*(arr).ptr)                                              \
		   )                                                                   \
	     : 0)

#define ArrayAdd(arr, x) (ArrayExtend(arr), (arr).ptr[(arr).len++] = (x))

#define ArrayNew(arr)                                                          \
	(ArrayExtend(arr), memset(&(arr).ptr[(arr).len++], 0, sizeof(*(arr).ptr)))

#define ArrayLoop(arr, body) ArrayLoopN(arr, it, body)

#define ArrayLoopN(arr, it, body)                                              \
	for(size_t i = 0; i < (arr).len; i++) {                                    \
		typeof((arr).ptr[0])* it = &(arr).ptr[i];                              \
		body                                                                   \
	}

#define ArrayFindI(arr, result, pred)                                          \
	ArrayLoopN(arr, it, {                                                      \
		if(pred) {                                                             \
			result = it;                                                       \
			break;                                                             \
		}                                                                      \
	})

#define ArrayFind(arr, result, pred)                                           \
	typeof((arr).ptr[0])* result = NULL;                                       \
	ArrayFindI(arr, result, pred)

#define ArrayLast(arr) (arr).ptr[(arr).len - 1]

#define ArrayFree(arr)                                                         \
	do {                                                                       \
		free((arr).ptr);                                                       \
		(arr).ptr = NULL;                                                      \
		(arr).len = (arr).cap = 0;                                             \
	} while(0)

////////////////////////////////////////////////////////////////////////////////

#define PoolN(t, n)                                                            \
	typedef struct {                                                           \
		struct {                                                               \
			t*     ptr;                                                        \
			size_t len;                                                        \
			size_t cap;                                                        \
		} used;                                                                \
		struct {                                                               \
			t**    ptr;                                                        \
			size_t len;                                                        \
			size_t cap;                                                        \
		} free;                                                                \
	} n

#define Pool(t) PoolN(t, t##Pool)

#define PoolNew(pool)                                                          \
	((pool).free.len > 0 ? (pool).free.ptr[--(pool).free.len]                  \
	                     : ArrayNew((pool).used))

#define PoolDel(pool, x) ArrayAdd((pool).free, (x))

#define PoolLoop(pool, body)                                                   \
	ArrayLoop(pool.used, {                                                     \
		if(it->oid == OBJ_INVALID) continue;                                   \
		body                                                                   \
	})

#if __INCLUDE_LEVEL__ == 0 /////////////////////////////////////////////////////

SDL_Window*   window;
SDL_Renderer* renderer;

TTF_Font*       font;
TTF_TextEngine* textEngine;

int64_t dtf;
double  fps;

bool debugVisible;

V2d rotate(V2d vec, double rot) {
	return m2d_mul_vec(m2d_rot(rot * DEG2RAD), vec);
}

double mod(double a, double b) {
	double z = fmod(a, b);
	if(z < 0) z += b;
	return z;
}

V2i64 v2i64_imp(V2i64 base, double m, double r) {
	V2d vec = rotate(v2d(m, 0), r);
	return v2i64_add(v2i642d(vec), base);
}

#endif
