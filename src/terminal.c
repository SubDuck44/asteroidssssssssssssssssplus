#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

typedef struct {
	TTF_Text frame_buf;
} Terminal;

#include "main.c"

#if __INCLUDE_LEVEL__ == 0 /////////////////////////////////////////////////////

#endif
