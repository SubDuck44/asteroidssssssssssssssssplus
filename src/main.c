#pragma once

#define _DEFAULT_SOURCE

#include "engine.c"
#include "gameobjects.c"

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

extern struct GameObject_Player player;
extern SDL_Window*              window;
extern SDL_Renderer*            renderer;

#if __INCLUDE_LEVEL__ == 0 /////////////////////////////////////////////////////

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h> // Dont move this or DIE

struct GameObject_Player player = {0};
SDL_Window*              window;
SDL_Renderer*            renderer;

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
	(void) appstate;
	(void) argc;
	(void) argv;

	if(Eng_Init() == SDL_APP_FAILURE) return SDL_APP_FAILURE;

	GameObject_player_create();
	GameObject_fps_display_create((SDL_FPoint) {20.0f, 20.0f});

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
	(void) appstate;
	return Eng_TickInput(event);
}

SDL_AppResult SDL_AppIterate(void* appstate) {
	(void) appstate;

	if(Eng_TickOnce() == ERR_FATAL) return SDL_APP_FAILURE;

	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
	(void) appstate;
	(void) result;
	// Window/renderer cleanup done by SDL
}
#endif
