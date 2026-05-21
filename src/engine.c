#pragma once

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#include "utils.c"
#include <SDL3_ttf/SDL_ttf.h>
#include <sys/types.h>

// True to quit game
extern bool die;

// Title of the application the program should use
#define APPLICATION_TITLE "Asteroidssssssssssssssss+"

// Default window size on startup
#define DEFAULT_SCREENWIDTH 1280
#define DEFAULT_SCREENHEIGHT 720
// -----------------------------------------------------------------------------
extern SDL_Point Eng_screensize;

// Frame handling
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
bool Eng_init(void);
void Eng_input_update(SDL_Event* event);
void Eng_input_deferred();
bool Eng_update_frame(void);

// Collision system
typedef void (*OnCollision)(void* self, void* owner, uint32_t typeof_owner);
typedef struct {
	void*       owner;
	OnCollision callback;
	uint32_t    typeof_owner;
	ssize_t     x_ind;
	ssize_t     y_ind;
	uint32_t    width;
	uint32_t    height;
} ColRect;
typedef struct {
	int64_t  val;
	ColRect* par;
} ColNode;
typedef struct {
	ColNode* arr;
	ssize_t  len;
	ssize_t  cap;
} ColNodes;
typedef struct {
	ColNodes x;
	ColNodes y;
} ColSys;
// -----------------------------------------------------------------------------
extern ColSys Eng_col;
// _____________________________________________________________________________
void Eng_make_hitbox(
	ColRect* dest, void* owner, OnCollision callback, uint32_t typeof_owner,
	int64_t x, int64_t y, uint32_t width, uint32_t height
);
void Eng_free_hitbox(ColRect* target);
void Eng_set_hitbox(ColRect* target, Vector2l pos);
void Eng_draw_hitbox(ColRect* target, const bool hit);
void Eng_update_hitbox(ColRect* target);

// Debug stuff
typedef struct {
	SDL_FPoint pos;
	TTF_Text*  display;
} DebugMenu;
// -----------------------------------------------------------------------------
extern DebugMenu Eng_debug_menu;
extern bool      Eng_debug_vis;
// -----------------------------------------------------------------------------
void update_debug_menu(DebugMenu* data);

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

// Entity handling
enum GameObject_Types : uint32_t {
	ENTITY_NONE,
	ENTITY_PLAYER,
	ENTITY_ASTEROID,
	ENTITY_NUM,
};

#include "objects/asteroid.c"
#include "objects/player.c"

#define ENTITIES                                                               \
	X(player)                                                                  \
	X(asteroid)

#define X(type)                                                                \
	DynArrN(struct Entity_##type, Entity_##type##s);                           \
	extern Entity_##type##s entities_##type##s;
ENTITIES
#undef X

// Main rendering interface
extern SDL_Window*   window;
extern SDL_Renderer* renderer;

// Camera system
extern Camera Eng_camera;

#if __INCLUDE_LEVEL__ == 0 /////////////////////////////////////////////////////

#include "repl.c"
#include "res.c"

#include <stdlib.h>
#include <unistd.h>

bool die = false;

SDL_Point Eng_screensize = {DEFAULT_SCREENWIDTH, DEFAULT_SCREENHEIGHT};

static uint64_t last_frame_time = 1;
uint32_t        Eng_desired_fps = 60;
uint32_t        Eng_current_fps = 1;

TTF_Font*       Eng_font;
TTF_TextEngine* Eng_text_engine;
const char EMB_IOSEVKA_FONT[] = {
#embed "../res/Iosevka-Regular.ttf"
};

ColSys Eng_col = {0};

DebugMenu Eng_debug_menu;
#ifndef NDEBUG
bool Eng_debug_vis = true;
#else
bool Eng_debug_vis = false;
#endif

uint8_t    Eng_key_cache[KEY_NUM] = {0};
SDL_FPoint Eng_mouse_pos          = {0};

// Entity handling
#define X(type) Entity_##type##s entities_##type##s = {0};
ENTITIES
#undef X

SDL_Window*   window;
SDL_Renderer* renderer;

Camera Eng_camera;

// Frame handling
double Eng_get_deltatime_factor(void) {
	return last_frame_time / ((double) 1'000'000'000 / Eng_desired_fps);
}

// Control flow
// ================================================================

/* Initialize the engine, required for… well… everything in
 * it to work */
bool Eng_init(void) {
	SDL_Log("INFO: Initializing " APPLICATION_TITLE "…");
	bool fatal_error = false;

	// Try setup main SDL lib
	if(!SDL_Init(SDL_INIT_VIDEO)) {
		fatal_error = true;
		printf(CODE_ERROR "ERR: Failed to start SDL" CODE_END);
	}

	// Try setup window
	if(!SDL_CreateWindowAndRenderer(
		   APPLICATION_TITLE, DEFAULT_SCREENWIDTH, DEFAULT_SCREENHEIGHT, 0,
		   &window, &renderer
	   )) {
		fatal_error = true;
		printf(
			CODE_ERROR "ERR: Failed to open window/start renderer\n\t%s\n",
			SDL_GetError()
		);
	}

	// Disable AA
	if(!SDL_SetDefaultTextureScaleMode(renderer, SDL_SCALEMODE_PIXELART)) {
		printf(CODE_WARN "WARN: Failed to disable AA\n\t%s\n", SDL_GetError());
	} else {
		printf("INFO: Disabled AA\n");
	}

	// Try setup font lib
	if(!TTF_Init()) {
		fatal_error = true;
		printf(
			CODE_ERROR "ERR: Failed to start TTF" CODE_END "\n\t%s\n",
			SDL_GetError()
		);
	}

	// Try setup font file
	SDL_IOStream* to_embedded_mem =
		SDL_IOFromConstMem(EMB_IOSEVKA_FONT, sizeof(EMB_IOSEVKA_FONT));
	if(to_embedded_mem == NULL) {
		fatal_error = true;
		printf(
			CODE_ERROR
			"ERR: Failed to load iostream for font to embedded mem" CODE_END
			"\n\t%s\n",
			SDL_GetError()
		);
		goto load_font_finished;
	}
	Eng_font = TTF_OpenFontIO(to_embedded_mem, true, DEFAULT_FONTSIZE);
	if(Eng_font == NULL) {
		fatal_error = true;
		printf(
			CODE_ERROR "ERR: Failed to load font from embedded mem" CODE_END
					   "\n\t%s\n",
			SDL_GetError()
		);
	} else {
		goto load_font_finished;
	}

	if(!SDL_CloseIO(to_embedded_mem)) {
		fatal_error = true;
		printf(
			CODE_ERROR "ERR: Failed to close iostream" CODE_END "\n\t%s\n",
			SDL_GetError()
		);
	}

load_font_finished:

	// Try setup text engine
	if(!renderer) {
		fatal_error = true;
		printf(
			CODE_ERROR
			"ERR: Failed to create text engine due to missing renderer" CODE_END
		);
	} else {
		Eng_text_engine = TTF_CreateRendererTextEngine(renderer);
		if(Eng_text_engine == NULL) {
			fatal_error = true;
			printf(
				CODE_ERROR "ERR: Failed to create text engine" CODE_END
						   "\n\t%s\n",
				SDL_GetError()
			);
		}
	}

	// Try setup VSync
	if(!SDL_SetRenderVSync(renderer, SDL_RENDERER_VSYNC_ADAPTIVE)) {
		printf(CODE_WARN "WARN: Failed to enable VSync" CODE_END);
	}

	// Try setup textures
	for(uint32_t i = 0; i < TEXTURES_COUNT; i++) {
		SDL_IOStream* iostream =
			SDL_IOFromConstMem(TEXTURES[i]->tex_data, TEXTURES[i]->tex_size);
		if(iostream == NULL) {
			fatal_error = true;
			printf(
				CODE_ERROR "ERR: Failed to open iostrema to embedded mem of "
						   "texture %u\n\t%s\n",
				i, SDL_GetError()
			);
			continue;
		}
		SDL_Surface* surface = SDL_LoadPNG_IO(iostream, true);
		if(!surface) {
			fatal_error = true;
			printf(
				CODE_ERROR "ERR: Failed to create surface for texture %u"
						   "\n\t%s\n",
				i, SDL_GetError()
			);
			continue;
		}
		TEXTURES[i]->tex = SDL_CreateTextureFromSurface(renderer, surface);
		if(TEXTURES[i]->tex == NULL) {
			fatal_error = true;
			printf(
				CODE_ERROR "ERR: Failed to load texture %u into VRAM" CODE_END
						   "\n\t%s\n",
				i, SDL_GetError()
			);
		}
		SDL_DestroySurface(surface);
	}

	// Try setup DebugRepl
	/* if(!Repl_init()) { */
	/* 	fatal_error = true; */
	/* 	printf(CODE_ERROR "ERR: Failed to start asriel" CODE_END); */
	/* } */

	// Try setup DebugMenu
	Eng_debug_menu =
		(DebugMenu) {.display = NULL, .pos = (SDL_FPoint) {20.0f, 20.0f}};
	Eng_debug_menu.display = TTF_CreateText(
		Eng_text_engine, Eng_font, "[PLACEHOLDER]",
		0 /* tells TTF to get the strlen itself */
	);
	if(Eng_debug_menu.display == NULL) {
		fatal_error = true;
		printf(
			CODE_ERROR "ERR: Failed to create TTF_Text for debug menu" CODE_END
					   "\n\t%s\n",
			SDL_GetError()
		);
	}

	// Arbitrary initializations
	if(!SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND)) {
		fatal_error = true;
		printf(
			CODE_ERROR "ERR: Failed to set renderer blend mode" CODE_END
					   "\n\t%s\n",
			SDL_GetError()
		);
	}
	Eng_camera = (Camera) {(Vector2l) {0, 0}, 1.0f, 1, Eng_screensize};

	if(fatal_error) return false;

	printf("INFO: Welcome to " APPLICATION_TITLE "!\n");

	if(!Entity_player_create()) {
		printf("ERR: Failed to create player");
		return false;
	}

	return true;
}

/*
Run all input capturing events, return value MUST be
returned from SDL_AppEvent() SDL_Event* event: Pass
SDL_Event* from SDL_AppEvent()
 */
void Eng_input_update(SDL_Event* event) {
	switch(event->type) {
	case SDL_EVENT_MOUSE_MOTION:
		Eng_mouse_pos = (SDL_FPoint) {event->motion.x, event->motion.y};
		break;
	case SDL_EVENT_MOUSE_BUTTON_DOWN:
		switch(event->button.button) {
		case SDL_BUTTON_LEFT:
			Eng_key_cache[KEY_MOUSE_LEFT] |= (1 << KEY_DOWN);
			Eng_key_cache[KEY_MOUSE_LEFT] |= (1 << KEY_PRESSED);
			Eng_key_cache[KEY_MOUSE_LEFT] &= ~(1 << KEY_RELEASED);
			break;
		case SDL_BUTTON_RIGHT:
			Eng_key_cache[KEY_MOUSE_RIGHT] |= (1 << KEY_DOWN);
			Eng_key_cache[KEY_MOUSE_RIGHT] |= (1 << KEY_PRESSED);
			Eng_key_cache[KEY_MOUSE_RIGHT] &= ~(1 << KEY_RELEASED);
			break;
		}
		break;

	case SDL_EVENT_MOUSE_BUTTON_UP:
		switch(event->button.button) {
		case SDL_BUTTON_LEFT:
			Eng_key_cache[KEY_MOUSE_LEFT] &= ~(1 << KEY_DOWN);
			Eng_key_cache[KEY_MOUSE_LEFT] &= ~(1 << KEY_PRESSED);
			Eng_key_cache[KEY_MOUSE_LEFT] |= (1 << KEY_RELEASED);
			break;
		case SDL_BUTTON_RIGHT:
			Eng_key_cache[KEY_MOUSE_RIGHT] &= ~(1 << KEY_DOWN);
			Eng_key_cache[KEY_MOUSE_RIGHT] &= ~(1 << KEY_PRESSED);
			Eng_key_cache[KEY_MOUSE_RIGHT] |= (1 << KEY_RELEASED);
			break;
		}
		break;

	case SDL_EVENT_KEY_DOWN:
		switch(event->key.key) {
		case SDLK_ESCAPE:
			// Manual quit from the window
			SDL_Log("INFO: Escape pressed, exiting…");
			die = true;
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
		die = true;
	}

	if(Eng_get_key_down(KEY_F3)) Eng_debug_vis = !Eng_debug_vis;
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
bool Eng_update_frame(void) {
	// Start frame timer
	const uint64_t frametime_start = SDL_GetTicksNS();

	// Check for new input events
	SDL_Event cur_event = {0};
	while(SDL_PollEvent(&cur_event)) {
		Eng_input_update(&cur_event);
	}

	if(Eng_get_key_pressed(KEY_MOUSE_LEFT)) {
		Entity_asteroid_create(Cam_screen_to_world(Eng_mouse_pos, &Eng_camera));
	}
#define DUMP_COL_X                                                             \
	do {                                                                       \
		SDL_Log("=Dumping col x==");                                           \
		for(ssize_t i = 0; i < Eng_col.x.len; i++) {                           \
			SDL_Log(                                                           \
				"%ld - %ld", i, Eng_col.x.arr[i].val / DEFAULT_FIXED_POINT     \
			);                                                                 \
		}                                                                      \
		SDL_Log("================");                                           \
		ASSERT_COL_X;                                                          \
	} while(0)
#define DUMP_COL_Y                                                             \
	do {                                                                       \
		SDL_Log("=Dumping col y==");                                           \
		for(ssize_t i = 0; i < Eng_col.y.len; i++) {                           \
			SDL_Log(                                                           \
				"%ld - %ld", i, Eng_col.y.arr[i].val / DEFAULT_FIXED_POINT     \
			);                                                                 \
		}                                                                      \
		SDL_Log("================");                                           \
		ASSERT_COL_Y;                                                          \
	} while(0);
#define ASSERT_COL_X                                                           \
	do {                                                                       \
		for(ssize_t i = 0; i < Eng_col.x.len - 1; i++) {                       \
			if(Eng_col.x.arr[i].val > Eng_col.x.arr[i + 1].val) {              \
				SDL_Log(                                                       \
					CODE_WARN                                                  \
					"WARNING: X array violated sorting assumption" CODE_END    \
				);                                                             \
				die = true;                                                    \
			}                                                                  \
		}                                                                      \
	} while(0)
#define ASSERT_COL_Y                                                           \
	do {                                                                       \
		for(ssize_t i = 0; i < Eng_col.y.len - 1; i++) {                       \
			if(Eng_col.y.arr[i].val > Eng_col.y.arr[i + 1].val) {              \
				SDL_Log(                                                       \
					CODE_WARN                                                  \
					"WARNING: Y array violated sorting assumption" CODE_END    \
				);                                                             \
				die = true;                                                    \
			}                                                                  \
		}                                                                      \
	} while(0);

	// Print col report
	if(Eng_get_key_pressed(KEY_RETURN)) {
		DUMP_COL_X;
		DUMP_COL_Y;
	}

	// Grey background
	SDL_SetRenderDrawColor(renderer, 39, 36, 43, 255);
	SDL_RenderClear(renderer);

	if(Eng_get_key_down(KEY_MINUS)) {
		Eng_camera.zoom_factor =
			CLAMP(INT8_MIN, INT8_MAX, Eng_camera.zoom_factor - 1);
		Eng_camera.zoom = 1 * pow(1.1, Eng_camera.zoom_factor);
	}
	if(Eng_get_key_down(KEY_PLUS)) {
		Eng_camera.zoom_factor =
			CLAMP(INT8_MIN, INT8_MAX, Eng_camera.zoom_factor + 1);
		Eng_camera.zoom = 1 * pow(1.1, Eng_camera.zoom_factor);
	}

#define X(type)                                                                \
	DynArrLoop(&entities_##type##s,                                            \
	           Entity_##type##_update(&entities_##type##s.arr[i], i);          \
	           printf("Updated " #type "\n"););

	ENTITIES
#undef X

	// Draw debug menu if debug is visible
	update_debug_menu(&Eng_debug_menu);

	SDL_RenderPresent(renderer);

	// Process DebugRepl
	if(SDL_TryWaitSemaphore(Repl_repl.semaphore)) {
		switch(Repl_repl.command) {
		case COMMAND_EXIT:
			SDL_Log("INFO: Received exit command from DebugRepl, exiting…");
			die = true;
			break;
		}
	}

	// Clear bitflags on input key_cache
	Eng_input_deferred();

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

	if(die) return false;

	return true;
}

// Collision system ============================================================
static ssize_t sort_colvalue(bool is_y, ssize_t index) {
	SDL_Log(
		"=Begin sorting index %ld =======================================",
		index
	);
	if(!is_y) {
		SDL_Log("=Item is on x array");
		SDL_Log("Trying to sort up…");
		while(index < Eng_col.x.len - 1) {
			if(Eng_col.x.arr[index].val <= Eng_col.x.arr[index + 1].val) {
				SDL_Log(
					"Finished sorting up: %ld %ld", Eng_col.x.arr[index].val,
					Eng_col.x.arr[index + 1].val
				);
				break;
			}
			SDL_Log(
				"Swapping %ld - %ld and %ld - %ld", index,
				Eng_col.x.arr[index].val, index + 1,
				Eng_col.x.arr[index + 1].val
			);
			ColNode c                = Eng_col.x.arr[index];
			Eng_col.x.arr[index]     = Eng_col.x.arr[index + 1];
			Eng_col.x.arr[index + 1] = c;

			index++;
		}
		SDL_Log("Trying to sort down…");
		while(index > 0) {
			if(Eng_col.x.arr[index].val >= Eng_col.x.arr[index - 1].val) {
				SDL_Log(
					"Finished sorting down: %ld %ld", Eng_col.x.arr[index].val,
					Eng_col.x.arr[index - 1].val
				);
				break;
			}
			SDL_Log(
				"Swapping %ld - %ld and %ld - %ld", index,
				Eng_col.x.arr[index].val, index - 1,
				Eng_col.x.arr[index - 1].val
			);
			ColNode c                = Eng_col.x.arr[index];
			Eng_col.x.arr[index]     = Eng_col.x.arr[index - 1];
			Eng_col.x.arr[index - 1] = c;

			index--;
		}
		DUMP_COL_X;
	} else {
		SDL_Log("=Item is on y array");
		SDL_Log("Trying to sort up…");
		while(index < Eng_col.y.len - 1) {
			if(Eng_col.y.arr[index].val <= Eng_col.y.arr[index + 1].val) {
				SDL_Log(
					"Finished sorting up: %ld %ld", Eng_col.y.arr[index].val,
					Eng_col.y.arr[index + 1].val
				);
				break;
			}
			SDL_Log(
				"Swapping %ld - %ld and %ld - %ld", index,
				Eng_col.y.arr[index].val, index + 1,
				Eng_col.y.arr[index + 1].val
			);
			ColNode c                = Eng_col.y.arr[index];
			Eng_col.y.arr[index]     = Eng_col.y.arr[index + 1];
			Eng_col.y.arr[index + 1] = c;

			index++;
		}
		SDL_Log("Trying to sort down…");
		while(index > 0) {
			if(Eng_col.y.arr[index].val >= Eng_col.y.arr[index - 1].val) {
				SDL_Log(
					"Finished sorting down: %ld %ld", Eng_col.y.arr[index].val,
					Eng_col.y.arr[index - 1].val
				);
				break;
			}
			SDL_Log(
				"Swapping %ld - %ld and %ld - %ld", index,
				Eng_col.y.arr[index].val, index - 1,
				Eng_col.y.arr[index - 1].val
			);
			ColNode c                = Eng_col.y.arr[index];
			Eng_col.y.arr[index]     = Eng_col.y.arr[index - 1];
			Eng_col.y.arr[index - 1] = c;

			index--;
		}
		DUMP_COL_Y;
	}
	SDL_Log(
		"=Finished sorting at %ld =========================================",
		index
	);
	return index;
}

void Eng_make_hitbox(
	ColRect* dest, void* owner, OnCollision callback, uint32_t typeof_owner,
	int64_t x, int64_t y, uint32_t width, uint32_t height
) {
	*dest = (ColRect) {
		.owner        = owner,
		.callback     = callback,
		.typeof_owner = typeof_owner,
		.width        = width * DEFAULT_FIXED_POINT,
		.height       = height * DEFAULT_FIXED_POINT,
		.x_ind        = -1,
		.y_ind        = -1,
	};

	ColNode x_node = {x, dest};
	ColNode y_node = {y, dest};

	DynArrPush(&Eng_col.x, x_node);
	DynArrPush(&Eng_col.y, y_node);

	dest->x_ind = sort_colvalue(0, Eng_col.x.len - 1);
	dest->y_ind = sort_colvalue(1, Eng_col.y.len - 1);
}

void Eng_free_hitbox(ColRect* target) {

	for(ssize_t i = target->x_ind; i < Eng_col.x.len - 2; i++) {
		Eng_col.x.arr[i + 1].par->x_ind = i;
		Eng_col.x.arr[i]                = Eng_col.x.arr[i + 1];
	}
	for(ssize_t i = target->y_ind; i < Eng_col.y.len - 2; i++) {
		Eng_col.y.arr[i + 1].par->y_ind = i;
		Eng_col.y.arr[i]                = Eng_col.y.arr[i + 1];
	}

	DynArrShrink(&Eng_col.x, 1);
	DynArrShrink(&Eng_col.y, 1);
}

void Eng_set_hitbox(ColRect* target, Vector2l pos) {
	Eng_col.x.arr[target->x_ind].val = pos.x;

	target->x_ind = sort_colvalue(0, target->x_ind);

	Eng_col.y.arr[target->y_ind].val = pos.y;

	target->y_ind = sort_colvalue(1, target->y_ind);
}

void Eng_draw_hitbox(ColRect* target, const bool hit) {
	Transform tf = {
		(Vector2l) {Eng_col.x.arr[target->x_ind].val,
	                Eng_col.y.arr[target->y_ind].val},
		(SDL_FPoint) {(int64_t) (target->width / DEFAULT_FIXED_POINT),
	                  (int64_t) (target->height / DEFAULT_FIXED_POINT)},
		(SDL_FPoint) {0, 0},
		0,
	};

	const SDL_FRect rect = Cam_transform_rect(&tf, &Eng_camera, NULL);

	if(hit) {
		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
	} else {
		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	}
	SDL_RenderRect(renderer, &rect);
}

void Eng_update_hitbox(ColRect* target) {
	// Check x
	ssize_t index     = target->x_ind;
	int64_t top_right = Eng_col.x.arr[index].val + target->width;
	while(index < Eng_col.x.len - 1) {
		ColRect* other = Eng_col.x.arr[index + 1].par;
		if(Eng_col.x.arr[index + 1].val > top_right) break;
		// x hit, check y
		int64_t other_y = Eng_col.y.arr[other->y_ind].val;
		if(other_y <= Eng_col.y.arr[target->y_ind].val + target->width &&
		   other_y >= Eng_col.y.arr[target->y_ind].val) {
			// collision confirmed, run callback
			other->callback(other->owner, target->owner, target->typeof_owner);
		}
		index++;
	}
}

// Debug stuff
// =================================================================
void update_debug_menu(DebugMenu* data) {
	if(Eng_debug_vis) {
		char           fps_string[256] = {0};
		const Vector2l mouse_world_pos =
			Cam_screen_to_world(Eng_mouse_pos, &Eng_camera);
		snprintf(
			fps_string, sizeof(fps_string),
			"FPS: %d\nCam Pos: %ld %ld \nMouse Pos: %f %f"
			"\nPos at mouse: %ld %ld",
			Eng_current_fps, Eng_camera.target.x / DEFAULT_FIXED_POINT,
			Eng_camera.target.y / DEFAULT_FIXED_POINT, Eng_mouse_pos.x,
			Eng_mouse_pos.y, mouse_world_pos.x / DEFAULT_FIXED_POINT,
			mouse_world_pos.y / DEFAULT_FIXED_POINT
		);
		TTF_SetTextString(data->display, fps_string, sizeof(fps_string));

		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		TTF_DrawRendererText(data->display, data->pos.x, data->pos.y);
	}
}

// GameObject management
#endif
