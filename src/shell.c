#pragma once

#include <SDL3/SDL.h>
#include <stdint.h>

#define ASCII_RETURN 13

typedef struct {
	char*    history_buf;
	uint32_t history_cap;
	uint32_t history_next;
} Shell;

int  shell_init(Shell* dest, const uint32_t hist_cap);
void shell_exit(Shell* target);
void shell_parse(Shell* target, char* src);
void shell_append(Shell* target, char* src, size_t len);

#if __INCLUDE_LEVEL__ == 0 /////////////////////////////////////////////////////

int shell_init(Shell* dest, const uint32_t hist_cap) {
	*dest = (Shell) {.history_buf  = SDL_calloc(hist_cap, sizeof(char)),
	                 .history_cap  = hist_cap,
	                 .history_next = 0};
	if(!dest->history_buf) {
		SDL_Log("Failed to allocate memory for shell_init");
		return 0;
	}
	return 1;
}

void shell_exit(Shell* target) {
	SDL_free(target->history_buf);
	*target = (Shell) {0};
}
void shell_parse(Shell* target, char* src) {
	(void) target;
	(void) src;
} // TODO

void shell_append(Shell* target, char* src, size_t len) {
	for(size_t it = 0; it < len; it++) {
		target->history_buf[target->history_next] = src[it];
		target->history_next = (target->history_next + 1) % target->history_cap;
	}
}

#endif
