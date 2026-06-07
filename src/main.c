#include "init.c"
#include "objects.c"

#define MIN_ASTEROIDS 20

static void mkObjects(void) {
	debugCreate();
	playerCreate();

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

		SDL_SetRenderDrawColor(renderer, GRAY);
		SDL_RenderClear(renderer);

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
