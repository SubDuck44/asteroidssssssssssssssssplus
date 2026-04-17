#include <SDL3/SDL_main.h> // Dont move this or DIE

#include "engine.c"
#include "gameobjects.c"

int main(int argc, char* argv[]) {
	/* INIT */
	(void) argc;
	(void) argv;
	if(!Eng_init()) return -1;

	ASSERT_PREDICATE(
		GameObject_player_create(), return SDL_APP_FAILURE;
		, CODE_SUCCESS GAMEOBJECT_CREATE_SUCCESS CODE_END,
		CODE_ERROR GAMEOBJECT_CREATE_FAILURE CODE_END
	);
	/* LOOP */
	while(true) {
		if(!Eng_update_frame()) return -1;
	}

	// Exit is handled by engine itself; will be force-aborted
}
