run: build
    ./build/asteroids16plus.x86_64

build:
    if [ ! -d build ]; then just _wipe; fi
    touch src/gen_res.sh
    meson compile -C build

wipe: _wipe run

_wipe:
    meson setup --wipe build