#pragma once

#define _DEFAULT_SOURCE

#include "res.c"

#include <endian.h>
#include <stdio.h>

#include <SDL3/SDL.h>
#include <SDL3_gfx/SDL3_gfxPrimitives.h>
#include <SDL3_ttf/SDL_ttf.h>

#define VSYNC_ON
#define DEF_SCREENWIDTH 1280
#define DEF_SCREENHEIGHT 720
#define DEF_FPS 60
#define DEF_FPS_TEXTBUFFER_SIZE 64
#define DEF_FONTSIZE 12.0f

#define RAD2DEG (180 / M_PI)
#define DEG2RAD (1 / RAD2DEG)

#define SDL_Err(fmt, ...)                                                      \
	do {                                                                       \
		SDL_LogError(                                                          \
			SDL_LOG_CATEGORY_APPLICATION, fmt ": %s",                          \
			__VA_ARGS__ __VA_OPT__(, ) SDL_GetError()                          \
		);                                                                     \
	} while(0)

#define GET_KEY_PRESSED(key) engine.key_cache[key]

#define PROPER_MOD(x, mod) ((x % mod) + mod) % mod
#define ROTATE(x) ((x % 360) + 360) % 360

#define KEYS X(W) X(A) X(S) X(D) X(LALT) X(I) X(J) X(K) X(L)

enum Keys {
#define X(x) KEY_##x,
	KEYS
#undef X
		KEY_MOUSE_LEFT,
	KEY_MOUSE_RIGHT,
	KEY_NUM,
};

typedef struct {
	bool         alive;
	SDL_FPoint   pos;
	SDL_FPoint   vel;
	double       rot;
	float        rot_vel;
	float        move_speed;
	float        rot_speed;
	SDL_Texture* tex;
} Player;

typedef struct {
	uint64_t        last_frame;
	uint64_t        last_average;
	uint32_t        desired_fps;
	uint16_t        current_fps;
	char            fps_textbuffer[DEF_FPS_TEXTBUFFER_SIZE];
	TTF_Font*       font;
	uint32_t        fontsize;
	TTF_TextEngine* text_engine;
	TTF_Text*       hewo;
	bool            key_cache[KEY_NUM];
	SDL_FPoint      mouse_pos;
} Engine;

extern SDL_Point screensize;
extern Player    player;
extern Engine    engine;

extern SDL_Window*   window;
extern SDL_Renderer* renderer;

extern const char EMB_IOSEVKA_FONT[];

// Vector magic
SDL_FPoint pointf_add(SDL_FPoint a, SDL_FPoint b);
SDL_FPoint pointf_rotate(SDL_FPoint a, float deg);
SDL_FPoint pointf_normalize(SDL_FPoint a);
SDL_FPoint pointf_scale(SDL_FPoint a, double scale);
SDL_FPoint pointf_angle_to(SDL_FPoint a, SDL_FPoint b);
float      pointf_length(SDL_FPoint a);
float      pointf_bearing(SDL_FPoint zero, SDL_FPoint tgt);

// Misc engine
double get_deltatime_factor(void);

#if __INCLUDE_LEVEL__ == 0 /////////////////////////////////////////////////////

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h> // Dont move this or DIE

SDL_Point     screensize = {DEF_SCREENWIDTH, DEF_SCREENHEIGHT};
Player        player     = {0};
Engine        engine     = {0};
SDL_Window*   window;
SDL_Renderer* renderer;

const char EMB_IOSEVKA_FONT[] = {
#embed "../res/Iosevka-Regular.ttf"
};

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

double get_deltatime_factor(void) {
	return ((double) (1'000'000'000) / engine.desired_fps) / engine.last_frame;
}

void engine_update_frame(void) {
	// Set FPS display
	{
		char fps_string[256] = {0};
		snprintf(fps_string, sizeof(fps_string), "FPS: %d", engine.current_fps);
		TTF_SetTextString(engine.hewo, fps_string, sizeof(fps_string));
	}
	// Player movement
	{
		if(GET_KEY_PRESSED(KEY_A)) player.rot_vel -= player.rot_speed;
		if(GET_KEY_PRESSED(KEY_D)) player.rot_vel += player.rot_speed;
		if(GET_KEY_PRESSED(KEY_W)) {
			player.vel = pointf_add(
				pointf_force(
					player.move_speed * get_deltatime_factor(), player.rot
				),
				player.vel
			);
		}
		if(GET_KEY_PRESSED(KEY_I))
			player.vel = pointf_add(
				pointf_force(player.move_speed, player.rot), player.vel
			);
		if(GET_KEY_PRESSED(KEY_J))
			player.vel = pointf_add(
				pointf_force(player.move_speed, ROTATE((int) player.rot - 90)),
				player.vel
			);

		if(GET_KEY_PRESSED(KEY_K))
			player.vel = pointf_add(
				pointf_force(player.move_speed, ROTATE((int) player.rot + 180)),
				player.vel
			);
		if(GET_KEY_PRESSED(KEY_L))
			player.vel = pointf_add(
				pointf_force(player.move_speed, ROTATE((int) player.rot + 90)),
				player.vel
			);
		if(GET_KEY_PRESSED(KEY_MOUSE_LEFT)) {
			player.rot = pointf_bearing(player.pos, engine.mouse_pos);
		}

		player.pos = pointf_add(player.pos, player.vel);
		player.rot = ROTATE((int) (player.rot + player.rot_vel));

		// TODO Switch these to modfs (or just if it)
		player.pos.x = PROPER_MOD((int) player.pos.x, screensize.x);
		player.pos.y = PROPER_MOD((int) player.pos.y, screensize.y);
	}
}

void engine_draw_frame(void) {
	SDL_FRect  player_rect = {player.pos.x, player.pos.y, 50, 50};
	SDL_FPoint player_off  = {25, 25};
	SDL_FPoint player_ctr  = pointf_add(player.pos, player_off);
	// Grey background
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	// Draw player
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	SDL_RenderTextureRotated(
		renderer, TEX_PLAYER.tex, NULL, &player_rect, player.rot, &player_off,
		SDL_FLIP_NONE
	);

	// Draw lateral movement guides
	if(GET_KEY_PRESSED(KEY_LALT)) {
		const SDL_FPoint bow =
			pointf_add(pointf_force(75, player.rot), player_ctr);
		const SDL_FPoint stern = pointf_add(
			pointf_force(75, PROPER_MOD((int) player.rot + 180, 360)),
			player_ctr
		);
		const SDL_FPoint port = pointf_add(
			pointf_force(75, PROPER_MOD((int) player.rot - 90, 360)), player_ctr
		);
		const SDL_FPoint starboard = pointf_add(
			pointf_force(75, PROPER_MOD((int) player.rot + 90, 360)), player_ctr
		);
		thickLineRGBA(
			renderer, bow.x, bow.y, stern.x, stern.y, 3, 25, 60, 165, 255
		);
		thickLineRGBA(
			renderer, port.x, port.y, starboard.x, starboard.y, 3, 25, 60, 165,
			255
		);
	}

	// Draw velocity vector
	SDL_FPoint dest  = pointf_add(player_ctr, pointf_scale(player.vel, 10));
	SDL_FPoint dest2 = pointf_add(player_ctr, pointf_scale(player.vel, -10));
	SDL_FRect  dest_icantbefucked = {dest.x - 12.5f, dest.y - 12.5f, 25, 25};
	SDL_FRect  dest_icantbefucked2eletricboogaloo = {
        dest2.x - 12.5f, dest2.y - 12.5f, 25, 25
    };
	thickLineColor(
		renderer, player_ctr.x, player_ctr.y, dest.x, dest.y, 3,
		htobe32(0x94DE0AFF)
	);
	SDL_RenderTexture(renderer, TEX_PROGRADE.tex, NULL, &dest_icantbefucked);
	thickLineColor(
		renderer, player_ctr.x, player_ctr.y, dest2.x, dest2.y, 3,
		htobe32(0xD2DB27FF)
	);
	SDL_RenderTexture(
		renderer, TEX_RETROGRADE.tex, NULL, &dest_icantbefucked2eletricboogaloo
	);

	// Draw FPS
	SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
	TTF_DrawRendererText(engine.hewo, 20.0f, 20.0f);

	SDL_RenderPresent(renderer);
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
	(void) appstate;
	(void) argc;
	(void) argv;
	// Try setup main SDL lib
	if(!SDL_Init(SDL_INIT_VIDEO)) {
		SDL_Err("Failed to initialize SDL");
		return SDL_APP_FAILURE;
	}

	// Try setup window
	if(!SDL_CreateWindowAndRenderer(
		   "Asteroidssssssssssssssss+", 1280, 720, 0, &window, &renderer
	   )) {
		SDL_Err("Failed to initizalized window/renderer");
		return SDL_APP_FAILURE;
	}

	// Disable AA
	SDL_SetDefaultTextureScaleMode(renderer, SDL_SCALEMODE_PIXELART);

	// Try setup font lib
	if(!TTF_Init()) {
		SDL_Err("Failed to load font engine");
		return SDL_APP_FAILURE;
	}

	// Try setup font file
	engine.font = TTF_OpenFontIO(
		SDL_IOFromConstMem(EMB_IOSEVKA_FONT, sizeof(EMB_IOSEVKA_FONT)), true,
		DEF_FONTSIZE
	);
	engine.fontsize = DEF_FONTSIZE;
	if(!engine.font) {
		SDL_Err("Failed to load font");
		return SDL_APP_FAILURE;
	}

	// Try setup text engine
	engine.text_engine = TTF_CreateRendererTextEngine(renderer);
	if(!engine.text_engine) {
		SDL_Err("Failed to initialize text engine");
		return SDL_APP_FAILURE;
	}

#ifdef VSYNC_ON
	// Try setup VSync
	if(!SDL_SetRenderVSync(renderer, SDL_RENDERER_VSYNC_ADAPTIVE)) {
		SDL_Err("Failed to activate VSync");
		return SDL_APP_FAILURE;
	}
#endif

	// Try setup textures
	for(uint32_t i = 0; i < TEXTURES_COUNT; i++) {
		SDL_Surface* surface = SDL_LoadPNG_IO(
			SDL_IOFromConstMem(TEXTURES[i]->tex_data, TEXTURES[i]->tex_size),
			true
		);
		if(!surface) {
			SDL_Err("Failed to load texture %d into RAM", i);
			return SDL_APP_FAILURE;
		}
		TEXTURES[i]->tex = SDL_CreateTextureFromSurface(renderer, surface);
		if(!TEXTURES[i]->tex) {
			SDL_Err("Failed to load texture %d into VRAM", i);
			SDL_DestroySurface(surface);
			return SDL_APP_FAILURE;
		}
		SDL_DestroySurface(surface);
	}

	// Arbitrary initializations
	player.alive      = true;
	player.pos        = (SDL_FPoint) {screensize.x >> 1, screensize.y >> 1};
	player.vel        = (SDL_FPoint) {0.0, 0.0};
	player.rot_vel    = 0.0;
	player.rot        = 0.0;
	player.rot_speed  = 0.2;
	player.move_speed = 0.25;

	engine.last_frame =
		1; /* DO NOT set this to 0 UNDER ANY CIRCUMSTANCE
	          I couldve invested in doing proper error checking, but that
	          was too much effort. Therefore, just leave this at 1. Don't
	          worry about it. (This avoids NaN contamination) */
	engine.desired_fps = DEF_FPS;
	engine.hewo = TTF_CreateText(engine.text_engine, engine.font, "hewo :3", 0);

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
	(void) appstate;
	switch(event->type) {
	case SDL_EVENT_MOUSE_MOTION:
		engine.mouse_pos = (SDL_FPoint) {event->motion.x, event->motion.y};
		break;
	case SDL_EVENT_MOUSE_BUTTON_DOWN:
		switch(event->button.button) {
		case SDL_BUTTON_LEFT:
			engine.key_cache[KEY_MOUSE_LEFT] = true;
			break;
		case SDL_BUTTON_RIGHT:
			engine.key_cache[KEY_MOUSE_RIGHT] = true;
			break;
		}
		break;

	case SDL_EVENT_MOUSE_BUTTON_UP:
		switch(event->button.button) {
		case SDL_BUTTON_LEFT:
			engine.key_cache[KEY_MOUSE_LEFT] = false;
			break;
		case SDL_BUTTON_RIGHT:
			engine.key_cache[KEY_MOUSE_RIGHT] = false;
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
		engine.key_cache[KEY_##x] = true;                                      \
		break;
			KEYS
#undef X
		}
		break;

	case SDL_EVENT_KEY_UP:
		switch(event->key.key) {
#define X(x)                                                                   \
	case SDLK_##x:                                                             \
		engine.key_cache[KEY_##x] = false;                                     \
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

SDL_AppResult SDL_AppIterate(void* appstate) {
	(void) appstate;

	// Start frame timer
	const uint64_t frametime_start = SDL_GetTicksNS();

	engine_update_frame();
	engine_draw_frame();

	// Stop frame timer
	engine.last_frame = SDL_GetTicksNS() - frametime_start;

	const uint64_t desired_frametime = 1'000'000'000 / engine.desired_fps;
	if(engine.last_frame < desired_frametime) {
		const uint64_t frame_diff = desired_frametime - engine.last_frame;
		SDL_DelayPrecise(frame_diff);
		engine.last_frame += frame_diff;
	}

	engine.current_fps =
		(((double) (1'000'000'000) / engine.last_frame) + engine.current_fps) /
		2;

	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
	(void) appstate;
	(void) result;
	// Window/renderer cleanup done by SDL internally
}
#endif
