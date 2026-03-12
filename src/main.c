#define SDL_MAIN_USE_CALLBACKS

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#define DEF_SCREENWIDTH 1280
#define DEF_SCREENHEIGHT 720
#define DEF_FPS 60

typedef struct {
  float x;
  float y;
} PointF;

typedef struct {
  bool alive;
  PointF pos;
  float move_speed;
} Player;

typedef struct {
  double last_frame;
  uint32_t desired_fps;
} Engine;

SDL_Point screensize = {DEF_SCREENWIDTH, DEF_SCREENHEIGHT};
Player player = {0};
Engine engine = {0};

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

void engine_update_frame(void) {}

void engine_draw_frame(void) {
  SDL_FRect player_rect = {player.pos.x, player.pos.y, 50, 50};
  // Grey background
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  // Draw player
  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
  SDL_RenderFillRect(renderer, &player_rect);

  SDL_RenderPresent(renderer);
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
  // Can we get SDL running?
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to initialize SDL: %s",
                 SDL_GetError());
    return SDL_APP_FAILURE;
  }
  // SDL is up!

  // Can we get a window up?
  if (!SDL_CreateWindowAndRenderer("Asteroidssssssssssssssss+", 1280, 720, 0,
                                   &window, &renderer)) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                 "Failed to initizalized window/renderer: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  // Window is up!

  // Can we get VSync up?
  if (!SDL_SetRenderVSync(renderer, SDL_RENDERER_VSYNC_ADAPTIVE)) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to activate VSync: %s",
                 SDL_GetError());
    return SDL_APP_FAILURE;
  }
  // VSync is ready!

  // Arbitrary initializations
  player.alive = true;
  player.pos = (PointF){screensize.x >> 1, screensize.y >> 1};
  player.move_speed = 50;

  engine.last_frame = 0;
  engine.desired_fps = DEF_FPS;

  // Success! Move on to program
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  switch (event->type) {
  case SDL_EVENT_KEY_DOWN:
    switch (event->key.key) {
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

SDL_AppResult SDL_AppIterate(void *appstate) {

  engine_update_frame();
  engine_draw_frame();

  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  // Window/renderer cleanup done by SDL internally
}
