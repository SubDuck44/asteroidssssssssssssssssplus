#pragma once

#include "object.c"
#include <SDL3_ttf/SDL_ttf.h>

FIELDS({
	TTF_Text* text; //
})

void _statsCreate(Stats* self) {
	(void) self;

	*self = (Stats) {
		.text = TTF_CreateText(textEngine, font, "[PLACEHOLDER]", 0),
		.pos  = (V2i64) {
			 .x = WINDOW_WIDTH / 2,
			 .y = 50,
        },
	};

	if(!self->text) {
		printf(FAIL("failed to create text for stats\n"));
		abort();
	}
}

void statsUpdate(Stats* self) {
	char buf[1024] = {0};
	snprintf(
		buf, sizeof(buf), //
		"SCORE: %u",      //
		score             //
	);

	TTF_SetTextString(self->text, buf, sizeof(buf));
}

void statsRender(Stats* self) {
	int w = 0;
	int h = 0;
	TTF_GetTextSize(self->text, &w, &h);

	TTF_DrawRendererText(
		self->text,                  //
		(float) self->pos.x - w / 2, //
		(float) self->pos.y - h / 2  //
	);
}
