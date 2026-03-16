#define SDL_MAIN_USE_CALLBACKS

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <math.h>
#include <stdio.h>

#define DEF_SCREENWIDTH 1280
#define DEF_SCREENHEIGHT 720
#define DEF_FPS 60
#define DEF_FPS_TEXTBUFFER_SIZE 64

#define DEG2RAD 0.01745329252
#define RAD2DEG 57.29577951

#define SDL_Err(fmt, ...)                                                      \
	do {                                                                       \
		SDL_LogError(                                                          \
			SDL_LOG_CATEGORY_APPLICATION, fmt ": %s",                          \
			__VA_ARGS__ __VA_OPT__(, ) SDL_GetError()                          \
		);                                                                     \
	} while(0)

#define REG_TEXTURE(tex_embed_var)                                             \
	{NULL, tex_embed_var, sizeof(tex_embed_var), {0.0f, 0.0f, 0.0f, 0.0f}}

#define GET_KEY_PRESSED(key) engine.key_cache[key]

#define PROPER_MOD(x, mod) ((x % mod) + mod) % mod

enum Texture_Cache_Textures { TEXTURE_PLAYER, TEXTURES_NUM };
enum Keys { KEY_W, KEY_A, KEY_S, KEY_D, KEY_NUM };

typedef struct {
	SDL_Texture*   tex;
	const uint8_t* tex_data;
	size_t         tex_size;
	SDL_FRect      src;
} Texture;

const uint8_t EMBED_TEXTURE_PLAYER[] = {
#embed "../res/player.png"
};

Texture textures[TEXTURES_NUM] = {
	REG_TEXTURE(EMBED_TEXTURE_PLAYER),
};

typedef struct {
	bool         alive;
	SDL_FPoint   pos;
	SDL_FPoint   vel;
	double       rot;
	float        move_speed;
	SDL_Texture* tex;
} Player;

typedef struct {
	uint64_t        last_frame;
	uint64_t        last_average;
	uint32_t        desired_fps;
	uint16_t        current_fps;
	char            fps_textbuffer[DEF_FPS_TEXTBUFFER_SIZE];
	TTF_Font*       font;
	TTF_TextEngine* text_engine;
	TTF_Text*       hewo;
	bool            key_cache[KEY_NUM];
} Engine;

SDL_Point screensize = {DEF_SCREENWIDTH, DEF_SCREENHEIGHT};
Player    player     = {0};
Engine    engine     = {0};

const char iosevka_font[] = {
#embed "../res/Iosevka-Regular.ttf"
};

static SDL_Window*   window   = NULL;
static SDL_Renderer* renderer = NULL;

SDL_FPoint pointf_add(const SDL_FPoint a, const SDL_FPoint b) {
	return (SDL_FPoint) {a.x + b.x, a.y + b.y};
}

SDL_FPoint pointf_rotate(const SDL_FPoint a, const float deg) {
	const float x = (a.x * cos(deg * DEG2RAD)) - (a.y * sin(deg * DEG2RAD));
	const float y = (a.x * sin(deg * DEG2RAD)) + (a.y * cos(deg * DEG2RAD));
	return (SDL_FPoint) {x, y};
}

float pointf_length(const SDL_FPoint a) {
	const float length = sqrt((a.x * a.x) + (a.y * a.y));
	if(length <= SDL_FLT_EPSILON) return 0.0f;
	return length;
}

SDL_FPoint pointf_normalize(const SDL_FPoint a) {
	const float length = sqrt((a.x * a.x) + (a.y * a.y));
	if(length <= SDL_FLT_EPSILON) return a;
	const float x = a.x / length;
	const float y = a.y / length;
	return (SDL_FPoint) {x, y};
}

SDL_FPoint pointf_scale(const SDL_FPoint a, const double scale) {
	if(scale <= SDL_FLT_EPSILON || pointf_length(a) <= SDL_FLT_EPSILON)
		return a;
	float x = a.x * scale;
	float y = a.y * scale;
	return (SDL_FPoint) {x, y};
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
	// Move player
	{
		SDL_FPoint new_pos = {0};
		if(GET_KEY_PRESSED(KEY_W)) new_pos.y -= 1;
		if(GET_KEY_PRESSED(KEY_A)) new_pos.x -= 1;
		if(GET_KEY_PRESSED(KEY_S)) new_pos.y += 1;
		if(GET_KEY_PRESSED(KEY_D)) new_pos.x += 1;
		new_pos = pointf_normalize(new_pos);
		new_pos =
			pointf_scale(new_pos, player.move_speed * get_deltatime_factor());
		player.pos = pointf_add(new_pos, player.pos);
	}
}

void engine_draw_frame(void) {
	SDL_FRect  player_rect = {player.pos.x, player.pos.y, 50, 50};
	SDL_FPoint player_ctr  = {player.pos.x + 25, player.pos.y + 25};

	// Grey background
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	// Draw player
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	SDL_RenderTextureRotated(
		renderer, textures[TEXTURE_PLAYER].tex, &textures[TEXTURE_PLAYER].src,
		&player_rect, player.rot, &player_ctr, SDL_FLIP_NONE
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

	// Try setup textures
	for(uint32_t i = 0; i < TEXTURES_NUM; i++) {
		SDL_Surface* surface = SDL_LoadPNG_IO(
			SDL_IOFromConstMem(textures[i].tex_data, textures[i].tex_size), true
		);
		if(!surface) {
			SDL_Err("Failed to load texture %d into RAM", i);
			return SDL_APP_FAILURE;
		}
		textures[i].tex = SDL_CreateTextureFromSurface(renderer, surface);
		if(!textures[i].tex) {
			SDL_Err("Failed to load texture %d into VRAM", i);
			SDL_DestroySurface(surface);
			return SDL_APP_FAILURE;
		}
		textures[i].src = (SDL_FRect) {0.0f, 0.0f, surface->w, surface->h};
		SDL_DestroySurface(surface);
	}

	// Arbitrary initializations
	player.alive      = true;
	player.pos        = (SDL_FPoint) {screensize.x >> 1, screensize.y >> 1};
	player.vel        = (SDL_FPoint) {0.0, 0.0};
	player.rot        = 0.0;
	player.move_speed = 50;

	engine.last_frame =
		1; /* DO NOT set this to 0 UNDER ANY CIRCUMSTANCE
	          I couldve invested in doing proper error checking, but that was
	          too much effort. Therefore, just leave this at 1. Don't worry
	          about it. (This avoids NaN contamination) */
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
			engine.key_cache[KEY_W] = true;
			break;
		case SDLK_A:
			engine.key_cache[KEY_A] = true;
			break;
		case SDLK_S:
			engine.key_cache[KEY_S] = true;
			break;
		case SDLK_D:
			engine.key_cache[KEY_D] = true;
			break;
		}
		break;
	case SDL_EVENT_KEY_UP:
		switch(event->key.key) {
		case SDLK_W:
			engine.key_cache[KEY_W] = false;
			break;
		case SDLK_A:
			engine.key_cache[KEY_A] = false;
			break;
		case SDLK_S:
			engine.key_cache[KEY_S] = false;
			break;
		case SDLK_D:
			engine.key_cache[KEY_D] = false;
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

	// Start frame timer
	const uint64_t frametime_start = SDL_GetTicksNS();

	engine_update_frame();
	engine_draw_frame();

	// Stop frame timer
	engine.last_frame = SDL_GetTicksNS() - frametime_start;
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
