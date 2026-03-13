#define SDL_MAIN_USE_CALLBACKS

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>

#define DEF_SCREENWIDTH 1280
#define DEF_SCREENHEIGHT 720
#define DEF_FPS 60
#define DEF_FPS_TEXTBUFFER_SIZE 64

#define SDL_Err(fmt, ...)                                                      \
	do {                                                                       \
		SDL_LogError(                                                          \
			SDL_LOG_CATEGORY_APPLICATION, fmt ": %s",                          \
			__VA_ARGS__ __VA_OPT__(, ) SDL_GetError()                          \
		);                                                                     \
	} while(0)

typedef struct {
	float x;
	float y;
} PointF;

typedef struct {
	bool   alive;
	PointF pos;
	float  move_speed;
} Player;

typedef struct {
	double          last_frame;
	uint32_t        desired_fps;
	uint16_t        frames_last_second;
	uint16_t        last_second_measurement;
	uint16_t        current_fps;
	char            fps_textbuffer[DEF_FPS_TEXTBUFFER_SIZE];
	TTF_Font*       font;
	TTF_TextEngine* text_engine;
	TTF_Text*       hewo;
} Engine;

SDL_Point screensize = {DEF_SCREENWIDTH, DEF_SCREENHEIGHT};
Player    player     = {0};
Engine    engine     = {0};

const char iosevka_font[] = {
#embed "../res/Iosevka-Regular.ttf"
};

static SDL_Window*   window   = NULL;
static SDL_Renderer* renderer = NULL;

void engine_update_frame(void) {
	// Get current fps
	uint16_t measurement = (uint16_t) (SDL_GetTicks() % 1000);
	if(measurement < engine.last_second_measurement) {
		engine.last_second_measurement = measurement;
		engine.current_fps             = ++engine.frames_last_second;
		engine.frames_last_second ^= engine.frames_last_second;
		snprintf(
			engine.fps_textbuffer, sizeof(engine.fps_textbuffer), "FPS: %d",
			engine.current_fps
		);
		TTF_SetTextString(engine.hewo, engine.fps_textbuffer, 0);
	} else {
		engine.frames_last_second++;
		engine.last_second_measurement = measurement;
	}
}

void engine_draw_frame(void) {
	SDL_FRect player_rect = {player.pos.x, player.pos.y, 50, 50};

	// Grey background
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	// Draw player
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	SDL_RenderFillRect(renderer, &player_rect);

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

	// Try setup font lib
	if(!TTF_Init()) {
		SDL_Err("Failed to load font engine");
		return SDL_APP_FAILURE;
	}

	// Try setup font file
	engine.font = TTF_OpenFontIO(
		SDL_IOFromConstMem(iosevka_font, sizeof(iosevka_font)), true, 12.0f
	);
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

	// Try setup VSync
	if(!SDL_SetRenderVSync(renderer, SDL_RENDERER_VSYNC_ADAPTIVE)) {
		SDL_Err("Failed to activate VSync");
		return SDL_APP_FAILURE;
	}

	// Arbitrary initializations
	player.alive      = true;
	player.pos        = (PointF) {screensize.x >> 1, screensize.y >> 1};
	player.move_speed = 50;

	engine.last_frame  = 0;
	engine.desired_fps = DEF_FPS;

	engine.hewo = TTF_CreateText(engine.text_engine, engine.font, "hewo :3", 0);

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
	(void) appstate;
	switch(event->type) {
	case SDL_EVENT_KEY_DOWN:
		switch(event->key.key) {
		case SDLK_ESCAPE:
			// Manual quit from the window
			return SDL_APP_SUCCESS;
		case SDLK_W:
			player.pos.y -= player.move_speed;
			break;
		case SDLK_A:
			player.pos.x -= player.move_speed;
			break;
		case SDLK_S:
			player.pos.y += player.move_speed;
			break;
		case SDLK_D:
			player.pos.x += player.move_speed;
			break;
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

	engine_update_frame();
	engine_draw_frame();

	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
	(void) appstate;
	(void) result;
	// Window/renderer cleanup done by SDL internally
}
