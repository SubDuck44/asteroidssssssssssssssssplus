#pragma once

#define APPLICATION_TITLE "Asteroidssssssssssssssss+"
#define _DEFAULT_SOURCE
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
#define ASSERT_PREDICATE(predicate, catch, success, error) predicate
#define ASSERT_PREDICATE_SDL(predicate, catch, success, error) predicate
#endif
#define SDL_Err(fmt, ...)                                                      \
	do {                                                                       \
		SDL_LogError(                                                          \
			SDL_LOG_CATEGORY_APPLICATION, fmt ": %s",                          \
			__VA_ARGS__ __VA_OPT__(, ) SDL_GetError()                          \
		);                                                                     \
	} while(0)
#define GAMEOBJECT_CREATE_SUCCESS "INFO: Successfully created GameObject"
#define GAMEOBJECT_CREATE_FAILURE "FATAL: Failed to create GameObject"
#define RAD2DEG (180 / M_PI)
#define DEG2RAD (1 / RAD2DEG)
#define WRAP_COMPASS(x) PROPER_MOD(x, 360)
#define PROPER_MOD(x, mod) (((x % mod) + mod) % mod)

#define DEFAULT_GAMEOBJECT_QUEUE_CAP 32
#define DEFAULT_UPDATECALLBACK_QUEUE_CAP 32
#define DEFAULT_SCREENWIDTH 1280
#define DEFAULT_SCREENHEIGHT 720
#define DEFAULT_VSYNC_ENABLE
#define DEFAULT_MAJORGRID_CELLSIZE 1024
#define DEFAULT_MINORGRID_FIXED_POINT 4194304
#define DEFAULT_FPS 60
#define DEFAULT_FPS_TEXTBUFFER_SIZE 64
#define DEFAULT_FONTSIZE 12.0f

typedef bool Error;

#include <SDL3/SDL.h>
#include <SDL3_gfx/SDL3_gfxPrimitives.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <math.h>
#include <stdlib.h>

#include <endian.h>
#include <stdio.h>

#include "res.c"

// Control flow
SDL_AppResult Eng_init(void);
void          Eng_exit(void);
SDL_AppResult Eng_tick_input(SDL_Event* event);
Error         Eng_tick_once(void);

// Camera/transform system
typedef struct {
	int32_t x;
	int32_t y;
	int32_t x_maj;
	int32_t y_maj;
} Position;
typedef struct {
	Position target;
	float    zoom;
} Camera;
// -----------------------------------------------------------------------------
SDL_FPoint Eng_get_screen_pos(Position target, Camera* cam);
SDL_FPoint Eng_position_to_pointf(Position target);
Position   Eng_get_world_pos(SDL_FPoint target, Camera* cam);
Position   Eng_pointf_to_position(SDL_FPoint target);
SDL_FRect  Eng_frect_scale(SDL_FRect target, float scale);
Position   Eng_position_add(Position a, Position b);
Position   Eng_position_add_pointf(Position a, SDL_FPoint b);
Position   Eng_position_invert(Position target);
Position   Eng_position_subtract(Position minuend, Position subtrahend);
float      Eng_get_distance(Position a, Position b);

// GameObject management
typedef struct {
	uint32_t type;
	void*    data;
} GameObject;
typedef Error (*Method)(void* data, uint32_t index);
typedef struct {
	Method func;
	void*  argv;
} UpdateHook;
// -----------------------------------------------------------------------------
Error Eng_create_object(
	void* src, void** new_ref, size_t data_size, uint32_t type
);
Error Eng_destroy_object(uint32_t target);
Error Eng_hook_update(Method func, void* data);
Error Eng_unhook_update(void* data);

// Vector math
SDL_FPoint Eng_pointf_add(SDL_FPoint a, SDL_FPoint b);
SDL_FPoint Eng_pointf_add_value(SDL_FPoint a, float b);
SDL_FPoint Eng_pointf_invert(SDL_FPoint target);
SDL_FPoint Eng_pointf_subtract(SDL_FPoint minuend, SDL_FPoint subtrahend);
SDL_FPoint Eng_pointf_rotate(SDL_FPoint a, float deg);
SDL_FPoint Eng_pointf_normalize(SDL_FPoint a);
SDL_FPoint Eng_pointf_scale(SDL_FPoint a, double scale);
SDL_FPoint pointf_angle_to(SDL_FPoint a, SDL_FPoint b);
SDL_FPoint Eng_pointf_force(float magnitude, float rotation);
float      Eng_pointf_length(SDL_FPoint a);
float      Eng_pointf_bearing(SDL_FPoint zero, SDL_FPoint tgt);

// Misc

double Eng_get_deltatime_factor(void);

// KEY input handling BEGIN
#define KEYS X(W) X(A) X(S) X(D) X(LALT) X(I) X(J) X(K) X(L)

enum Keys : uint32_t {
#define X(x) KEY_##x,
	KEYS
#undef X
		KEY_MOUSE_LEFT,
	KEY_MOUSE_RIGHT,
	KEY_NUM,
};

#define Eng_get_key_pressed(key) Eng_key_cache[key]

extern bool       Eng_key_cache[KEY_NUM];
extern SDL_FPoint Eng_mouse_pos;
// KEY input handling END

extern SDL_Point Eng_screensize;
extern uint32_t  Eng_desired_fps;
extern uint32_t  Eng_current_fps;

extern TTF_Font*       Eng_font;
extern TTF_TextEngine* Eng_text_engine;
extern const char      EMB_IOSEVKA_FONT[];

extern SDL_Window*   window;
extern SDL_Renderer* renderer;

extern Camera Eng_std_camera;

#if __INCLUDE_LEVEL__ == 0 /////////////////////////////////////////////////////

static GameObject* game_objects     = {0};
static uint32_t    game_objects_len = 0;
static uint32_t    game_objects_cap = DEFAULT_GAMEOBJECT_QUEUE_CAP;

static UpdateHook* update_callbacks     = {0};
static uint32_t    update_callbacks_len = 0;
static uint32_t    update_callbacks_cap = DEFAULT_UPDATECALLBACK_QUEUE_CAP;

static uint64_t last_frame_time = 1;
uint32_t        Eng_desired_fps = 60;
uint32_t        Eng_current_fps = 1;

static uint16_t fontsize;
TTF_Font*       Eng_font;
TTF_TextEngine* Eng_text_engine;

SDL_Window*   window;
SDL_Renderer* renderer;

Camera Eng_std_camera;

const char EMB_IOSEVKA_FONT[] = {
#embed "../res/Iosevka-Regular.ttf"
};

// KEY input handling BEGIN

bool       Eng_key_cache[KEY_NUM] = {0};
SDL_FPoint Eng_mouse_pos          = {0};

// KEY input handling END

SDL_Point Eng_screensize = {DEFAULT_SCREENWIDTH, DEFAULT_SCREENHEIGHT};

// Control flow
// ================================================================

/* Initialize the engine, required for… well… everything in
 * it to work */
SDL_AppResult Eng_init(void) {
	SDL_Log("INFO: Initializing " APPLICATION_TITLE "…");
	bool fatal_error = false;

	// Try allocate memory for GameObject queue
	ASSERT_PREDICATE(
		(game_objects =
	         calloc(DEFAULT_GAMEOBJECT_QUEUE_CAP, sizeof(game_objects[0]))),
		fatal_error = true;
		,
		CODE_SUCCESS "INFO: Successfully allocated memory "
					 "for GameObject queue" CODE_END,
		CODE_ERROR "FATAL: Failed to allocate memory "
				   "for GameObject queue" CODE_END
	);

	// Try allocate memory for UpdateHook queue
	ASSERT_PREDICATE(
		update_callbacks = calloc(
			DEFAULT_UPDATECALLBACK_QUEUE_CAP, sizeof(update_callbacks[0])
		),
		fatal_error = true;
		,
		CODE_SUCCESS "INFO: Successfully allocated memory "
					 "for UpdateHook queue" CODE_END,
		CODE_ERROR "FATAL: Failed to allocate memory for "
				   "UpdateHook queue" CODE_END
	);

	// Try setup main SDL lib
	ASSERT_PREDICATE_SDL(SDL_Init(SDL_INIT_VIDEO), fatal_error = true;
	                     ,
	                     CODE_SUCCESS
	                     "INFO: Successfully initialized SDL" CODE_END,
	                     CODE_ERROR "FATAL: Failed to initialize SDL" CODE_END);

	// Try setup window
	ASSERT_PREDICATE_SDL(SDL_CreateWindowAndRenderer(
							 "Asteroidssssssssssssssss+", 1280, 720, 0, &window,
							 &renderer
						 ),
	                     fatal_error = true;
	                     ,
	                     CODE_SUCCESS "INFO: Successfully initialized "
	                                  "window/renderer" CODE_END,
	                     CODE_ERROR "FATAL: Failed to initialize "
	                                "window/renderer" CODE_END);

	// Disable AA
	SDL_SetDefaultTextureScaleMode(renderer, SDL_SCALEMODE_PIXELART);

	// Try setup font lib
	ASSERT_PREDICATE(
		TTF_Init(), fatal_error = true;
		, CODE_SUCCESS "INFO: Successfully started TTF" CODE_END,
		CODE_ERROR "FATAL: Failed to start TTF" CODE_END
	);

	// Try setup font file
	ASSERT_PREDICATE(
		Eng_font = TTF_OpenFontIO(
			SDL_IOFromConstMem(EMB_IOSEVKA_FONT, sizeof(EMB_IOSEVKA_FONT)),
			true, DEFAULT_FONTSIZE
		),
		fatal_error = true;
		, CODE_SUCCESS "INFO: Successfully loaded font" CODE_END,
		CODE_ERROR "FATAL: Failed to load font" CODE_END
	);
	fontsize = DEFAULT_FONTSIZE;

	// Try setup text engine
	ASSERT_PREDICATE(
		Eng_text_engine = TTF_CreateRendererTextEngine(renderer),
		fatal_error     = true;
		,
		CODE_SUCCESS "INFO: Successfully initialized font "
					 "engine" CODE_END,
		CODE_ERROR "FATAL: Failed to initialize font engine" CODE_END
	);

#ifdef VSYNC_ON
	// Try setup VSync
	ASSERT_PREDICATE_SDL(
		SDL_SetRenderVSync(renderer, SDL_RENDERER_VSYNC_ADAPTIVE), ;
		, CODE_SUCCESS "INFO: Successfully enabled VSync" CODE_END,
		CODE_WARNING "WARNING: Failed to enable VSync" CODE_END
	);
#endif

	// Try setup textures
	bool texture_failed = false;

	for(uint32_t i = 0; i < TEXTURES_COUNT; i++) {
		SDL_Surface* surface = SDL_LoadPNG_IO(
			SDL_IOFromConstMem(TEXTURES[i]->tex_data, TEXTURES[i]->tex_size),
			true
		);
		if(!surface) {
#ifndef NDEBUG
			SDL_Err(
				CODE_ERROR "FATAL: Failed to load texture "
						   "%d into RAM" CODE_END,
				i
			);
			fatal_error    = true;
			texture_failed = true;
#endif
		} else {
			TEXTURES[i]->tex = SDL_CreateTextureFromSurface(renderer, surface);
#ifndef NDEBUG
			if(!TEXTURES[i]->tex) {
				SDL_Err(
					CODE_ERROR "FATAL: Failed to load texture %d "
							   "into VRAM" CODE_END,
					i
				);
				fatal_error    = true;
				texture_failed = true;
			}
#endif
			SDL_DestroySurface(surface);
		}
	}
#ifndef NDEBUG
	if(!texture_failed)
		SDL_Log(
			CODE_SUCCESS "INFO: Successfully initialized "
						 "%d textures" CODE_END,
			TEXTURES_COUNT
		);
#endif

	// Arbitrary initializations
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	Eng_std_camera = (Camera) {(Position) {0}, 1.0f};

	ASSERT_PREDICATE(!fatal_error, return SDL_APP_FAILURE;
	                 ,
	                 CODE_SUCCESS "INFO: Green across the board, "
	                              "launching…" CODE_END,
	                 CODE_ERROR "FATAL: Caught one or more fatal "
	                            "exceptions, aborting…" CODE_END);
	SDL_Log("INFO: Welcome to " APPLICATION_TITLE "!");
	return SDL_APP_CONTINUE;
}

/* Runs at the end of the program. */
void Eng_exit(void) {
	SDL_Log("INFO: Shutting down…");
}

/*
Run all input capturing events, return value MUST be
returned from SDL_AppEvent() SDL_Event* event: Pass
SDL_Event* from SDL_AppEvent()
 */
SDL_AppResult Eng_tick_input(SDL_Event* event) {
	switch(event->type) {
	case SDL_EVENT_MOUSE_MOTION:
		Eng_mouse_pos = (SDL_FPoint) {event->motion.x, event->motion.y};
		break;
	case SDL_EVENT_MOUSE_BUTTON_DOWN:
		switch(event->button.button) {
		case SDL_BUTTON_LEFT:
			Eng_key_cache[KEY_MOUSE_LEFT] = true;
			break;
		case SDL_BUTTON_RIGHT:
			Eng_key_cache[KEY_MOUSE_RIGHT] = true;
			break;
		}
		break;

	case SDL_EVENT_MOUSE_BUTTON_UP:
		switch(event->button.button) {
		case SDL_BUTTON_LEFT:
			Eng_key_cache[KEY_MOUSE_LEFT] = false;
			break;
		case SDL_BUTTON_RIGHT:
			Eng_key_cache[KEY_MOUSE_RIGHT] = false;
			break;
		}
		break;

	case SDL_EVENT_KEY_DOWN:
		switch(event->key.key) {
		case SDLK_ESCAPE:
			// Manual quit from the window
			SDL_Log("INFO: Escape pressed, exiting…");
			return SDL_APP_SUCCESS;

#define X(x)                                                                   \
	case SDLK_##x:                                                             \
		Eng_key_cache[KEY_##x] = true;                                         \
		break;
			KEYS
#undef X
		}
		break;

	case SDL_EVENT_KEY_UP:
		switch(event->key.key) {
#define X(x)                                                                   \
	case SDLK_##x:                                                             \
		Eng_key_cache[KEY_##x] = false;                                        \
		break;
			KEYS
#undef X
		}
		break;

	case SDL_EVENT_QUIT:
		// If system sends quit event (e.g. closing the
		// window from the OS)
		SDL_Log(
			"INFO: Received termination request from "
			"system, exiting…"
		);
		return SDL_APP_SUCCESS;
	}

	return SDL_APP_CONTINUE;
}

/*
Process all callbacks in the update_callbacks queue
*/
Error Eng_tick_once(void) {
	// Start frame timer
	const uint64_t frametime_start = SDL_GetTicksNS();

	// Grey background
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	for(uint32_t i = 0; i < update_callbacks_len; i++) {
		if(update_callbacks[i].func(update_callbacks[i].argv, i) == ERR_FATAL) {
			return ERR_FATAL;
		}
	}

	SDL_RenderPresent(renderer);

	// Stop frame timer
	last_frame_time = SDL_GetTicksNS() - frametime_start;

	const uint64_t desired_frametime = 1'000'000'000 / Eng_desired_fps;
	if(last_frame_time < desired_frametime) {
		const uint64_t frame_diff = desired_frametime - last_frame_time;
		SDL_DelayPrecise(frame_diff);
		last_frame_time += frame_diff;
	}

	Eng_current_fps =
		(((double) (1'000'000'000) / last_frame_time) + Eng_current_fps) / 2;

	return ERR_PASS;
}

// Camera/transform system
// =====================================================

/* Converts a given Position to screenspace position in
SDL_FPoint format using a given camera transform Position
target: position to convert Camera* cam: reference to the
camera used */
SDL_FPoint Eng_get_screen_pos(Position target, Camera* cam) {
	Position diff =
		(Position) {target.x - cam->target.x, target.y - cam->target.y,
	                target.x_maj - cam->target.x_maj,
	                target.y_maj - cam->target.y_maj};
	int32_t screen_x = ((diff.x / DEFAULT_MINORGRID_FIXED_POINT) +
	                    (DEFAULT_MAJORGRID_CELLSIZE >> 1)) +
	                   (diff.x_maj * DEFAULT_MAJORGRID_CELLSIZE);
	int32_t screen_y = ((diff.y / DEFAULT_MINORGRID_FIXED_POINT) +
	                    (DEFAULT_MAJORGRID_CELLSIZE >> 1)) +
	                   (diff.y_maj * DEFAULT_MAJORGRID_CELLSIZE);
	return (SDL_FPoint) {screen_x, screen_y};
}

SDL_FPoint Eng_position_to_pointf(Position target) {
	int32_t screen_x = ((target.x / DEFAULT_MINORGRID_FIXED_POINT) +
	                    (DEFAULT_MAJORGRID_CELLSIZE >> 1)) +
	                   (target.x_maj * DEFAULT_MAJORGRID_CELLSIZE);
	int32_t screen_y = ((target.y / DEFAULT_MINORGRID_FIXED_POINT) +
	                    (DEFAULT_MAJORGRID_CELLSIZE >> 1)) +
	                   (target.y_maj * DEFAULT_MAJORGRID_CELLSIZE);
	return (SDL_FPoint) {screen_x, screen_y};
}

/* Converts a given screenspace position in SDL_FPoint
format to a position using a given camera transform
SDL_FPoint target: screenspace position to convert
Camera* camm: reference to the camera transform used */
Position Eng_get_world_pos(SDL_FPoint target, Camera* cam) {
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

Position Eng_pointf_to_position(SDL_FPoint target) {
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

	return (Position) {x, y, x_maj, y_maj};
}
/* Scale the width and height components of a given
SDL_FRect to a given factor (for example camera zoom)
SDL_FRect target: rectangle to scale float scale:
factor to multiply by*/
SDL_FRect Eng_frect_scale(SDL_FRect target, float scale) {
	return (SDL_FRect) {target.x, target.y, target.w * scale, target.h * scale};
}

/* Adds two positions ontop of another, handling overflow
 from minor to major
 * grid.
 Position a: first summand
 Position b: second summand */
Position Eng_position_add(Position a, Position b) {
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

/* Adds an SDL_FPoint ontop of a position. Note that this
function does NOT do screenspace conversion on the
SDL_FPoint. Position a: position to add ontop of SDL_FPoint
b: vector2 to add ontop of the position */
Position Eng_position_add_pointf(Position a, SDL_FPoint b) {
	Position b_pos = Eng_pointf_to_position(
		Eng_pointf_add_value(b, (float) (DEFAULT_MAJORGRID_CELLSIZE >> 1))
	);
	return Eng_position_add(a, b_pos);
}

Position Eng_position_invert(Position target) {
	return (Position) {-target.x, -target.y, -target.x_maj, -target.y_maj};
}

Position Eng_position_subtract(Position minuend, Position subtrahend) {
	return Eng_position_add(minuend, Eng_position_invert(subtrahend));
}

float Eng_get_distance(Position from, Position to) {
	return Eng_pointf_length(Eng_pointf_subtract(
		Eng_position_to_pointf(to), Eng_position_to_pointf(from)
	));
}

// GameObject management
// =======================================================

/*
Register and allocate a GameObject
void* src:          Give desired default values for data in
struct of given type size_t data_size:   Give size of
reserved space for GameObject uint32_t type:      Give type
of GameObject for recognition
 */
Error Eng_create_object(
	void* src, void** new_ref, size_t data_size, uint32_t type
) {
	if(!src) {
		SDL_Log(
			CODE_ERROR "ERROR: Tried to create GameObject "
					   "of type %d with null "
					   "data: %p" CODE_END,
			type, src
		);
		return ERR_PASS;
	}

	if(game_objects_len + 1 > game_objects_cap) {
		GameObject* tmp = reallocarray(
			game_objects, game_objects_cap * 2, sizeof(GameObject)
		);
		if(!tmp) {
			SDL_Log(
				CODE_ERROR "ERROR: Failed to allocate "
						   "memory for GameObject "
						   "queue expansion from "
						   "%d to %d" CODE_END,
				game_objects_cap, game_objects_cap * 2
			);
			return ERR_FATAL;
		}
		game_objects = tmp;
		game_objects_cap *= 2;
	}
	void* data = malloc(data_size);

	if(!data) {
		SDL_Log(
			CODE_ERROR "ERROR: Failed to allocate memory "
					   "for GameObject of "
					   "type %d" CODE_END,
			type
		);
		return ERR_FATAL;
	}

	memcpy(data, src, data_size);
	game_objects[game_objects_len] = (GameObject) {type, data};
	*new_ref                       = data;
	game_objects_len++;
	return ERR_PASS;
}

/*
Unregister and deallocate a GameObject
uint32_t target: Give uint32 to index target GameObject in
game_objects array
 */
Error Eng_destroy_object(uint32_t target) {
	if(target >= game_objects_len) {
		SDL_Log(
			CODE_ERROR "ERROR: Tried to destroy invalid "
					   "GameObject %d: Index out of "
					   "range" CODE_END,
			target
		);
		return ERR_PASS;
	}

	if(!game_objects[target].data) {
		SDL_Log(
			CODE_ERROR "ERROR. Tried to destroy GameObject %d with "
					   "null data" CODE_END,
			target
		);
		return ERR_PASS;
	}
	free(game_objects[target].data);

	if(target != game_objects_len - 1) {
		game_objects[target] = game_objects[game_objects_len - 1];
	}
	game_objects_len--;
	return ERR_PASS;
}

/*
Register a callback to be called at Eng_TickOnce()
Method func: Give callback function pointer according to
Method signature void* data:  Give optional varargs as
argument to callback
*/
Error Eng_hook_update(Method func, void* data) {
	if(!func) {
		SDL_Log(
			CODE_ERROR "ERROR: Tried to hook UpdateHook "
					   "callback with null "
					   "function pointer" CODE_END
		);
		return ERR_PASS;
	}

	if(update_callbacks_len + 1 > update_callbacks_cap) {
		UpdateHook* tmp = reallocarray(
			update_callbacks, update_callbacks_cap * 2, sizeof(UpdateHook)
		);
		if(!tmp) {
			SDL_Log(
				CODE_ERROR "ERROR: Failed to allocate "
						   "memory for UpdateHook "
						   "queue expansion"
						   "from %d to %d" CODE_END,
				update_callbacks_cap, update_callbacks_cap * 2
			);
			return ERR_FATAL;
		}
		update_callbacks = tmp;
		update_callbacks_cap *= 2;
	}

	update_callbacks[update_callbacks_len] = (UpdateHook) {func, data};
	update_callbacks_len++;
	return ERR_PASS;
}

/*
Unregister a callback, so it is no longer called on
Eng_TickOnce() void* data: Give GameObject.data pointer of
target GameObject
 */
Error Eng_unhook_update(void* data) {
	if(!data) {
		SDL_Log(
			CODE_ERROR "ERROR: Tried to unhook null "
					   "Update callback" CODE_END
		);
		return ERR_PASS;
	}

	for(uint32_t i = 0; i < update_callbacks_len; i++) {
		if(data == update_callbacks[i].argv) {
			if(i != update_callbacks_len - 1) {
				update_callbacks[i] =
					update_callbacks[update_callbacks_len - 1];
			}
			update_callbacks_len--;

			if(update_callbacks_len < update_callbacks_cap >> 1) {
				UpdateHook* tmp = reallocarray(
					update_callbacks, update_callbacks_cap >> 1,
					sizeof(UpdateHook)
				);

				if(!tmp) {
					SDL_Log(
						CODE_ERROR "ERROR: Failed to allocate memory "
								   "for UpdateHook queue "
								   "shrinking from %d to %d" CODE_END,
						update_callbacks_cap, update_callbacks_cap >> 1
					);
					return ERR_FATAL;
				}

				update_callbacks     = tmp;
				update_callbacks_cap = update_callbacks_cap >> 1;
			}
			return ERR_PASS;
		}
	}
	SDL_Log(
		CODE_ERROR "ERROR: Tried to unhook invalid Update "
				   "callback: %p" CODE_END,
		data
	);
	return ERR_PASS;
}

// Vector math
// =================================================================

/* Add the components of two vector2s together
   SDL_FPoint a: First summand
   SDL_FPoit b: Second summand*/
SDL_FPoint Eng_pointf_add(SDL_FPoint a, SDL_FPoint b) {
	return (SDL_FPoint) {a.x + b.x, a.y + b.y};
}

SDL_FPoint Eng_pointf_add_value(SDL_FPoint a, float b) {
	return (SDL_FPoint) {a.x + b, a.y + b};
}

SDL_FPoint Eng_pointf_invert(SDL_FPoint target) {
	return (SDL_FPoint) {-target.x, -target.y};
}

SDL_FPoint Eng_pointf_subtract(SDL_FPoint minuend, SDL_FPoint subtrahend) {
	return Eng_pointf_add(minuend, Eng_pointf_invert(subtrahend));
}

/* Rotates a vector around its origin point by a given
 * amount of degrees clockwise with 0° at -y (North)
 * SDL_FPoint a: target vector2
 * float deg: angle to rotate by
 */
SDL_FPoint Eng_pointf_rotate(SDL_FPoint a, float deg) {
	const float x = (a.x * cos(deg * DEG2RAD)) - (a.y * sin(deg * DEG2RAD));
	const float y = (a.x * sin(deg * DEG2RAD)) + (a.y * cos(deg * DEG2RAD));
	return (SDL_FPoint) {x, y};
}

/* Set the lengths of a vector2 to 1 while preserving the
 * rotation SDL_FPoint a: target vector2
 */
SDL_FPoint Eng_pointf_normalize(SDL_FPoint a) {
	const float length = sqrt((a.x * a.x) + (a.y * a.y));
	if(length <= SDL_FLT_EPSILON) return a;
	const float x = a.x / length;
	const float y = a.y / length;
	return (SDL_FPoint) {x, y};
}

/* Multiply the components of a vector2 by a given scale
 * multiplier SDL_FPoint a: target Vector2 double scale:
 * multiplier */
SDL_FPoint Eng_pointf_scale(SDL_FPoint a, double scale) {
	if(scale == SDL_FLT_EPSILON || Eng_pointf_length(a) <= SDL_FLT_EPSILON)
		return a;
	float x = a.x * scale;
	float y = a.y * scale;
	return (SDL_FPoint) {x, y};
}

/* Generate a vector2 with a given length or rotation,
 * shorthand for pointf_scale(pointf_rotate(vec, rot))
 * float magnitude: the length of the Vector float
 * rotation: the rotation in degrees, running clockwise and
 * centered at -y (North) */
SDL_FPoint Eng_pointf_force(float magnitude, float rotation) {
	SDL_FPoint vec = (SDL_FPoint) {0.0f, -1.0};
	vec            = Eng_pointf_scale(vec, magnitude);
	vec            = Eng_pointf_rotate(vec, rotation);
	return vec;
}

/* Get the length of a given vector2
 * SDL_FPoint a: the target vector2 to get the length of */
float Eng_pointf_length(SDL_FPoint a) {
	const float length = sqrt((a.x * a.x) + (a.y * a.y));
	if(length <= SDL_FLT_EPSILON) return 0.0f;
	return length;
}

/* Get the direction to a certain vector2 from another
 * vector2 on the compass scale (360°, 0° at -y) SDL_FPoint
 * zero: The reference point, where the bearing line is
 * drawn from SDL_FPoint tgt: The target point, where the
 * bearing line will be drawn two */
float Eng_pointf_bearing(SDL_FPoint zero, SDL_FPoint tgt) {
	float angle = atan2(tgt.x - zero.x, zero.y - tgt.y) * RAD2DEG;
	if(angle < 0) angle += 360;
	return angle;
}

/* Returns the factor to multiply a time-based value by in
 * order to compensate for frame disparities. For example,
 * if the value is 1.1, the previous frame was finished in
 * 110% of the time it should have. Multiply this value by
 * the animation speed, for example, and the visual
 * movement will stay uniform across framerates. */
double Eng_get_deltatime_factor(void) {
	return ((double) (1'000'000'000) / Eng_desired_fps) / last_frame_time;
}

#endif
