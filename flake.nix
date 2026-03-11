{
  outputs = { flake-utils, nixpkgs, ... }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = import nixpkgs { inherit system; };

        cargoNix = import ./Cargo.nix {
          inherit pkgs;

          buildRustCrateForPkgs = _: args: (pkgs.buildRustCrate.override {
            defaultCrateOverrides = pkgs.defaultCrateOverrides // {
              asteroidssssssssssssssss = old: {
                buildInputs = with pkgs; [
                  sdl3
                ];

                extraRustcOpts = (old.extraRustcOpts or [ ]) ++
                  [ "-C" "link-arg=-lSDL3" ];
              };
            };
          }) (args // {
            extraRustcOpts = (args.extraRustcOpts or [ ]) ++
              [ "-C" "link-arg=-fuse-ld=mold" ];

            nativeBuildInputs = with pkgs; [
              mold
            ];
          });
        };
      in
      rec {
        packages.default = cargoNix.rootCrate.build;

        devShells.default = pkgs.mkShell {
          inputsFrom = [ packages.default ];
          packages = with pkgs; [
            crate2nix
          ];
        };
      });
}
