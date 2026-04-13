#pragma once

#define _POSIX_C_SOURCE 200809L

#include "utils.c"

enum ReplCommands : uint8_t {
	COMMAND_INVALID,
	COMMAND_SPAWN_ASTEROID,
	COMMAND_EXIT,
};

typedef struct {
	uint8_t        command;
	SDL_Thread*    thread;
	SDL_Semaphore* semaphore;
} DebugRepl;

extern DebugRepl Repl_repl;

Error Repl_init(void);
int   Repl_run(void* data);

#if __INCLUDE_LEVEL__ == 0 /////////////////////////////////////////////////////

DebugRepl Repl_repl = {0};

Error Repl_init(void) {
	Repl_repl.thread = SDL_CreateThread(Repl_run, "DebugRepl", &Repl_repl);
	ASSERT_PREDICATE_SDL(
		Repl_repl.thread, return ERR_FATAL;
		,
		CODE_SUCCESS "INFO: Successfully forked thread for DebugRepl" CODE_END,
		CODE_ERROR "FATAL: Failed to fork thread for DebugRepl" CODE_END
	);
	Repl_repl.semaphore = SDL_CreateSemaphore(0);
	ASSERT_PREDICATE_SDL(
		Repl_repl.semaphore, return ERR_FATAL;
		,
		CODE_SUCCESS
		"INFO: Successfully created semaphore for DebugRepl" CODE_END,
		CODE_ERROR "FATAL: Failed to create semaphore for DebugRepl" CODE_END
	);

	return ERR_PASS;
}

int Repl_run(void* data) {
	DebugRepl* self = data;
	char*      buf  = NULL;
	size_t     cap  = 0;
	ssize_t    len  = 0;

	while(true) {
		SDL_Log(">");

		len = getline(&buf, &cap, stdin);

		if(len > 0) SDL_Log("%s", buf);

		if(len == -1) {
			self->command = COMMAND_EXIT;
			SDL_SignalSemaphore(self->semaphore);
			return 0;
		}
	}
}

#endif
