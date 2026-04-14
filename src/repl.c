#pragma once

#define _POSIX_C_SOURCE 200809L
#define REPL_ARGBUF_SIZE 16

#include <stdio.h>

#include "utils.c"

enum ReplCommands : uint8_t {
	COMMAND_INVALID,
	COMMAND_ECHO,
	COMMAND_EXIT,
	COMMAND_SPAWN_ASTEROID,
	COMMAND_NUM,
};

typedef struct {
	uint8_t        command;
	SDL_Thread*    thread;
	SDL_Semaphore* semaphore;
	char*          buf;
	size_t         cap;
	ssize_t        len;
} DebugRepl;

extern DebugRepl Repl_repl;

Error Repl_init(void);
int   Repl_run(void* data);

#if __INCLUDE_LEVEL__ == 0 /////////////////////////////////////////////////////

const char* commands[] = {
	" ",
	"echo",
	"exit",
	"spawn",
};

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

static char* try_get_arg(uint16_t index, char** arg_buf, uint16_t arg_buf_len) {
	if(index > arg_buf_len - 1) return NULL;
	if(strlen(arg_buf[index]) <= 0) return NULL;
	return arg_buf[index];
}

int Repl_run(void* data) {
	DebugRepl* self = data;
	while(true) {
		// Asteroids read and integrated evaluation loop
		printf("asriel: ");

		self->len = getline(&self->buf, &self->cap, stdin);

		char*    arg_buf[REPL_ARGBUF_SIZE];
		uint16_t arg_buf_len = 0;
		char*    last        = self->buf;
		for(uint16_t i = 0; i < self->len && arg_buf_len <= REPL_ARGBUF_SIZE;
		    i++) {
			if(self->buf[i] == ' ' || self->buf[i] == '\n') {
				self->buf[i]         = '\0';
				arg_buf[arg_buf_len] = last;
				arg_buf_len++;
				last = &self->buf[mini(self->len - 1, i + 1)];
			};
		}

		uint16_t match = 0;

		for(uint16_t i = 0; i < sizeof(commands) / sizeof(char*); i++) {
			if(!strncmp(commands[i], self->buf, self->len)) {
				match = i;
				break;
			}
		}

		char* arg = NULL;

		switch(match) {
		case COMMAND_INVALID:
			SDL_Log("ERROR: Could not find command \"%s\"", self->buf);
			break;
		case COMMAND_ECHO:
			arg = try_get_arg(1, arg_buf, arg_buf_len);
			if(!arg) {
				SDL_Log("ERROR: null arg to echo");
				break;
			}
			SDL_Log("%s", arg);
			break;
		case COMMAND_EXIT:
			self->command = COMMAND_EXIT;
			SDL_SignalSemaphore(self->semaphore);
			break;
		case COMMAND_SPAWN_ASTEROID:
			self->command = COMMAND_SPAWN_ASTEROID;
			SDL_SignalSemaphore(self->semaphore);
			break;
		default:
			SDL_Log("You shouldn't have seen this");
			break;
		}

		if(self->len == -1) {
			self->command = COMMAND_EXIT;
			SDL_SignalSemaphore(self->semaphore);
			return 0;
		}
	}
}

#endif
