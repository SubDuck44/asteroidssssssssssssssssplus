#include "object.c"

FIELDS({
	char      buf[8192];
	TTF_Text* txt;
})

void _debugCreate(Debug* self) {
	*self = (Debug) {
		.txt = TTF_CreateText(textEngine, font, "", 0),
	};
}

void debugUpdate(Debug* self) {
	V2i64 mouseWld = cam2wld(mousePos);

	snprintf(
		self->buf, sizeof(self->buf),
		"FPS: %f\n"               //
		"Cam Pos: %ld %ld\n"      //
		"Cam Ctr: %ld %ld\n"      //
		"Mouse Pos: %f %f\n"      //
		"Pos at mouse: %ld %ld\n" //
#define X(name) #name "Pool alloc: %zu - %zu\n"
		OBJECTS
#undef X
		"%s",                //
		fps,                 //
		V2i64_Arg(cam.pos),  //
		V2i64_Arg(cam.ctr),  //
		V2d_Arg(mousePos),   //
		V2i64_Arg(mouseWld), //
#define X(name) name##Pool.used.len, name##Pool.free.len,
		OBJECTS
#undef X
		""
	);
	TTF_SetTextString(self->txt, self->buf, 0);
}

void debugRender(Debug* self) {
	if(!debugVisible) return;
	TTF_DrawRendererText(self->txt, 20, 20);
}
