#pragma once

#define _DEFAULT_SOURCE

#include "engine.c"

#if __INCLUDE_LEVEL__ == 0 /////////////////////////////////////////////////////

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h> // Dont move this or DIE

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
	(void) appstate;
	(void) argc;
	(void) argv;

	if(Eng_init() == SDL_APP_FAILURE) return SDL_APP_FAILURE;

	ASSERT_PREDICATE(
		GameObject_player_create(), return SDL_APP_FAILURE;
		, CODE_SUCCESS GAMEOBJECT_CREATE_SUCCESS CODE_END,
		CODE_ERROR GAMEOBJECT_CREATE_FAILURE CODE_END
	);

	ASSERT_PREDICATE(GameObject_fps_display_create((SDL_FPoint) {20.0f, 20.0f}),
	                 return SDL_APP_FAILURE;
	                 , CODE_SUCCESS GAMEOBJECT_CREATE_SUCCESS CODE_END,
	                 CODE_ERROR GAMEOBJECT_CREATE_FAILURE CODE_END);

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
	(void) appstate;
	return Eng_tick_input(event);
}

SDL_AppResult SDL_AppIterate(void* appstate) {
	(void) appstate;

	if(Eng_tick_once() == ERR_FATAL) return SDL_APP_FAILURE;

	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
	(void) appstate;
	(void) result;
	// Window/renderer cleanup done by SDL
}
#endif
