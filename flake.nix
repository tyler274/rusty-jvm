{
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs";
  };

  outputs = { self, nixpkgs }: 
    let pkgs = nixpkgs.legacyPackages.x86_64-linux;
    in {
      packages.x86_64-linux.hello = pkgs.hello; 

      devShells.x86_64-linux.default =
        (pkgs.mkShell.override { stdenv = pkgs.llvmPackages_13.stdenv; }) { 
          buildInputs = [
          pkgs.pkgconfig
          pkgs.python 
          pkgs.cmake
          pkgs.openssl 
          pkgs.zlib 
          pkgs.libgit2
          pkgs.libxml2 
          pkgs.pcre
          pkgs.xorg.libX11
          pkgs.ncurses
          pkgs.gtk3
          pkgs.glib
          pkgs.vulkan-loader
          pkgs.llvmPackages.libclang
          pkgs.rr
          ];

          shellHook = ''
            export LD_LIBRARY_PATH="${pkgs.vulkan-loader}/lib:${pkgs.xorg.libX11.out}/lib:${pkgs.llvmPackages.libclang}/lib"
          ''; 
        };
    };

}