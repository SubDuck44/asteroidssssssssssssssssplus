{
  outputs = { flake-utils, nixpkgs, ... }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
        inherit (pkgs.lib.fileset) toSource unions;

        sdl3-gfx = pkgs.stdenv.mkDerivation {
          pname = "sdl3-gfx";
          version = "1.0.1-whocares";

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
            meson
            ninja
            pkg-config
          ];

          buildInputs = with pkgs; [
            sdl3
            sdl3-ttf
            sdl3-gfx
          ];
        };

        devShells.default = pkgs.mkShell {
          inputsFrom = [ packages.default ];

          packages = with pkgs; [
            clang-tools
            just
          ];
        };
      }
    );
}
