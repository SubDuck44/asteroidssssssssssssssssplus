#include <SDL3/SDL_main.h> // Dont move this or DIE

#include "engine.c"

int main(int argc, char* argv[]) {
	/* INIT */
	(void) argc;
	(void) argv;
	if(Eng_init() == FAILURE) return -1;

	/* LOOP */
	for(;;) {
		if(Eng_update_frame() == FAILURE) return -1;
	}

	Eng_exit();
}
