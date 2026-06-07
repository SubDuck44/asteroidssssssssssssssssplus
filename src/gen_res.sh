#!/usr/bin/env bash
set -euo pipefail

decls=""
defns=""
array=""
count=0

for f in "$(dirname "$0")/../res/"*.png; do
	n="$(basename "$f")"
	n="${n%.png}"
	n="${n^^}"

	decls+="
extern Texture TEX_$n;"

	defns+="
static const uint8_t TEX_${n}_DATA[] = {
#embed \"../res/$(basename "$f")\"
};

Texture TEX_$n = {NULL, TEX_${n}_DATA, sizeof(TEX_${n}_DATA), 0, 0};
"

	array+="&TEX_$n, "
	((count += 1))
done

cat <<EOF
#pragma once

#include <SDL3/SDL.h>

typedef struct {
  SDL_Texture*   tex;
  const uint8_t* tex_data;
  size_t         tex_size;
  float          w, h;
} Texture;

#define TEXTURES_COUNT $count

extern Texture* TEXTURES[TEXTURES_COUNT];
$decls

#if __INCLUDE_LEVEL__ == 0
$defns
Texture* TEXTURES[] = {$array};
#endif
EOF
