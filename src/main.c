#pragma once

#define _DEFAULT_SOURCE

#include "engine.c"
#include "gameobjects.c"
#include "res.c"

#define VSYNC_ON
#define DEF_FPS 60
#define DEF_FPS_TEXTBUFFER_SIZE 64
#define DEF_FONTSIZE 12.0f

#define SDL_Err(fmt, ...)                                                      \
	do {                                                                       \
		SDL_LogError(                                                          \
			SDL_LOG_CATEGORY_APPLICATION, fmt ": %s",                          \
			__VA_ARGS__ __VA_OPT__(, ) SDL_GetError()                          \
		);                                                                     \
	} while(0)

#define GET_KEY_PRESSED(key) engine.key_cache[key]

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

extern struct GameObject_Player player;
extern Engine                   engine;
extern SDL_Window*              window;
extern SDL_Renderer*            renderer;

extern const char EMB_IOSEVKA_FONT[];

// Misc engine
double get_deltatime_factor(void);

#if __INCLUDE_LEVEL__ == 0 /////////////////////////////////////////////////////

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h> // Dont move this or DIE

struct GameObject_Player player = {0};
Engine                   engine = {0};
SDL_Window*              window;
SDL_Renderer*            renderer;

const char EMB_IOSEVKA_FONT[] = {
#embed "../res/Iosevka-Regular.ttf"
};

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
}

void engine_draw_frame(void) {
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
	if(Eng_Init() == SDL_APP_FAILURE) return SDL_APP_FAILURE;

	engine.last_frame =
		1; /* DO NOT set this to 0 UNDER ANY CIRCUMSTANCE
	          I couldve invested in doing proper error checking, but that
	          was too much effort. Therefore, just leave this at 1. Don't
	          worry about it. (This avoids NaN contamination) */
	engine.desired_fps = DEF_FPS;
	engine.hewo = TTF_CreateText(engine.text_engine, engine.font, "hewo :3", 0);

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	GameObject_fps_display_create((SDL_FPoint) {20.0f, 20.0f});
	GameObject_player_create();

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
	(void) appstate;
	return Eng_TickInput(event);
}

SDL_AppResult SDL_AppIterate(void* appstate) {
	(void) appstate;

	// Start frame timer
	const uint64_t frametime_start = SDL_GetTicksNS();

	if(Eng_TickOnce() == ERR_FATAL) return SDL_APP_FAILURE;
	SDL_RenderPresent(renderer);

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
