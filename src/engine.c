#pragma once

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#include "utils.c"
#include <SDL3_ttf/SDL_ttf.h>

// Title of the application the program should use
#define APPLICATION_TITLE "Asteroidssssssssssssssss+"

// DynArr default capacities
#define DEFAULT_GAMEOBJECT_QUEUE_CAP 32
#define DEFAULT_UPDATECALLBACK_QUEUE_CAP 32
#define DEFAULT_TOASTQUEUE_SIZE 8
#define DEFAULT_COLTREE_SIZE 16
#define DEFAULT_FPS_TEXTBUFFER_SIZE 64

// Default window size on startup
#define DEFAULT_SCREENWIDTH 1280
#define DEFAULT_SCREENHEIGHT 720
// -----------------------------------------------------------------------------
extern SDL_Point Eng_screensize;

// Frame handling
#define DEFAULT_VSYNC_ENABLE
#define DEFAULT_FPS 60
// -----------------------------------------------------------------------------
extern uint32_t Eng_desired_fps;
extern uint32_t Eng_current_fps;
// -----------------------------------------------------------------------------
double Eng_get_deltatime_factor(void);

// Text handling
#define DEFAULT_FONTSIZE 12.0f
// -----------------------------------------------------------------------------
extern TTF_Font*       Eng_font;
extern TTF_TextEngine* Eng_text_engine;
extern const char      EMB_IOSEVKA_FONT[];

/* -----------------------------------------------------------------------------
|  IMPLEMENTATION                                                              |
----------------------------------------------------------------------------- */

// Control flow
Error             Eng_init(void);
[[noreturn]] void Eng_exit(void);
Error             Eng_input_update(SDL_Event* event);
void              Eng_input_deferred();
Error             Eng_update_frame(void);

// Collision system
typedef struct {
	Vector2l pos;
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
extern ColTree Eng_std_collision_tree;
// -----------------------------------------------------------------------------
Error Eng_init_coltree(ColTree* dest);
Error Eng_register_hitbox(
	Vector2l pos, Vector2f size, void* owner, uint32_t typeof_owner,
	ColRect** dest, ColTree* in
);
Error   Eng_unregister_hitbox(ColRect* target, ColTree* in);
Error   Eng_update_hitbox(ColRect* target, Vector2l* pos, Vector2f* size);
ColInfo Eng_get_collision(ColRect* target, ColTree* in);

// Debug stuff
typedef struct {
	SDL_FPoint pos;
	TTF_Text*  display;
} DebugMenu;
// -----------------------------------------------------------------------------
extern DebugMenu Eng_std_debug_menu;
extern bool      Eng_debug_vis;
// -----------------------------------------------------------------------------
void update_debug_menu(DebugMenu* data);

// GameObject management
#define GAMEOBJECT_CREATE_SUCCESS "INFO: Successfully created GameObject"
#define GAMEOBJECT_CREATE_FAILURE "FATAL: Failed to create GameObject"
// -----------------------------------------------------------------------------
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
void* Eng_get_gameobject(uint32_t type, int32_t index);

// Input handling
#define KEYS                                                                   \
	X(W)                                                                       \
	X(A)                                                                       \
	X(S)                                                                       \
	X(D)                                                                       \
	X(LALT) X(I) X(J) X(K) X(L) X(MINUS) X(PLUS) X(F3) X(RETURN) X(1) X(2) X(3)
// -----------------------------------------------------------------------------
enum KeyStates : uint8_t {
	KEY_DOWN,
	KEY_PRESSED,
	KEY_RELEASED,
};
enum Keys : uint32_t {
#define X(x) KEY_##x,
	KEYS
#undef X
		KEY_MOUSE_LEFT,
	KEY_MOUSE_RIGHT,
	KEY_NUM,
};
// -----------------------------------------------------------------------------
#define Eng_get_key_down(key) (Eng_key_cache[key] & (1 << KEY_DOWN))
#define Eng_get_key_pressed(key) (Eng_key_cache[key] & (1 << KEY_PRESSED))
#define Eng_get_key_released(key) (Eng_key_cache[key] & (1 << KEY_RELEASED))
// -----------------------------------------------------------------------------
extern uint8_t    Eng_key_cache[KEY_NUM];
extern SDL_FPoint Eng_mouse_pos;

// Main rendering interface
extern SDL_Window*   window;
extern SDL_Renderer* renderer;

// Camera system
extern Camera Eng_std_camera;

#if __INCLUDE_LEVEL__ == 0 /////////////////////////////////////////////////////

#include "gameobjects.c"
#include "repl.c"
#include "res.c"

#include <inttypes.h>
#include <stdlib.h>
#include <unistd.h>

SDL_Point Eng_screensize = {DEFAULT_SCREENWIDTH, DEFAULT_SCREENHEIGHT};

static uint64_t last_frame_time = 1;
uint32_t        Eng_desired_fps = 60;
uint32_t        Eng_current_fps = 1;

static uint16_t fontsize;
TTF_Font*       Eng_font;
TTF_TextEngine* Eng_text_engine;
const char EMB_IOSEVKA_FONT[] = {
#embed "../res/Iosevka-Regular.ttf"
};

ColTree Eng_std_collision_tree;

DebugMenu Eng_std_debug_menu;
#ifndef NDEBUG
bool Eng_debug_vis = true;
#else
bool Eng_debug_vis = false;
#endif

static GameObject* game_objects     = {0};
static uint32_t    game_objects_len = 0;
static uint32_t    game_objects_cap = DEFAULT_GAMEOBJECT_QUEUE_CAP;

static UpdateHook* update_callbacks     = {0};
static uint32_t    update_callbacks_len = 0;
static uint32_t    update_callbacks_cap = DEFAULT_UPDATECALLBACK_QUEUE_CAP;


uint8_t    Eng_key_cache[KEY_NUM] = {0};
SDL_FPoint Eng_mouse_pos          = {0};

SDL_Window*   window;
SDL_Renderer* renderer;

Camera Eng_std_camera;

// Frame handling
double Eng_get_deltatime_factor(void) {
	return last_frame_time / ((double) 1'000'000'000 / Eng_desired_fps);
}

// Control flow ================================================================

/* Initialize the engine, required for… well… everything in
 * it to work */
Error Eng_init(void) {
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
#else
	(void) texture_failed;
#endif

	// Try setup ColTree
	ASSERT_PREDICATE(
		Eng_init_coltree(&Eng_std_collision_tree), fatal_error = true;
		, CODE_SUCCESS "INFO: Succesfully initialized ColTree" CODE_END,
		CODE_ERROR "FATAL: Failed to initialize ColTree" CODE_END
	);

	// Try setup DebugRepl
	ASSERT_PREDICATE(
		Repl_init(), fatal_error = true;
		, CODE_SUCCESS "INFO: Successfully initialized DebugRepl" CODE_END,
		CODE_ERROR "FATAL: Failed to initialize DebugRepl" CODE_END
	);

	// Try setup DebugMenu
	Eng_std_debug_menu =
		(DebugMenu) {.display = NULL, .pos = (SDL_FPoint) {20.0f, 20.0f}};
	ASSERT_PREDICATE(
		(Eng_std_debug_menu.display =
	         TTF_CreateText(Eng_text_engine, Eng_font, "hewo :3", 0)),
		fatal_error = true;
		, CODE_SUCCESS "INFO: Successfully initialized DebugMenu" CODE_END,
		CODE_ERROR "FATAL: Failed to initialize DebugMenu" CODE_END
	);

	// Arbitrary initializations
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	Eng_std_camera = (Camera) {(Vector2l) {0, 0}, 1.0f, 1, Eng_screensize};

	ASSERT_PREDICATE(!fatal_error, Eng_exit();
	                 ,
	                 CODE_SUCCESS "INFO: Green across the board, "
	                              "launching…" CODE_END,
	                 CODE_ERROR "FATAL: Caught one or more fatal "
	                            "exceptions, aborting…" CODE_END);
	SDL_Log("INFO: Welcome to " APPLICATION_TITLE "!");
	return true;
}

/* Runs at the end of the program. */
[[noreturn]] void Eng_exit(void) {
	SDL_Log("INFO: Shutting down…");
	_exit(0);
}

/*
Run all input capturing events, return value MUST be
returned from SDL_AppEvent() SDL_Event* event: Pass
SDL_Event* from SDL_AppEvent()
 */
Error Eng_input_update(SDL_Event* event) {
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
			Eng_exit();
			break;

#define X(x)                                                                   \
	case SDLK_##x:                                                             \
		Eng_key_cache[KEY_##x] |= (1 << KEY_DOWN);                             \
		Eng_key_cache[KEY_##x] |= (1 << KEY_PRESSED);                          \
		Eng_key_cache[KEY_##x] &= ~(1 << KEY_RELEASED);                        \
		break;
			KEYS
#undef X
		}
		break;

	case SDL_EVENT_KEY_UP:
		switch(event->key.key) {
#define X(x)                                                                   \
	case SDLK_##x:                                                             \
		Eng_key_cache[KEY_##x] &= ~(1 << KEY_DOWN);                            \
		Eng_key_cache[KEY_##x] &= ~(1 << KEY_PRESSED);                         \
		Eng_key_cache[KEY_##x] |= (1 << KEY_RELEASED);                         \
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
		Eng_exit();
	}

	if(Eng_get_key_down(KEY_F3)) Eng_debug_vis = !Eng_debug_vis;

	return true;
}

void Eng_input_deferred(void) {
	for(uint32_t i = 0; i < KEY_NUM; i++) {
		Eng_key_cache[i] &= ~(1 << KEY_PRESSED);
		Eng_key_cache[i] &= ~(1 << KEY_RELEASED);
	}
}

/*
Process all callbacks in the update_callbacks queue
*/
Error Eng_update_frame(void) {
	// Start frame timer
	const uint64_t frametime_start = SDL_GetTicksNS();

	// Check for new input events
	SDL_Event cur_event = {0};
	while(SDL_PollEvent(&cur_event)) {
		Eng_input_update(&cur_event);
	}

	if(Eng_get_key_down(KEY_W)) {
		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		SDL_FRect rect = {
			50, 50, Eng_screensize.x - 100, Eng_screensize.y - 100
		};
		SDL_RenderRect(renderer, &rect);
	}

	// Grey background
	SDL_SetRenderDrawColor(renderer, 39, 36, 43, 255);
	SDL_RenderClear(renderer);

	if(Eng_get_key_down(KEY_MINUS)) {
		Eng_std_camera.zoom_factor =
			clamp(INT8_MIN, INT8_MAX, Eng_std_camera.zoom_factor - 1);
		Eng_std_camera.zoom = 1 * pow(1.1, Eng_std_camera.zoom_factor);
	}
	if(Eng_get_key_down(KEY_PLUS)) {
		Eng_std_camera.zoom_factor =
			clamp(INT8_MIN, INT8_MAX, Eng_std_camera.zoom_factor + 1);
		Eng_std_camera.zoom = 1 * pow(1.1, Eng_std_camera.zoom_factor);
	}

	for(uint32_t i = 0; i < update_callbacks_len; i++) {
		if(update_callbacks[i].func(update_callbacks[i].argv, i) == false) {
			return false;
		}
	}

	// Draw debug menu if debug is visible
	update_debug_menu(&Eng_std_debug_menu);

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

	// Process DebugRepl
	if(SDL_TryWaitSemaphore(Repl_repl.semaphore)) {
		switch(Repl_repl.command) {
		case COMMAND_EXIT:
			SDL_Log("INFO: Received exit command from DebugRepl, exiting…");
			Eng_exit();
		case COMMAND_SPAWN_ASTEROID:
			SDL_Log("INFO: Spawning asteroid above camera target");
			ASSERT_PREDICATE(
				GameObject_asteroid_create(NULL), return false;
				,
				CODE_SUCCESS
				"INFO: Successfully created GameObject asteroid" CODE_END,
				CODE_ERROR
				"FATAL: Failed to create GameObject asteroid" CODE_END
			);
		}
	}

	// Clear bitflags on input key_cache
	Eng_input_deferred();

	return true;
}

// Collision system ============================================================

Error Eng_init_coltree(ColTree* dest) {
	dest->arr = calloc(DEFAULT_COLTREE_SIZE, sizeof(ColRect));
	ASSERT_PREDICATE(
		dest->arr, return false;
		,
		CODE_SUCCESS
		"INFO: Successfully allocated memory for collision tree" CODE_END,
		CODE_ERROR
		"FATAL: Failed to allocate memory for collision tree" CODE_END
	);
	dest->cap = DEFAULT_COLTREE_SIZE;
	dest->len = 0;

	return true;
}

Error Eng_register_hitbox(
	Vector2l pos, Vector2f size, void* owner, uint32_t typeof_owner,
	ColRect** dest, ColTree* in
) {
	ColRect data = {
		.pos = pos, .size = size, .owner = owner, .typeof_owner = typeof_owner
	};

	if(in->len + 1 > in->cap) {
		ColRect* tmp = reallocarray(in->arr, in->cap * 2, sizeof(*in->arr));
		ASSERT_PREDICATE(
			tmp, return false;
			, CODE_SUCCESS "INFO: Successfully expanded ColTree" CODE_END,
			CODE_ERROR "FATAL: Failed to expand Coltree" CODE_END
		);
		in->arr = tmp;
	}
	in->arr[in->len] = data;
	*dest            = &in->arr[in->len];
	in->len++;

	return true;
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
					tmp, return false;
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
		CODE_WARN
		"WARNING: Could not find target ColRect %p in ColTree %p" CODE_END,
		(void*) target, (void*) in
	);

	return true;
}

Error Eng_update_hitbox(ColRect* target, Vector2l* pos, Vector2f* size) {
	target->pos  = (pos) ? *pos : target->pos;
	target->size = (size) ? *size : target->size;
	if(Eng_debug_vis) {
		SDL_FPoint pos    = {0};
		SDL_FRect  dest   = {0, 0, target->size.x, target->size.y};
		SDL_FPoint origin = {0, 0};
		Cam_transform(&target->pos, &pos, &dest, &origin, &Eng_std_camera);
		dest.x -= dest.w / 2;
		dest.y -= dest.h / 2;

		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		SDL_RenderRect(renderer, &dest);
	}
	return true;
}

ColInfo Eng_get_collision(ColRect* target, ColTree* in) {
	for(uint16_t i = 0; i < in->len; i++) {
		ColRect* cur = &in->arr[i];
		if(colrect_check_collision(
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

// Debug stuff =================================================================
void update_debug_menu(DebugMenu* data) {
	if(Eng_debug_vis) {
		char                      fps_string[256] = {0};
		struct GameObject_Player* player =
			Eng_get_gameobject(GAMEOBJECT_PLAYER, 0);
		snprintf(
			fps_string, sizeof(fps_string),
			"FPS: %d\nCam Pos: %" PRId64 " %" PRId64
			"\nGameObjects loaded: %d, Updates scheduled: %d\nPlayer modules: "
			"%b",
			Eng_current_fps, Eng_std_camera.target.x / DEFAULT_FIXED_POINT,
			Eng_std_camera.target.y / DEFAULT_FIXED_POINT, game_objects_len,
			update_callbacks_len, (player) ? player->modules : 0
		);
		TTF_SetTextString(data->display, fps_string, sizeof(fps_string));

		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		TTF_DrawRendererText(data->display, data->pos.x, data->pos.y);
	}
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
		return true;
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
			return false;
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
		return false;
	}

	memcpy(data, src, data_size);
	game_objects[game_objects_len] = (GameObject) {type, data};
	*new_ref                       = data;
	game_objects_len++;
	return true;
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
		return true;
	}

	if(!game_objects[target].data) {
		SDL_Log(
			CODE_ERROR "ERROR. Tried to destroy GameObject %d with "
					   "null data" CODE_END,
			target
		);
		return true;
	}
	free(game_objects[target].data);

	if(target != game_objects_len - 1) {
		game_objects[target] = game_objects[game_objects_len - 1];
	}
	game_objects_len--;
	return true;
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
		return true;
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
			return false;
		}
		update_callbacks = tmp;
		update_callbacks_cap *= 2;
	}

	update_callbacks[update_callbacks_len] = (UpdateHook) {func, data};
	update_callbacks_len++;
	return true;
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
		return true;
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
					return false;
				}

				update_callbacks     = tmp;
				update_callbacks_cap = update_callbacks_cap >> 1;
			}
			return true;
		}
	}
	SDL_Log(
		CODE_ERROR "ERROR: Tried to unhook invalid Update "
				   "callback: %p" CODE_END,
		data
	);
	return true;
}

void* Eng_get_gameobject(uint32_t type, int32_t index) {
	for(uint32_t i = 0; i < game_objects_len; i++) {
		if(game_objects[i].type == type) {
			index--;
			if(index < 0) return game_objects[i].data;
		}
	}
	SDL_Log(
		CODE_WARN
		"ERROR: Failed to find component of type %u at index %u" CODE_END,
		type, index
	);
	return NULL;
}

#endif
