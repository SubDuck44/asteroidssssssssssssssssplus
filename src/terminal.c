#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "main.c"
#include "shell.c"

typedef struct {
	char*     input_buf;
	uint32_t  input_len;
	uint32_t  input_cap;
	char*     frame_buf;
	TTF_Text* display;
	Shell*    attached_shell;
	uint16_t  columns;
	uint16_t  rows;
} Terminal;

int terminal_init(
	Terminal* dest, Shell* attach_shell, const uint32_t input_cap,
	const uint16_t cols, const uint16_t rows
);
int terminal_flush(Terminal* target);
int terminal_write(Terminal* term, char* src, size_t len);
int terminal_flush(Terminal* target);

#if __INCLUDE_LEVEL__ == 0 /////////////////////////////////////////////////////

int terminal_init(
	Terminal* dest, Shell* attach_shell, const uint32_t input_cap,
	const uint16_t cols, const uint16_t rows
) {
	*dest = (Terminal) {
		.input_buf      = SDL_calloc(input_cap, sizeof(char)),
		.input_cap      = input_cap,
		.input_len      = 0,
		.attached_shell = attach_shell,
		.columns        = cols,
		.rows           = rows,
		.frame_buf      = SDL_calloc(cols * rows, sizeof(char)),
	};
	TTF_CreateText(
		engine.text_engine, engine.font, dest->frame_buf, cols * rows
	);
	if(!dest->input_buf) {
		SDL_Log("Failed to allocate memory for terminal_init");
		return 0;
	}
	return 1;
}

void terminal_exit(Terminal* term) {
	if(!(term->attached_shell == &engine.std_shell)) {
		shell_exit(term->attached_shell);
	}
	SDL_free(term->input_buf);
	SDL_free(term->frame_buf);
	TTF_DestroyText(term->display);
	*term = (Terminal) {0};
}

int terminal_write_char(Terminal* term, const char character, const bool hide) {
	if(character == ASCII_RETURN) {
		if(!hide)
			shell_append(
				term->attached_shell, term->input_buf, term->input_len
			);
		shell_parse(term->attached_shell, term->input_buf);
		terminal_flush(term);
	}
	if(term->input_len + 1 > term->input_cap) {
		size_t new_size = term->input_cap << 1;
		char*  tmp      = SDL_realloc(term->input_buf, sizeof(char) * new_size);
		if(!tmp) {
			SDL_Log(
				"Failed to allocate memory for terminal input_buf expansion"
			);
			return 0;
		}
		term->input_buf = tmp;
		term->input_cap = new_size;
	}
	term->input_buf[term->input_len] = character;
	term->input_len++;
	return 1;
}

int terminal_write(Terminal* term, char* src, size_t len) {
	if(len + term->input_len > term->input_cap) {
		size_t new_size = term->input_cap << 1;
		while(new_size < len + term->input_len) {
			new_size *= 2;
		}
		char* tmp = SDL_realloc(term->input_buf, sizeof(char) * new_size);
		if(!tmp) {
			SDL_Log(
				"Failed to allocate memory for terminal input_buf expansion"
			);
			return 0;
		}
		term->input_buf = tmp;
		term->input_cap = new_size;
	}
	SDL_memcpy(term->input_buf + term->input_len, src, sizeof(char) * len);
	return 1;
}

int terminal_write_exec(
	Terminal* term, char* src, size_t len, const bool hide
) {
	SDL_memset(term->input_buf, 0, sizeof(char) * term->input_len);
	term->input_len = 0;
	if(!terminal_write(term, src, len)) return 0;
	if(!hide)
		shell_append(term->attached_shell, term->input_buf, term->input_len);
	shell_parse(
		term->attached_shell, term->input_buf
	); // TODO Add input checking in case parse requires it in the future
	if(!terminal_flush(term)) return 0;
	return 1;
}

int terminal_flush(Terminal* target) {
	if(target->input_len <= target->input_cap >> 1) {
		size_t new_size = target->input_cap >> 1;
		char*  tmp = SDL_realloc(target->input_buf, sizeof(char) * new_size);
		if(!tmp) {
			SDL_Log(
				"Failed to deallocate memory for terminal input_buf shrinking"
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

void terminal_draw(Terminal* term) {}

#endif
