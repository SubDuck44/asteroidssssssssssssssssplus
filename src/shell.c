#pragma once

#include <SDL3/SDL.h>
#include <stdint.h>

#define ASCII_RETURN 13

typedef struct {
	char*    input_buf;
	uint32_t input_len;
	uint32_t input_cap;
	char*    history_buf;
	uint32_t history_cap;
	uint32_t history_next;
} Shell;

int  shell_init(Shell* dest, const uint32_t input_cap, const uint32_t hist_cap);
void shell_exit(Shell* target);
int  shell_write_char(Shell* target, const char character);
int  shell_flush(Shell* target);
void shell_append(Shell* target, char* src, size_t len);
int  shell_write(Shell* target, char* src, size_t len);
int  shell_write_exec(Shell* target, char* src, size_t len, const bool hide);

#if __INCLUDE_LEVEL__ == 0 /////////////////////////////////////////////////////

int shell_init(Shell* dest, const uint32_t input_cap, const uint32_t hist_cap) {
	*dest = (Shell) {.input_buf    = SDL_calloc(input_cap, sizeof(char)),
	                 .input_cap    = input_cap,
	                 .input_len    = 0,
	                 .history_buf  = SDL_calloc(hist_cap, sizeof(char)),
	                 .history_cap  = hist_cap,
	                 .history_next = 0};
	if(!dest->history_buf || !dest->input_buf) {
		SDL_Log("Failed to allocate memory for shell_init");
		return 0;
	}
	return 1;
}

void shell_exit(Shell* target) {
	SDL_free(target->history_buf);
	SDL_free(target->input_buf);
	*target = (Shell) {0};
}

int shell_flush(Shell* target) {
	if(target->input_len <= target->input_cap >> 1) {
		size_t new_size = target->input_cap >> 1;
		char*  tmp = SDL_realloc(target->input_buf, sizeof(char) * new_size);
		if(!tmp) {
			SDL_Log(
				"Failed to deallocate memory for shell input_buf shrinking"
			);
			return 0;
		}
		target->input_buf = tmp;
		target->input_cap = new_size;
	}
	SDL_memset(target->input_buf, 0, target->input_cap * sizeof(char));
	target->input_len = 0;
	return 1;
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

int shell_write_char(Shell* target, const char character) {
	if(character == ASCII_RETURN) {
		shell_append(target, target->input_buf, target->input_len);
		shell_parse(target, target->input_buf);
		shell_flush(target);
	}
	if(target->input_len + 1 > target->input_cap) {
		size_t new_size = target->input_cap << 1;
		char*  tmp = SDL_realloc(target->input_buf, sizeof(char) * new_size);
		if(!tmp) {
			SDL_Log("Failed to allocate memory for shell input_buf expansion");
			return 0;
		}
		target->input_buf = tmp;
		target->input_cap = new_size;
	}
	target->input_buf[target->input_len] = character;
	target->input_len++;
	return 1;
}

int shell_write(Shell* target, char* src, size_t len) {
	if(len + target->input_len > target->input_cap) {
		size_t new_size = target->input_cap << 1;
		while(new_size < len + target->input_len) {
			new_size *= 2;
		}
		char* tmp = SDL_realloc(target->input_buf, sizeof(char) * new_size);
		if(!tmp) {
			SDL_Log("Failed to allocate memory for shell input_buf expansion");
			return 0;
		}
		target->input_buf = tmp;
		target->input_cap = new_size;
	}
	SDL_memcpy(target->input_buf + target->input_len, src, sizeof(char) * len);
	return 1;
}

int shell_write_exec(Shell* target, char* src, size_t len, const bool hide) {
	SDL_memset(target->input_buf, 0, sizeof(char) * target->input_len);
	target->input_len = 0;
	if(!shell_write(target, src, len)) return 0;
	if(!hide) shell_append(target, target->input_buf, target->input_len);
	shell_parse(
		target, target->input_buf
	); // TODO Add input checking in case parse requires it in the future
	if(!shell_flush(target)) return 0;
	return 1;
}

#endif
