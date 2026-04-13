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
#define WRAP_COMPASS(x) PROPER_MOD(x, 360)
#define PROPER_MOD(x, mod) (((x % mod) + mod) % mod)

#define DEFAULT_GAMEOBJECT_QUEUE_CAP 32
#define DEFAULT_UPDATECALLBACK_QUEUE_CAP 32
#define DEFAULT_SCREENWIDTH 1280
#define DEFAULT_SCREENHEIGHT 720
#define DEFAULT_VSYNC_ENABLE
#define DEFAULT_FPS 60
#define DEFAULT_FPS_TEXTBUFFER_SIZE 64
#define DEFAULT_FONTSIZE 12.0f
#define DEFAULT_COLTREE_SIZE 16

typedef bool Error;

#include <SDL3/SDL.h>
#include <SDL3_gfx/SDL3_gfxPrimitives.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdlib.h>

#include <endian.h>
#include <stdio.h>

#include "res.c"
#include "utils.c"

// Control flow
SDL_AppResult Eng_init(void);
void          Eng_exit(void);
SDL_AppResult Eng_tick_input(SDL_Event* event);
Error         Eng_tick_once(void);

// Collision system
typedef struct {
	Position pos;
	Vector2f size;
	void*    owner;
	uint32_t typeof_owner;
} ColRect;
typedef struct {
	ColRect* arr;
	uint16_t len;
	uint16_t cap;
} ColTree;
typedef struct {
	ColRect* collider;
	void*    owner;
	uint32_t typeof_owner;
	bool     collided;
} ColInfo;
// -----------------------------------------------------------------------------
Error Eng_init_coltree(ColTree* dest);
Error Eng_register_hitbox(
	Position pos, Vector2f size, void* owner, uint32_t typeof_owner,
	ColRect** dest, ColTree* in
);
Error   Eng_unregister_hitbox(ColRect* target, ColTree* in);
Error   Eng_update_hitbox(ColRect* target, Position* pos, Vector2f* size);
ColInfo Eng_get_collision(ColRect* target, ColTree* in);

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

// Misc

double Eng_get_deltatime_factor(void);

// KEY input handling BEGIN
#define KEYS                                                                   \
	X(W) X(A) X(S) X(D) X(LALT) X(I) X(J) X(K) X(L) X(MINUS) X(PLUS) X(F3)

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

extern ColTree Eng_std_collision_tree;
extern bool    Eng_debug_vis;

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

ColTree Eng_std_collision_tree;

#ifndef NDEBUG
bool Eng_debug_vis = true;
#else
bool Eng_debug_vis = false;
#endif

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

	// Try setup ColTree
	ASSERT_PREDICATE(
		Eng_init_coltree(&Eng_std_collision_tree), fatal_error = true;
		, CODE_SUCCESS "INFO: Succesfully initialized ColTree" CODE_END,
		CODE_ERROR "FATAL: Failed to initialize ColTree" CODE_END
	);

	// Arbitrary initializations
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	Eng_std_camera =
		(Camera) {Pos_easy_get_pos(0, 0, 0, 0), 1.0f, 1, Eng_screensize};

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

	if(Eng_get_key_pressed(KEY_F3)) Eng_debug_vis = !Eng_debug_vis;

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

	if(Eng_get_key_pressed(KEY_MINUS)) {
		Eng_std_camera.zoom_factor =
			clamp(INT8_MIN, INT8_MAX, Eng_std_camera.zoom_factor - 1);
		Eng_std_camera.zoom = 1 * pow(1.1, Eng_std_camera.zoom_factor);
	}
	if(Eng_get_key_pressed(KEY_PLUS)) {
		Eng_std_camera.zoom_factor =
			clamp(INT8_MIN, INT8_MAX, Eng_std_camera.zoom_factor + 1);
		Eng_std_camera.zoom = 1 * pow(1.1, Eng_std_camera.zoom_factor);
	}

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

// Collision system ============================================================

Error Eng_init_coltree(ColTree* dest) {
	dest->arr = calloc(DEFAULT_COLTREE_SIZE, sizeof(ColRect));
	ASSERT_PREDICATE(
		dest->arr, return ERR_FATAL;
		,
		CODE_SUCCESS
		"INFO: Successfully allocated memory for collision tree" CODE_END,
		CODE_ERROR
		"FATAL: Failed to allocate memory for collision tree" CODE_END
	);
	dest->cap = DEFAULT_COLTREE_SIZE;
	dest->len = 0;

	return ERR_PASS;
}

Error Eng_register_hitbox(
	Position pos, Vector2f size, void* owner, uint32_t typeof_owner,
	ColRect** dest, ColTree* in
) {
	ColRect data = {
		.pos = pos, .size = size, .owner = owner, .typeof_owner = typeof_owner
	};

	if(in->len + 1 > in->cap) {
		ColRect* tmp = reallocarray(in->arr, in->cap * 2, sizeof(*in->arr));
		ASSERT_PREDICATE(
			tmp, return ERR_FATAL;
			, CODE_SUCCESS "INFO: Successfully expanded ColTree" CODE_END,
			CODE_ERROR "FATAL: Failed to expand Coltree" CODE_END
		);
		in->arr = tmp;
	}
	in->arr[in->len] = data;
	*dest            = &in->arr[in->len];
	in->len++;

	return ERR_PASS;
}

Error Eng_unregister_hitbox(ColRect* target, ColTree* in) {
	for(uint16_t i = 0; i < in->len; i++) {
		if(&in->arr[i] == target) {
			if(i != in->len - 1) { in->arr[i] = in->arr[in->len - 1]; }
			in->len--;

			if(in->len <= in->cap >> 1 &&
			   in->cap >> 1 <= DEFAULT_COLTREE_SIZE) {
				ColRect* tmp =
					reallocarray(in->arr, in->cap >> 1, sizeof(*in->arr));
				ASSERT_PREDICATE(
					tmp, return ERR_FATAL;
					, CODE_SUCCESS "INFO: Successfully shrunk ColTree" CODE_END,
					CODE_ERROR "FATAL: Failed to shrink ColTree" CODE_END
				);
				in->arr = tmp;
				in->cap /= 2;
			}
			break;
		}
	}

	SDL_Log(
		"WARNING: Could not find target ColRect %p in ColTree %p",
		(void*) target, (void*) in
	);

	return ERR_PASS;
}

Error Eng_update_hitbox(ColRect* target, Position* pos, Vector2f* size) {
	target->pos  = (pos) ? *pos : target->pos;
	target->size = (size) ? *size : target->size;
	if(Eng_debug_vis) {
		SDL_FPoint pos    = {0};
		SDL_FRect  dest   = {0, 0, target->size.x, target->size.y};
		SDL_FPoint origin = {0, 0};
		Pos_cam_transform(&target->pos, &pos, &dest, &origin, &Eng_std_camera);
		dest.x -= dest.w / 2;
		dest.y -= dest.h / 2;

		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		SDL_RenderRect(renderer, &dest);
	}
	return ERR_PASS;
}

ColInfo Eng_get_collision(ColRect* target, ColTree* in) {
	for(uint16_t i = 0; i < in->len; i++) {
		ColRect* cur = &in->arr[i];
		if(Pos_check_collision(
			   target->pos, target->size, cur->pos, cur->size
		   )) {
			ColInfo data = {
				.collider     = cur,
				.owner        = cur->owner,
				.typeof_owner = cur->typeof_owner,
				.collided     = true,
			};
			return data;
		}
	}

	return (ColInfo) {0};
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

// Misc

double Eng_get_deltatime_factor(void) {
	return last_frame_time / ((double) 1'000'000'000 / Eng_desired_fps);
}

#endif
