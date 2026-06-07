#!/usr/bin/env bash
set -euo pipefail

paths=()
names=()
for path in "$(dirname "$0")/objects/"*; do
	paths+=("$path")

	name="$(basename "$path")"
	name="${name#*-}" # strip z-order
	name="${name%.*}" # strip extension
	names+=("$name")
done

cat <<-EOF
	#pragma once

	#include <SDL3_gfx/SDL3_gfxPrimitives.h>
	#include <SDL3_ttf/SDL_ttf.h>
	#include <la.h>
	#include <stdio.h>

	#include "camera.c"
	#include "collision.c"
	#include "input.c"
	#include "res.c"
	#include "utils.c"

	#define FIELDS(x)
EOF

echo 'typedef enum {'
echo '  OBJ_INVALID,'
for n in "${names[@]}"; do
	echo "  OBJ_${n^^},"
done
echo '} ObjID;'

for i in "${!names[@]}"; do
	name="${names[$i]}"
	type="${name^}"

	cat <<-EOF
		typedef struct {
				ObjID  oid;
				V2i64  pos; /* world coordinates              */
				V2i64  vel; /* with premultiplied FIXED_POINT */
				double rot; /* rotation in degrees            */
				double rvl; /* rotational velocity            */
				double scl; /* scale                          */
				Hitbox hit;
				$(grep -A99 FIELDS "${paths[$i]}" | sed '1d; /})/Q')
		} ${type};
	EOF

	echo "Pool(${name^});"
	echo "extern ${type}Pool ${name}Pool;"

	echo "${type}* ${name}Create(void);"
	echo "void    _${name}Create(${type}* self);"
	echo "void     ${name}Update(${type}* self);"
	echo "void     ${name}Render(${type}* self);"
	echo "void     ${name}Delete(${type}* self);"
done

cat <<-EOF
	#define OBJECTS $(printf 'X(%s) ' "${names[@]}")

	void objectDelete(void* obj);

	#if __INCLUDE_LEVEL__ == 0

	void objectDelete(void* obj) {
		 switch(*(ObjID*) obj) {
EOF

for name in "${names[@]}"; do
	echo "case OBJ_${name^^}: ${name}Delete((${name^}*) obj); break;"
done
echo "default: break; } }"

for name in "${names[@]}"; do
	echo "${name^}Pool ${name}Pool;"

	type="${name^}"
	cat <<-EOF
		${type}* ${name}Create(void) {
			${type}* new = PoolNew(${name}Pool);
			_${name}Create(new);
			new->oid = OBJ_${name^^};
			return new;
		}

		void ${name}Delete(${type}* self) {
			 self->oid = OBJ_INVALID;
			 PoolDel(${name}Pool, self);
		}
	EOF
done
echo '#endif'
