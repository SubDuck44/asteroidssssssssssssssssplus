#include "init.c"
#include "objects.c"

#define MIN_ASTEROIDS 20

static void parallaxDraw(void) {
	V2i64 size_w = {
		.x = WINDOW_WIDTH * FIXED_POINT,
		.y = WINDOW_HEIGHT * FIXED_POINT,
	};
	V2d size_s = {
		.x = WINDOW_WIDTH * cam.scl,
		.y = WINDOW_HEIGHT * cam.scl,
	};
	V2d   tl_s   = {.x = 0, .y = 0};
	V2i64 tl_w   = cam2wld(tl_s);
	V2d   origin = {
		  .x = -(int64_t) ((tl_w.x % size_w.x) / FIXED_POINT),
		  .y = -(int64_t) ((tl_w.y % size_w.y) / FIXED_POINT),
    };
	SDL_FRect dest_rect = {
		.w = size_s.x,
		.h = size_s.y,
	};

	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	for(double y = origin.y; y < WINDOW_HEIGHT; y += size_s.y) {
		for(double x = origin.x; x < WINDOW_WIDTH; x += size_s.x) {
			dest_rect.x = x;
			dest_rect.y = y;

			SDL_RenderRect(renderer, &dest_rect);
		}
	}
}

static void mkObjects(void) {
	debugCreate();
	playerCreate();
	statsCreate();

	for(size_t i = 0; i < MIN_ASTEROIDS; i++) {
		asteroidCreate();
	}
}

int main(void) {
	init();
	mkObjects();

	bool running = true;
	while(running) {
		uint64_t frameBegin = SDL_GetTicksNS();

		// UPDATE
		////////////////////////////////////////////////////////////////////////

		handleInput(&running);

		if(PRES(MOUSE_LEFT)) {
			Asteroid* asteroid = asteroidCreate();
			asteroid->pos      = cam2wld(mousePos);
		}

		if(PRES(F3)) debugVisible ^= 1;

#define ZOOM_FACTOR 1.05
		if(DOWN(PLUS)) zoomCamera(ZOOM_FACTOR);
		if(DOWN(MINUS)) zoomCamera(1 / ZOOM_FACTOR);

#define X(name)                                                                \
	PoolLoop(name##Pool, {                                                     \
		name##Update(it);                                                      \
		hitboxUpdate(it);                                                      \
	});

		OBJECTS
#undef X

		while(asteroidPool.used.len - asteroidPool.free.len < MIN_ASTEROIDS) {
			asteroidCreate();
		}

		// RENDER
		////////////////////////////////////////////////////////////////////////

		SDL_SetRenderDrawColor(renderer, GRAY);
		SDL_RenderClear(renderer);

		parallaxDraw();

#define X(name)                                                                \
	PoolLoop(name##Pool, {                                                     \
		name##Render(it);                                                      \
		if(debugVisible) {                                                     \
			V2d pos = wld2cam(it->pos);                                        \
			lineRGBA(renderer, pos.x - 10, pos.y, pos.x + 10, pos.y, CYA);     \
			lineRGBA(renderer, pos.x, pos.y - 10, pos.x, pos.y + 10, CYA);     \
			hitboxRender(it);                                                  \
		}                                                                      \
	});

		OBJECTS
#undef X

		SDL_RenderPresent(renderer);

		uint64_t frameTime = SDL_GetTicksNS() - frameBegin;

		if(frameTime < DFT) {
			uint64_t delta = DFT - frameTime;
			SDL_DelayPrecise(delta);
			frameTime += delta;
		}

		dtf = frameTime;
		fps = (fps + (double) BIL / frameTime) / 2;
	}

	SDL_Quit();
}
