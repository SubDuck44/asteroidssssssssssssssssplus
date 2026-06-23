{
  outputs = { flake-utils, nixpkgs, ... }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
        inherit (pkgs.lib.fileset) toSource unions;

        sdl3-gfx = pkgs.stdenv.mkDerivation {
          pname = "sdl3-gfx";
          version = "1.0.1-unstable-2026-02-02";

          src = pkgs.fetchFromGitHub {
            owner = "sabdul-khabir";
            repo = "sdl3_gfx";
            rev = "0bbee988bb0caa3e98a9d78c7a2d106925c8275a";
            hash = "sha256-uHyCXYTv8D2DzuLSyIsgYfWgtrCdC5UiZEYhUdFzNOk=";
          };

          nativeBuildInputs = with pkgs; [
            cmake
          ];

          buildInputs = with pkgs; [
            sdl3
          ];
        };

        la = pkgs.stdenv.mkDerivation (drv: {
          pname = "la";
          version = "0-unstable-2026-05-07";

          src = pkgs.fetchFromGitHub {
            owner = "tsoding";
            repo = drv.pname;
            rev = "09985aa1d948936e28ea9de094572cf8d0ac48e4";
            hash = "sha256-DoersVH1dha7NUZFruam0M9LCJJTTO8mOJspsxHBacs=";
          };

          patches = [
            ./la-types.patch
          ];

          nativeBuildInputs = with pkgs; [
            pkg-config
          ];

          buildPhase = ''
            cc -I thirdparty src/lag.c -o lag
            ./lag > la.h

            cc                                \
              -Wall -Wextra -Werror           \
              -Wno-pragma-once-outside-header \
              -D LA_IMPLEMENTATION            \
              -fPIC -shared -O3 -x c          \
              la.h -o la.so

            sed -i '/LA_IMPLEMENTATION/Q' la.h

            cat << EOF > la.pc
            prefix=$out
            includedir=\''${prefix}/include
            libdir=\''${prefix}/lib

            Name: ${drv.pname}
            Version: ${drv.version}
            Description:
            Cflags: -I\''${includedir}
            Libs: -L\''${libdir} -lm -l${drv.pname}
            EOF
          '';

          installPhase = ''
            install -Dm555 {,$out/include/}la.h
            install -Dm555 {,$out/lib/}la.so
            install -Dm555 {,$out/lib/pkgconfig/}la.pc
          '';
        });

        python = pkgs.python3.withPackages (p: with p; [
          fonttools
        ]);

        env = {
          iosevka = "${pkgs.iosevka}/share/fonts/truetype/Iosevka-Regular.ttf";
        };
      in
      rec {
        packages.default = pkgs.stdenv.mkDerivation {
          pname = "asteroids16plus";
          version = "0.0.1";

          src = toSource {
            root = ./.;
            fileset = unions [
              ./meson.build
              ./src
              ./res
            ];
          };

          nativeBuildInputs = with pkgs; [
            breakpointHook
            meson
            ninja
            pkg-config
            python
          ];

          buildInputs = with pkgs; [
            la
            sdl3
            sdl3-gfx
            sdl3-ttf
          ];

          inherit env;

          mesonBuildType = "release";
          mesonFlags = [ "--werror" ];

          prePatch = ''
            patchShebangs src/gen_*
          '';

          preConfigure = ''
            meson rewrite kwargs set project / version "$version"
          '';
        };

        devShells.default = pkgs.mkShell {
          inputsFrom = [ packages.default ];

          packages = with pkgs; [
            clang-tools
            gdb
            imagemagick
            just
            tokei
            valgrind
          ];

          inherit env;
        };
      }
    );
}
