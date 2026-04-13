#pragma once

#include <math.h>

#include <SDL3/SDL.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define RAD2DEG (180 / M_PI)
#define DEG2RAD (1 / RAD2DEG)

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

// Camera/transform system
#define DEFAULT_MAJORGRID_CELLSIZE 1024
#define DEFAULT_MINORGRID_FIXED_POINT 4194304

typedef struct {
	int32_t x;
	int32_t y;
	int32_t x_maj;
	int32_t y_maj;
} Position;
typedef struct {
	int32_t min;
	int32_t maj;
} Distance;
typedef struct {
	Position  target;
	float     zoom;
	int8_t    zoom_factor;
	SDL_Point screensize;
} Camera;
// -----------------------------------------------------------------------------
SDL_FPoint Pos_world_to_screen(Position target, Camera* cam);
Position   Pos_screen_to_world(SDL_FPoint target, Camera* cam);
void       Pos_cam_transform(
		  Position* world_pos, SDL_FPoint* screen_pos, SDL_FRect* dest,
		  SDL_FPoint* origin, Camera* cam
	  );
Position Pos_easy_get_pos(
	int32_t pixel_x, int32_t pixel_y, int32_t maj_x, int32_t maj_y
);
Position Pos_add(Position a, Position b);
Position Pos_add_Vec2f(Position a, Vector2f b);
Position Pos_invert(Position target);
Position Pos_subtract(Position minuend, Position subtrahend);
Distance Pos_get_distance(Position from, Position to);

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
double clamp(double min, double max, double val);
double min(double a, double b);
double max(double a, double b);

#if __INCLUDE_LEVEL__ == 0 /////////////////////////////////////////////////////
// Vector2f math ===============================================================

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

// SDL_FPoint math =============================================================

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

// Camera/transform system
// =====================================================

/* Converts a given Position to screenspace position in
SDL_FPoint format using a given camera transform Position
target: position to convert Camera* cam: reference to the
camera used */
SDL_FPoint Pos_world_to_screen(Position target, Camera* cam) {
	int32_t diff_x = (target.x - cam->target.x) / DEFAULT_MINORGRID_FIXED_POINT;
	int32_t diff_y = (target.y - cam->target.y) / DEFAULT_MINORGRID_FIXED_POINT;

	int32_t diff_x_maj = target.x_maj - cam->target.x_maj;
	int32_t diff_y_maj = target.y_maj - cam->target.y_maj;

	double screen_x = diff_x + (diff_x_maj * DEFAULT_MAJORGRID_CELLSIZE);
	double screen_y = diff_y + (diff_y_maj * DEFAULT_MAJORGRID_CELLSIZE);

	screen_x *= cam->zoom;
	screen_y *= cam->zoom;

	screen_x += cam->screensize.x >> 1;
	screen_y += cam->screensize.y >> 1;

	return (SDL_FPoint) {screen_x, screen_y};
}

/* Converts a given screenspace position in SDL_FPoint
format to a position using a given camera transform
SDL_FPoint target: screenspace position to convert
Camera* camm: reference to the camera transform used */
Position Pos_screen_to_world(SDL_FPoint target, Camera* cam) {
	int32_t x_maj =
		((int32_t) target.x / DEFAULT_MAJORGRID_CELLSIZE) - (target.x < 0);
	int32_t y_maj =
		((int32_t) target.y / DEFAULT_MAJORGRID_CELLSIZE) - (target.y < 0);

	int32_t x = target.x - (x_maj * DEFAULT_MAJORGRID_CELLSIZE);
	int32_t y = target.y - (y_maj * DEFAULT_MAJORGRID_CELLSIZE);

	x -= DEFAULT_MAJORGRID_CELLSIZE / 2;
	y -= DEFAULT_MAJORGRID_CELLSIZE / 2;

	x *= DEFAULT_MINORGRID_FIXED_POINT;
	y *= DEFAULT_MINORGRID_FIXED_POINT;

	x_maj += cam->target.x_maj;
	y_maj += cam->target.y_maj;
	x += cam->target.x;
	y += cam->target.y;

	return (Position) {x, y, x_maj, y_maj};
}

void Pos_cam_transform(
	Position* world_pos, SDL_FPoint* screen_pos, SDL_FRect* dest,
	SDL_FPoint* origin, Camera* cam
) {
	int32_t diff_x = (world_pos->x / DEFAULT_MINORGRID_FIXED_POINT) -
	                 (cam->target.x / DEFAULT_MINORGRID_FIXED_POINT);
	int32_t diff_y = (world_pos->y / DEFAULT_MINORGRID_FIXED_POINT) -
	                 (cam->target.y / DEFAULT_MINORGRID_FIXED_POINT);

	int32_t diff_x_maj = world_pos->x_maj - cam->target.x_maj;
	int32_t diff_y_maj = world_pos->y_maj - cam->target.y_maj;

	double screen_x = diff_x + (diff_x_maj * DEFAULT_MAJORGRID_CELLSIZE);
	double screen_y = diff_y + (diff_y_maj * DEFAULT_MAJORGRID_CELLSIZE);

	screen_x *= cam->zoom;
	screen_y *= cam->zoom;

	screen_x += cam->screensize.x >> 1;
	screen_y += cam->screensize.y >> 1;

	float dest_w = dest->w * cam->zoom;
	float dest_h = dest->h * cam->zoom;

	screen_x += dest->x;
	screen_y += dest->y;

	*origin = (SDL_FPoint) {origin->x * cam->zoom, origin->y * cam->zoom};
	*dest   = (SDL_FRect) {screen_x - origin->x, screen_y - origin->y, dest_w,
	                       dest_h};
	screen_pos->x = screen_x;
	screen_pos->y = screen_y;
}

Position Pos_add_Vec2f(Position a, Vector2f b) {
	int32_t x_maj = (b.x / DEFAULT_MAJORGRID_CELLSIZE);
	int32_t y_maj = (b.y / DEFAULT_MAJORGRID_CELLSIZE);

	int32_t x = b.x - (x_maj * DEFAULT_MAJORGRID_CELLSIZE);
	int32_t y = b.y - (y_maj * DEFAULT_MAJORGRID_CELLSIZE);

	x *= DEFAULT_MINORGRID_FIXED_POINT;
	y *= DEFAULT_MINORGRID_FIXED_POINT;

	return Pos_add(a, (Position) {x, y, x_maj, y_maj});
}

Position Pos_easy_get_pos(
	int32_t pixel_x, int32_t pixel_y, int32_t maj_x, int32_t maj_y
) {
	return (Position) {pixel_x * DEFAULT_MINORGRID_FIXED_POINT,
	                   pixel_y * DEFAULT_MINORGRID_FIXED_POINT, maj_x, maj_y};
}

/* Adds two positions ontop of another, handling overflow
 from minor to major
 * grid.
 Position a: first summand
 Position b: second summand */
Position Pos_add(Position a, Position b) {
	int32_t maj_x = a.x_maj + b.x_maj;
	int32_t maj_y = a.y_maj + b.y_maj;
	// TODO MAKE THIS BRANCHLESS PLEASSEEEEE
	if(((int64_t) a.x + (int64_t) b.x) < INT32_MIN) maj_x--;
	if(((int64_t) a.y + (int64_t) b.y) < INT32_MIN) maj_y--;
	if(((int64_t) a.x + (int64_t) b.x) > INT32_MAX) maj_x++;
	if(((int64_t) a.y + (int64_t) b.y) > INT32_MAX) maj_y++;
	uint32_t x = a.x + b.x;
	uint32_t y = a.y + b.y;
	return (Position) {x, y, maj_x, maj_y};
}

Position Pos_invert(Position target) {
	return (Position) {-target.x, -target.y, -target.x_maj, -target.y_maj};
}

Position Pos_subtract(Position minuend, Position subtrahend) {
	return Pos_add(minuend, Pos_invert(subtrahend));
}

Distance Pos_get_distance(Position from, Position to) {
	const Position diff = Pos_subtract(to, from);
	return (Distance) {Vec2_length((Vector2) {diff.x, diff.y}),
	                   Vec2_length((Vector2) {diff.x_maj, diff.y_maj})};
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
	if(angle < 0) angle += 360;
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

double min(double a, double b) {
	return (a < b) ? a : b;
}

double max(double a, double b) {
	return (a > b) ? a : b;
}

#endif
