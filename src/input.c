#pragma once

#include <SDL3/SDL.h>
#include <la.h>

////////////////////////////////////////////////////////////////////////////////

#define KEY_STATES                                                             \
	X(NONE) /* key is released (level) */                                      \
	X(PRES) /* key has just been pressed (edge) */                             \
	X(DOWN) /* key is down, but not yet long enough to trigger HELD (level) */ \
	X(TAPP) /* key has JUST been released within tap interval (edge) */        \
	X(HELD) /* key has  NOT been released within tap interval (level) */

typedef enum {
#define X(x) KS_##x,
	KEY_STATES
#undef X
} KeyState;

const char* showKeyState(KeyState ks);

////////////////////////////////////////////////////////////////////////////////

#define BUTTONS X(LEFT) X(RIGHT)

#define KEYS                                                                   \
	X(W)                                                                       \
	X(A)                                                                       \
	X(S)                                                                       \
	X(D)                                                                       \
	X(I)                                                                       \
	X(J)                                                                       \
	X(K)                                                                       \
	X(L)                                                                       \
	X(LALT)                                                                    \
	X(PLUS)                                                                    \
	X(MINUS)                                                                   \
	X(F3)                                                                      \
	X(0)                                                                       \
	X(1)                                                                       \
	X(2)                                                                       \
	X(3)

typedef enum {
#define X(x) KEY_MOUSE_##x,
	BUTTONS
#undef X

#define X(x) KEY_##x,
		KEYS
#undef X

			KEY_NUM,
} Key;

const char* showKey(Key k);

////////////////////////////////////////////////////////////////////////////////

extern V2d      mousePos;
extern KeyState keyState[KEY_NUM];

#define DOWN(k) (keyState[KEY_##k] != KS_NONE)
#define PRES(k) (keyState[KEY_##k] == KS_PRES)

void handleInput(bool* running);

#if __INCLUDE_LEVEL__ == 0 /////////////////////////////////////////////////////

#include "utils.c"

const char* showKeyState(KeyState ks) {
	switch(ks) {
#define X(x)                                                                   \
	case KS_##x:                                                               \
		return "KS_" #x;

		KEY_STATES
#undef X

	default:
		return NULL;
	}
}

const char* showKey(Key k) {
	switch(k) {
#define X(x)                                                                   \
	case KEY_MOUSE_##x:                                                        \
		return "KEY_MOUSE_" #x;

		BUTTONS
#undef X

#define X(x)                                                                   \
	case KEY_##x:                                                              \
		return "KEY_" #x;

		KEYS
#undef X

			default : return NULL;
	}
}

V2d      mousePos;
KeyState keyState[KEY_NUM];

static void handleKeyDown(Key k) {
	// if the key was up, it has just now been pressed
	if(keyState[k] == KS_NONE) keyState[k] = KS_PRES;
	// if the key was already down, it's being held longer
	else if(keyState[k] == KS_DOWN)
		keyState[k] = KS_HELD;
}

static void handleKeyUp(Key k) {
	// if the key was down a short while, it was tapped
	if(keyState[k] == KS_DOWN) keyState[k] = KS_TAPP;
	// if the key was down a long while, it's now up
	else if(keyState[k] == KS_HELD)
		keyState[k] = KS_NONE;
}

void handleInput(bool* running) {
	for(Key k = 0; k < KEY_NUM; k++) {
		// if the key was just pressed, it's now down
		if(keyState[k] == KS_PRES) keyState[k] = KS_DOWN;
		// if the key was just tapped, it's now up
		else if(keyState[k] == KS_TAPP)
			keyState[k] = KS_NONE;
	}

	SDL_Event event = {0};
	while(SDL_PollEvent(&event)) {
		switch(event.type) {
		case SDL_EVENT_QUIT:
			*running = false;
			break;

		case SDL_EVENT_MOUSE_MOTION:
			SDL_ConvertEventToRenderCoordinates(renderer, &event);
			mousePos = v2d(event.motion.x, event.motion.y);
			break;

		case SDL_EVENT_MOUSE_BUTTON_DOWN:
			switch(event.button.button) {
#define X(x)                                                                   \
	case SDL_BUTTON_##x:                                                       \
		handleKeyDown(KEY_MOUSE_##x);                                          \
		break;

				BUTTONS
#undef X
			}
			break;

		case SDL_EVENT_MOUSE_BUTTON_UP:
			switch(event.button.button) {
#define X(x)                                                                   \
	case SDL_BUTTON_##x:                                                       \
		handleKeyUp(KEY_MOUSE_##x);                                            \
		break;

				BUTTONS
#undef X
			}
			break;

		case SDL_EVENT_KEY_DOWN:
			switch(event.key.key) {
#define X(x)                                                                   \
	case SDLK_##x:                                                             \
		handleKeyDown(KEY_##x);                                                \
		break;

				KEYS
#undef X
			}
			break;

		case SDL_EVENT_KEY_UP:
			switch(event.key.key) {
#define X(x)                                                                   \
	case SDLK_##x:                                                             \
		handleKeyUp(KEY_##x);                                                  \
		break;

				KEYS
#undef X
			}
			break;
		}
	}
}

#endif
