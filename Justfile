run: build
	./build/asteroids16plus.x86_64

build:
	#!/usr/bin/env bash
	if [ ! -d build ] || [ ! -f build/meson-private/coredata.dat ]; then
		just _wipe
	fi

	mkdir -p build/stamps

	touch src/gen_res.sh
	touch src/gen_objects.sh

	old="$(< build/stamps/src)"
	new="$(find src -name '*.c' | sort)"
	if [ "$old" != "$new" ]; then
	  meson setup --reconfigure build
	  echo "$new" > build/stamps/src
	fi

	meson compile -v -C build

wipe: _wipe run

_wipe:
	meson setup --wipe build
