#include <SDL3/SDL_main.h> // Dont move this or DIE
#include <stdio.h>

#include "engine.c"

int main(int argc, char* argv[]) {
	/* INIT */
	(void) argc;
	(void) argv;
	if(!Eng_init()) return -1;

	/* LOOP */
	for(;;) {
		if(!Eng_update_frame()) break;
	}

	SDL_Quit();
	TTF_Quit();

	printf("INFO: Goodbye!\n");
}
