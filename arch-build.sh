#!/usr/bin/env bash
set -euo pipefail
self="$(realpath "$(dirname "$0")")"

if ! command -v git; then sudo pacman -S --noconfirm git; fi
if ! pacman -Q base-devel; then sudo pacman -S --noconfirm base-devel; fi

if ! command -v yay; then
	d="$(mktemp -d)"
	pushd "$d"
	git clone https://aur.archlinux.org/yay-bin yay
	cd yay
	makepkg -si
	popd
	rm -rf "$d"
fi

# sdl3-gfx is in the AUR
yay -S --noconfirm meson ninja pkg-config sdl3 sdl3_image sdl3_ttf sdl3_gfx-git

la="$(mktemp -d)"
git clone https://github.com/tsoding/la "$la"
cd "$la"
git checkout 09985aa1d948936e28ea9de094572cf8d0ac48e4
git apply "$self/la-types.patch"
cc -I thirdparty src/lag.c -o lag
./lag > la.h
cc \
	-Wall -Wextra -Werror \
	-Wno-pragma-once-outside-header \
	-D LA_IMPLEMENTATION \
	-fPIC -shared -O3 -x c \
	la.h -o la.so
sed -i '/LA_IMPLEMENTATION/Q' la.h
cat << EOF > la.pc
Name: la
Version: 0
Description:
Cflags: -I$PWD
Libs: -L$PWD -lm -lla
EOF

cd "$self"
export PKG_CONFIG_PATH="$la"
rm -rf build
meson setup build
meson compile -C build
