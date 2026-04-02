#pragma once

#define _DEFAULT_SOURCE
#define ERR_FATAL false
#define ERR_PASS true

#define RAD2DEG (180 / M_PI)
#define DEG2RAD (1 / RAD2DEG)
#define WRAP_COMPASS(x) ((x % 360) + 360) % 360
#define PROPER_MOD(x, mod) ((x % mod) + mod) % mod

#define DEFAULT_GAMEOBJECT_QUEUE_CAP 32
#define DEFAULT_UPDATECALLBACK_QUEUE_CAP 32
#define DEFAULT_SCREENWIDTH 1280
#define DEFAULT_SCREENHEIGHT 720

typedef bool Error;

#include <SDL3/SDL.h>
#include <SDL3_gfx/SDL3_gfxPrimitives.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <math.h>
#include <stdlib.h>

#include <endian.h>
#include <stdio.h>

// Control flow
SDL_AppResult Eng_Init(void);
SDL_AppResult Eng_TickInput(SDL_Event* event);
Error         Eng_TickOnce(void);

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
SDL_FPoint pointf_add(SDL_FPoint a, SDL_FPoint b);
SDL_FPoint pointf_rotate(SDL_FPoint a, float deg);
SDL_FPoint pointf_normalize(SDL_FPoint a);
SDL_FPoint pointf_scale(SDL_FPoint a, double scale);
SDL_FPoint pointf_angle_to(SDL_FPoint a, SDL_FPoint b);
SDL_FPoint pointf_force(float magnitude, float rotation);
float      pointf_length(SDL_FPoint a);
float      pointf_bearing(SDL_FPoint zero, SDL_FPoint tgt);

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

#include "main.c"

#if __INCLUDE_LEVEL__ == 0 /////////////////////////////////////////////////////

static GameObject* game_objects     = {0};
static uint32_t    game_objects_len = 0;
static uint32_t    game_objects_cap = DEFAULT_GAMEOBJECT_QUEUE_CAP;

static UpdateHook* update_callbacks     = {0};
static uint32_t    update_callbacks_len = 0;
static uint32_t    update_callbacks_cap = DEFAULT_UPDATECALLBACK_QUEUE_CAP;

// KEY input handling BEGIN

bool       Eng_key_cache[KEY_NUM] = {0};
SDL_FPoint Eng_mouse_pos          = {0};

// KEY input handling END

SDL_Point Eng_screensize = {DEFAULT_SCREENWIDTH, DEFAULT_SCREENHEIGHT};

// Control flow ================================================================

SDL_AppResult Eng_Init(void) {
	if((game_objects =
	        calloc(DEFAULT_GAMEOBJECT_QUEUE_CAP, sizeof(game_objects[0]))) ==
	   NULL)
		return SDL_APP_FAILURE;
	if((update_callbacks = calloc(
			DEFAULT_UPDATECALLBACK_QUEUE_CAP, sizeof(update_callbacks[0])
		)) == NULL)
		return SDL_APP_FAILURE;
	return SDL_APP_CONTINUE;
}

/*
Run all input capturing events, return value MUST be returned from
SDL_AppEvent()
SDL_Event* event: Pass SDL_Event* from SDL_AppEvent()
 */
SDL_AppResult Eng_TickInput(SDL_Event* event) {
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
		// If system sends quit event (e.g. closing the window from the OS)
		return SDL_APP_SUCCESS;
	}

	return SDL_APP_CONTINUE;
}

/*
Process all callbacks in the update_callbacks queue
*/
Error Eng_TickOnce(void) {
	// Grey background
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	for(uint32_t i = 0; i < update_callbacks_len; i++) {
		if(update_callbacks[i].func(update_callbacks[i].argv, i) == ERR_FATAL) {
			return ERR_FATAL;
		}
	}
	return ERR_PASS;
}

// GameObject management =======================================================

/*
Register and allocate a GameObject
void* src:          Give desired default values for data in struct of given type
size_t data_size:   Give size of reserved space for GameObject
uint32_t type:      Give type of GameObject for recognition
 */
Error Eng_create_object(
	void* src, void** new_ref, size_t data_size, uint32_t type
) {
	if(!src) {
		SDL_Log(
			"Tried to create GameObject of type %d with null data: %p", type,
			src
		);
		return ERR_PASS;
	}

	if(game_objects_len + 1 > game_objects_cap) {
		GameObject* tmp = reallocarray(
			game_objects, game_objects_cap * 2, sizeof(GameObject)
		);
		if(!tmp) {
			SDL_Log(
				"Failed to allocate memory for GameObject queue expansion from "
				"%d to %d",
				game_objects_cap, game_objects_cap * 2
			);
			return ERR_FATAL;
		}
		game_objects = tmp;
		game_objects_cap *= 2;
	}
	void* data = malloc(data_size);

	if(!data) {
		SDL_Log("Failed to allocate memory for GameObject of type %d", type);
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
uint32_t target: Give void* to address of target GameObject.data
 */
Error Eng_destroy_object(uint32_t target) {
	if(target >= game_objects_len) {
		SDL_Log(
			"Tried to destroy invalid GameObject %d: Index out of range", target
		);
		return ERR_PASS;
	}

	if(!game_objects[target].data) {
		SDL_Log("Tried to destroy GameObject %d with null data", target);
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
Method func: Give callback function pointer according to Method signature
void* data:  Give optional varargs as argument to callback
*/
Error Eng_hook_update(Method func, void* data) {
	if(!func) {
		SDL_Log("Tried to hook UpdateHook callback with null function pointer");
		return ERR_PASS;
	}

	if(update_callbacks_len + 1 > update_callbacks_cap) {
		UpdateHook* tmp = reallocarray(
			update_callbacks, update_callbacks_cap * 2, sizeof(UpdateHook)
		);
		if(!tmp) {
			SDL_Log(
				"Failed to allocate memory for UpdateHook queue expansion"
				"from %d to %d",
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
Unregister a callback, so it is no longer called on Eng_TickOnce()
void* data: Give GameObject.data pointer of target GameObject
 */
Error Eng_unhook_update(void* data) {
	if(!data) {
		SDL_Log("Tried to unhook null Update callback");
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
						"Failed to allocate memory for UpdateHook queue "
						"shrinking from %d to %d",
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
	SDL_Log("Tried to unhook invalid Update callback: %p", data);
	return ERR_PASS;
}

// Vector math =================================================================

SDL_FPoint pointf_add(SDL_FPoint a, SDL_FPoint b) {
	return (SDL_FPoint) {a.x + b.x, a.y + b.y};
}

SDL_FPoint pointf_rotate(SDL_FPoint a, float deg) {
	const float x = (a.x * cos(deg * DEG2RAD)) - (a.y * sin(deg * DEG2RAD));
	const float y = (a.x * sin(deg * DEG2RAD)) + (a.y * cos(deg * DEG2RAD));
	return (SDL_FPoint) {x, y};
}

SDL_FPoint pointf_normalize(SDL_FPoint a) {
	const float length = sqrt((a.x * a.x) + (a.y * a.y));
	if(length <= SDL_FLT_EPSILON) return a;
	const float x = a.x / length;
	const float y = a.y / length;
	return (SDL_FPoint) {x, y};
}

SDL_FPoint pointf_scale(SDL_FPoint a, double scale) {
	if(scale == SDL_FLT_EPSILON || pointf_length(a) <= SDL_FLT_EPSILON)
		return a;
	float x = a.x * scale;
	float y = a.y * scale;
	return (SDL_FPoint) {x, y};
}

SDL_FPoint pointf_force(float magnitude, float rotation) {
	SDL_FPoint vec = (SDL_FPoint) {0.0f, -1.0};
	vec            = pointf_scale(vec, magnitude);
	vec            = pointf_rotate(vec, rotation);
	return vec;
}

float pointf_length(SDL_FPoint a) {
	const float length = sqrt((a.x * a.x) + (a.y * a.y));
	if(length <= SDL_FLT_EPSILON) return 0.0f;
	return length;
}

float pointf_bearing(SDL_FPoint zero, SDL_FPoint tgt) {
	float angle = atan2(tgt.x - zero.x, zero.y - tgt.y) * RAD2DEG;
	if(angle < 0) angle += 360;
	return angle;
}

#endif
