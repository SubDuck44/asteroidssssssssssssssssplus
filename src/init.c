#pragma once

void init(void);

#if __INCLUDE_LEVEL__ == 0 /////////////////////////////////////////////////////

#include <stdio.h>

#include "res.c"
#include "utils.c"

static void initSDL(void) {
	if(!SDL_Init(SDL_INIT_VIDEO)) SDL_Die("failed to start SDL");

	if(!SDL_CreateWindowAndRenderer(
		   WINDOW_TITLE,  //
		   WINDOW_WIDTH,  //
		   WINDOW_HEIGHT, //
		   0,             //
		   &window,       //
		   &renderer      //
	   ))
		SDL_Die("failed to create window");

	if(!SDL_SetDefaultTextureScaleMode(renderer, SDL_SCALEMODE_PIXELART))
		SDL_Die("failed to disable AA");

	if(!SDL_SetRenderVSync(renderer, SDL_RENDERER_VSYNC_ADAPTIVE))
		SDL_Die("failed to enable VSync");

	if(!SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND))
		SDL_Die("failed to set blend mode");
}

static void initTTF(void) {
	static const char iosevka[] = {
#embed "../res/Iosevka-Regular.ttf"
  };

	if(!TTF_Init()) SDL_Die("failed to start TTF");

	SDL_IOStream* io = SDL_IOFromConstMem(iosevka, sizeof(iosevka));
	if(!io) SDL_Die("failed to create font iostream");

	font = TTF_OpenFontIO(io, true, FONT_SIZE);
	if(!font) SDL_Die("failed to create font");

	textEngine = TTF_CreateRendererTextEngine(renderer);
	if(!textEngine) SDL_Die("failed to create text engine");
}

static void loadTextures(void) {
	for(size_t i = 0; i < TEXTURES_COUNT; i++) {
		Texture* t = TEXTURES[i];

		SDL_IOStream* io = SDL_IOFromConstMem(t->tex_data, t->tex_size);
		if(!io) SDL_Die("failed to create texture iostream");

		SDL_Surface* srf = SDL_LoadPNG_IO(io, true);
		if(!srf) SDL_Die("failed to create texture surface");

		t->tex = SDL_CreateTextureFromSurface(renderer, srf);
		if(!t->tex) SDL_Die("failed to create texture");

		if(!SDL_GetTextureSize(t->tex, &t->w, &t->h))
			SDL_Die("failed to get texture size");

		SDL_DestroySurface(srf);
	}
}

void init(void) {
	initSDL();
	initTTF();
	loadTextures();

	puts(OKAY("Welcome to " WINDOW_TITLE "!"));
}

#endif
