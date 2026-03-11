{
  outputs = { flake-utils, nixpkgs, ... }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };
        inherit (pkgs.lib.fileset) toSource unions;
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
            ];
          };

          nativeBuildInputs = with pkgs; [
            meson
            ninja
            pkg-config
          ];

          buildInputs = with pkgs; [
            sdl3
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
