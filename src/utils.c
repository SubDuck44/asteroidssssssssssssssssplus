#pragma once

#include <SDL3/SDL.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define RAD2DEG (180 / M_PI)
#define DEG2RAD (1 / RAD2DEG)

// Errors
#define ERR_FATAL false
#define ERR_PASS true
#define CODE_ERROR "[1;31m"
#define CODE_WARN "[1;33m"
#define CODE_SUCCESS "[1;32m"
#define CODE_END "[m"
#ifndef NDEBUG
#define ASSERT_PREDICATE(predicate, catch, success, error)                     \
	do {                                                                       \
		if(!(predicate)) {                                                     \
			SDL_Log(error);                                                    \
			catch                                                              \
		} else {                                                               \
			SDL_Log(success);                                                  \
		}                                                                      \
	} while(0)
#define ASSERT_PREDICATE_SDL(predicate, catch, success, error)                 \
	do {                                                                       \
		if(!(predicate)) {                                                     \
			SDL_Err(error);                                                    \
			catch                                                              \
		} else {                                                               \
			SDL_Log(success);                                                  \
		}                                                                      \
	} while(0)
#else
#define ASSERT_PREDICATE(predicate, catch, success, error) (void) (predicate)
#define ASSERT_PREDICATE_SDL(predicate, catch, success, error)                 \
	(void) (predicate)
#endif
#define SDL_Err(fmt, ...)                                                      \
	do {                                                                       \
		SDL_LogError(                                                          \
			SDL_LOG_CATEGORY_APPLICATION, fmt ": %s",                          \
			__VA_ARGS__ __VA_OPT__(, ) SDL_GetError()                          \
		);                                                                     \
	} while(0)
typedef bool Error;

// Vector2f math
typedef struct {
	double x;
	double y;
} Vector2f;
// -----------------------------------------------------------------------------
Vector2f   Vec2f_add(Vector2f a, Vector2f b);
Vector2f   Vec2f_add_value(Vector2f a, double b);
Vector2f   Vec2f_invert(Vector2f target);
Vector2f   Vec2f_subtract(Vector2f minuend, Vector2f subtrahend);
Vector2f   Vec2f_rotate(Vector2f a, double deg);
Vector2f   Vec2f_normalize(Vector2f a);
Vector2f   Vec2f_scale(Vector2f a, double scale);
double     Vec2f_angle_to(Vector2f from, Vector2f to);
Vector2f   Vec2f_force(double magnitude, double rotation);
double     Vec2f_length(Vector2f a);
SDL_FPoint Vec2f_to_FPoint(Vector2f a);

// Vector2 long math
typedef struct {
	int64_t x;
	int64_t y;
} Vector2l;
// -----------------------------------------------------------------------------
Vector2l Vec2l_add(Vector2l a, Vector2l b);
Vector2l Vec2l_add_value(Vector2l a, int64_t b);
Vector2l Vec2l_add_fpoint(Vector2l a, SDL_FPoint b);
Vector2l Vec2l_add_Vec2f(Vector2l a, Vector2f b);
Vector2l Vec2l_invert(Vector2l target);
Vector2l Vec2l_subtract(Vector2l minuend, Vector2l subtrahend);
Vector2l Vec2l_rotate(Vector2l a, int64_t deg);
Vector2l Vec2l_normalize(Vector2l a, uint64_t fixed_point);
Vector2l Vec2l_scale(Vector2l a, int64_t scale);
int64_t  Vec2l_angle_to(Vector2l from, Vector2l to);
Vector2l Vec2l_force(int64_t magnitude, int64_t rotation);
int64_t  Vec2l_length(Vector2l a);
double   Vec2l_get_distance(Vector2l from, Vector2l to);

// Camera/transform system
#define DEFAULT_COLLGRID_CELLSIZE 1024
#define DEFAULT_FIXED_POINT 4194304

typedef struct {
	Vector2l  target;
	float     zoom;
	int8_t    zoom_factor;
	SDL_Point screensize;
} Camera;
// -----------------------------------------------------------------------------
SDL_FPoint Cam_world_to_screen(Vector2l target, Camera* cam);
Vector2l   Cam_screen_to_world(SDL_FPoint target, Camera* cam);
void       Cam_transform(
		  Vector2l* world_pos, SDL_FPoint* screen_pos, SDL_FRect* dest,
		  SDL_FPoint* origin, Camera* cam
	  );

// Vector2 math
typedef struct {
	int32_t x;
	int32_t y;
} Vector2;
// -----------------------------------------------------------------------------
Vector2 Vec2_add(Vector2 a, Vector2 b);
Vector2 Vec2_add_value(Vector2 a, int32_t b);
Vector2 Vec2_invert(Vector2 target);
Vector2 Vec2_subtract(Vector2 minuend, Vector2 subtrahend);
Vector2 Vec2_rotate(Vector2 a, int32_t deg);
Vector2 Vec2_normalize(Vector2 a, uint32_t fixed_point);
Vector2 Vec2_scale(Vector2 a, int32_t scale);
int32_t Vec2_angle_to(Vector2 a, Vector2 b);
Vector2 Vec2_force(int32_t magnitude, int32_t rotation);
int32_t Vec2_length(Vector2 a);

// SDL_FPoint math
SDL_FPoint FPoint_add(SDL_FPoint a, SDL_FPoint b);
SDL_FPoint FPoint_add_value(SDL_FPoint a, float b);
SDL_FPoint FPoint_invert(SDL_FPoint target);
SDL_FPoint FPoint_subtract(SDL_FPoint minuend, SDL_FPoint subtrahend);
SDL_FPoint FPoint_rotate(SDL_FPoint a, float deg);
SDL_FPoint FPoint_normalize(SDL_FPoint a);
SDL_FPoint FPoint_scale(SDL_FPoint a, float scale);
float      FPoint_angle_to(SDL_FPoint from, SDL_FPoint to);
SDL_FPoint FPoint_force(float magnitude, float rotation);
float      FPoint_length(SDL_FPoint a);
Vector2f   FPoint_to_Vec2f(SDL_FPoint a);

// Misc
double  clamp(double min, double max, double val);
int32_t clampi(int32_t minimum, int32_t maximum, int32_t value);
int32_t mini(int32_t a, int32_t b);
int32_t maxi(int32_t a, int32_t b);
double  min(double a, double b);
double  max(double a, double b);
bool    colrect_check_collision(
	   Vector2l self_pos, Vector2f self_size, Vector2l other_pos,
	   Vector2f other_size
   );

#if __INCLUDE_LEVEL__ == 0 /////////////////////////////////////////////////////

// Vector2l math ===============================================================

Vector2l Vec2l_add(Vector2l a, Vector2l b) {
	return (Vector2l) {a.x + b.x, a.y + b.y};
}

Vector2l Vec2l_add_value(Vector2l a, int64_t b) {
	return (Vector2l) {a.x + b, a.y + b};
}

Vector2l Vec2l_add_fpoint(Vector2l a, SDL_FPoint b) {
	return (Vector2l) {a.x + b.x, a.y + b.y};
}

Vector2l Vec2l_add_Vec2f(Vector2l a, Vector2f b) {
	return (Vector2l) {a.x + b.x, a.y + b.y};
}

Vector2l Vec2l_invert(Vector2l target) {
	return (Vector2l) {-target.x, -target.y};
}

Vector2l Vec2l_subtract(Vector2l minuend, Vector2l subtrahend) {
	return (Vector2l) {minuend.x - subtrahend.x, minuend.y - subtrahend.y};
}

Vector2l Vec2l_rotate(Vector2l a, int64_t deg) {
	return (Vector2l) {(a.x * cos(deg * DEG2RAD)) - (a.y * sin(deg * DEG2RAD)),
	                   (a.x * sin(deg * DEG2RAD)) + (a.y * cos(deg * DEG2RAD))};
}

Vector2l Vec2l_normalize(Vector2l a, uint64_t fixed_point) {
	const double length = sqrt((a.x * a.x) + (a.y * a.y)) / fixed_point;
	if(length <= SDL_FLT_EPSILON) return a;
	return (Vector2l) {a.x / length, a.y / length};
}

Vector2l Vec2l_scale(Vector2l a, int64_t scale) {
	return (Vector2l) {a.x * scale, a.y * scale};
}

int64_t Vec2l_angle_to(Vector2l from, Vector2l to) {
	double angle = atan2(to.x - from.x, from.y - to.y) * RAD2DEG;
	if(angle < SDL_FLT_EPSILON) angle += 360;
	return angle;
}

Vector2l Vec2l_force(int64_t magnitude, int64_t rotation) {
	return Vec2l_rotate(Vec2l_scale((Vector2l) {0, -1}, magnitude), rotation);
}

int64_t Vec2l_length(Vector2l a) {
	return sqrt((a.x * a.x) + (a.y * a.y));
}

double Vec2l_get_distance(Vector2l from, Vector2l to) {
	return Vec2l_length(Vec2l_subtract(to, from));
}

// Vector2f math
// ===============================================================

Vector2f Vec2f_add(Vector2f a, Vector2f b) {
	return (Vector2f) {a.x + b.x, a.y + b.y};
}

Vector2f Vec2f_add_value(Vector2f a, double b) {
	return (Vector2f) {a.x * b, a.y * b};
}

Vector2f Vec2f_invert(Vector2f target) {
	return (Vector2f) {-target.x, -target.y};
}

Vector2f Vec2f_subtract(Vector2f minuend, Vector2f subtrahend) {
	return (Vector2f) {minuend.x - subtrahend.x, minuend.y - subtrahend.y};
}

Vector2f Vec2f_rotate(Vector2f a, double deg) {
	return (Vector2f) {(a.x * cos(deg * DEG2RAD)) - (a.y * sin(deg * DEG2RAD)),
	                   (a.x * sin(deg * DEG2RAD)) + (a.y * cos(deg * DEG2RAD))};
}

Vector2f Vec2f_normalize(Vector2f a) {
	const double length = sqrt((a.x * a.x) + (a.y * a.y));
	if(length <= SDL_FLT_EPSILON) return a;
	return (Vector2f) {a.x / length, a.y / length};
}

Vector2f Vec2f_scale(Vector2f a, double scale) {
	return (Vector2f) {a.x * scale, a.y * scale};
}

double Vec2f_angle_to(Vector2f from, Vector2f to) {
	double angle = atan2(to.x - from.x, from.y - to.y) * RAD2DEG;
	if(angle < SDL_FLT_EPSILON) angle += 360;
	return angle;
}

Vector2f Vec2f_force(double magnitude, double rotation) {
	return Vec2f_rotate(
		Vec2f_scale((Vector2f) {0.0, -1.0}, magnitude), rotation
	);
}

double Vec2f_length(Vector2f a) {
	return sqrt((a.x * a.x) + (a.y * a.y));
}

SDL_FPoint Vec2f_to_FPoint(Vector2f a) {
	return (SDL_FPoint) {a.x, a.y};
}

// SDL_FPoint math
// =============================================================

SDL_FPoint FPoint_add(SDL_FPoint a, SDL_FPoint b) {
	return (SDL_FPoint) {a.x + b.x, a.y + b.y};
}

SDL_FPoint FPoint_add_value(SDL_FPoint a, float b) {
	return (SDL_FPoint) {a.x + b, a.y + b};
}

SDL_FPoint FPoint_invert(SDL_FPoint target) {
	return (SDL_FPoint) {-target.x, target.y};
}

SDL_FPoint FPoint_subtract(SDL_FPoint minuend, SDL_FPoint subtrahend) {
	return (SDL_FPoint) {minuend.x - subtrahend.x, minuend.y - subtrahend.y};
}

SDL_FPoint FPoint_rotate(SDL_FPoint a, float deg) {
	return (SDL_FPoint) {
		(a.x * cos(deg * DEG2RAD)) - (a.y * sin(deg * DEG2RAD)),
		(a.x * sin(deg * DEG2RAD)) + (a.y * cos(deg * DEG2RAD))
	};
}

SDL_FPoint FPoint_normalize(SDL_FPoint a) {
	const double length = sqrt((a.x * a.x) + (a.y * a.y));
	if(length <= SDL_FLT_EPSILON) return a;
	return (SDL_FPoint) {a.x / length, a.y / length};
}

SDL_FPoint FPoint_scale(SDL_FPoint a, float scale) {
	return (SDL_FPoint) {a.x * scale, a.y * scale};
}

float FPoint_angle_to(SDL_FPoint from, SDL_FPoint to) {
	double angle = atan2(to.x - from.x, from.y - to.y) * RAD2DEG;
	if(angle < SDL_FLT_EPSILON) angle += 360;
	return angle;
}

SDL_FPoint FPoint_force(float magnitude, float rotation) {
	return FPoint_rotate(
		FPoint_scale((SDL_FPoint) {0.0f, -1.0f}, magnitude), rotation
	);
}

float FPoint_length(SDL_FPoint a) {
	return sqrt((a.x * a.x) + (a.y * a.y));
}

Vector2f FPoint_to_Vec2f(SDL_FPoint a) {
	return (Vector2f) {a.x, a.y};
}

// Camera/transform system REWRITE
// =============================================
SDL_FPoint Cam_world_to_screen(Vector2l target, Camera* cam) {
	int64_t diff_x = (target.x - cam->target.x) / DEFAULT_FIXED_POINT;
	int64_t diff_y = (target.y - cam->target.y) / DEFAULT_FIXED_POINT;

	double screen_x = (double) diff_x;
	double screen_y = (double) diff_y;

	screen_x *= cam->zoom;
	screen_y *= cam->zoom;

	screen_x += cam->screensize.x >> 1;
	screen_y += cam->screensize.y >> 1;

	return (SDL_FPoint) {screen_x, screen_y};
}

Vector2l Cam_screen_to_world(SDL_FPoint target, Camera* cam) {
	int64_t world_x = (target.x * DEFAULT_FIXED_POINT) - cam->target.x;
	int64_t world_y = (target.y * DEFAULT_FIXED_POINT) - cam->target.y;

	return (Vector2l) {world_x, world_y};
}

void Cam_transform(
	Vector2l* world_pos, SDL_FPoint* screen_pos, SDL_FRect* dest,
	SDL_FPoint* origin, Camera* cam
) {
	SDL_FPoint screen_origin = Cam_world_to_screen(*world_pos, cam);

	screen_origin = FPoint_scale(screen_origin, cam->zoom);

	float dest_w = dest->w * cam->zoom;
	float dest_h = dest->h * cam->zoom;

	screen_origin = FPoint_add(screen_origin, (SDL_FPoint) {dest->x, dest->y});

	*screen_pos = screen_origin;

	*origin = (SDL_FPoint) {origin->x * cam->zoom, origin->y * cam->zoom};

	*dest = (SDL_FRect) {screen_origin.x - origin->x,
	                     screen_origin.y - origin->y, dest_w, dest_h};
}

// Vector2 math
// ================================================================

/* Adds two vector */
Vector2 Vec2_add(Vector2 a, Vector2 b) {
	return (Vector2) {a.x + b.x, a.y + b.y};
}

/* Adds a value onto both components of a vector */
Vector2 Vec2_add_value(Vector2 a, int32_t b) {
	return (Vector2) {a.x + b, a.y + b};
}

/* Inverts the components of a vector */
Vector2 Vec2_invert(Vector2 target) {
	return (Vector2) {-target.x, -target.y};
}

/* Subtracts the components of a vector from another */
Vector2 Vec2_subtract(Vector2 minuend, Vector2 subtrahend) {
	return (Vector2) {minuend.x - subtrahend.x, minuend.y - subtrahend.y};
}

/* Rotates a vector (in degrees) CLOCKWISE around NORTH-ZERO (0° at -y) */
Vector2 Vec2_rotate(Vector2 a, int32_t deg) {
	return (Vector2) {(a.x * cos(deg * DEG2RAD)) - (a.y * sin(deg * DEG2RAD)),
	                  (a.x * sin(deg * DEG2RAD)) + (a.y * cos(deg * DEG2RAD))};
}

/* Sets the length of a vector to FIXED_POINT without sacrificing its
 * rotation
 */
Vector2 Vec2_normalize(Vector2 a, uint32_t fixed_point) {
	const int32_t length = sqrt((a.x * a.x) + (a.y * a.y)) * fixed_point;
	if(length <= 0) return a;
	return (Vector2) {a.x / length, a.y / length};
}

/* Multiplies the components of a vector by a scale factor */
Vector2 Vec2_scale(Vector2 a, int32_t scale) {
	return (Vector2) {a.x * scale, a.y * scale};
}

/* Gets the angle to a vector2 from a certain vector2 on a 360 DEGREEE
 * CLOCKWISE circle with NORTH-UP (0° at -y) */
int32_t Vec2_angle_to(Vector2 from, Vector2 to) {
	uint32_t angle = atan2(to.x - from.x, from.y - to.y) * RAD2DEG;
	return angle;
}

/* Creates a vector2 with a given length (magnitude) and rotation */
Vector2 Vec2_force(int32_t magnitude, int32_t rotation) {
	return Vec2_rotate(Vec2_scale((Vector2) {0, -1}, magnitude), rotation);
}

/* Gets the length of a vector */
int32_t Vec2_length(Vector2 a) {
	return sqrt((a.x * a.x) + (a.y * a.y));
}

// Misc
// ========================================================================

double clamp(double minimum, double maximum, double val) {
	return min(maximum, max(minimum, val));
}

int32_t clampi(int32_t minimum, int32_t maximum, int32_t value) {
	return mini(maximum, maxi(minimum, value));
}

double min(double a, double b) {
	return (a < b) ? a : b;
}

int32_t mini(int32_t a, int32_t b) {
	return (a <= b) ? a : b;
}

double max(double a, double b) {
	return (a > b) ? a : b;
}

int32_t maxi(int32_t a, int32_t b) {
	return (a >= b) ? a : b;
}

bool colrect_check_collision(
	Vector2l self_pos, Vector2f self_size, Vector2l other_pos,
	Vector2f other_size
) {
	other_pos.x -= self_pos.x;
	other_pos.y -= self_pos.y;

	self_pos.x = 0;
	self_pos.y = 0;

	return (
		self_size.x >= other_pos.x && 0 <= other_pos.x + other_size.x &&
		self_size.y >= other_pos.x && 0 <= other_pos.y + other_size.y
	);
}

#endif
