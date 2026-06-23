run: build
	./build/asteroids16plus.x86_64

build:
	#!/usr/bin/env bash
	if [ ! -d build ] || [ ! -f build/meson-private/coredata.dat ]; then
		just _wipe
	fi

	mkdir -p build/stamps

	old="$(< build/stamps/res)"
	new="$(stat res -c %Y; sha256sum "$iosevka")"
	if [ "$old" != "$new" ]; then
		touch src/gen_res.sh
		echo "$new" > build/stamps/res
	fi

	old="$(< build/stamps/obj)"
	new="$(find src/objects/ -type f -exec sha256sum {} \; | sort)"
	if [ "$old" != "$new" ]; then
		touch src/gen_objects.sh
		echo "$new" > build/stamps/obj
	fi

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
