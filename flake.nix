{
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  outputs = {
    self,
    nixpkgs,
  }: let
    pkgs = nixpkgs.legacyPackages.x86_64-linux;
  in rec {
    packages.x86_64-linux = {
      vtk = pkgs.vtk.overrideAttrs rec {
        postFixup = ''
          # hack together includes
          cp -rv $out/include/vtk/* $out/include/
        '';
      };
      # use our own cgal package instead of upstream to enable qt support
      cgal-qt = pkgs.stdenv.mkDerivation rec {
        pname = "cgal";
        version = "5.5.3";

        src = pkgs.fetchurl {
          url = "https://github.com/CGAL/cgal/releases/download/v${version}/CGAL-${version}.tar.xz";
          hash = "sha256-CgT2YmkyVjKLBbq/q7XjpbfbL1pY1S48Ug350IKN3XM=";
        };

        # note: optional component libCGAL_ImageIO would need zlib and opengl;
        #   there are also libCGAL_Qt{3,4} omitted ATM
        buildInputs = with pkgs; [boost gmp mpfr qt5.qtbase];
        nativeBuildInputs = with pkgs; [cmake ninja];

        inherit (pkgs.cgal) patches;

        postFixup = ''
          # fix (hack?) qt graphics viewer support

          substituteInPlace $out/lib/cmake/CGAL/CGALConfig.cmake \
            --replace-fail "set(CGAL_GRAPHICSVIEW_PACKAGE_DIR \''${CGAL_ROOT})" "set(CGAL_GRAPHICSVIEW_PACKAGE_DIR $out)"

          mv "$out/lib/cmake/CGAL/demo" "$out"
          mv "$out/include/CGAL/Qt/resources/ImageInterface.ui" "$out/include/CGAL/Qt"
        '';

        dontWrapQtApps = true;
        doCheck = false;
      };
    };

    devShells.x86_64-linux.default = pkgs.mkShell {
      buildInputs = [
        pkgs.qtcreator
        pkgs.cmake
        pkgs.ninja
        pkgs.gdb
        pkgs.qt5.full # cgal doesn't support qt6 yet
        pkgs.qt5.qtbase
        #packages.x86_64-linux.cgal-qt
        #pkgs.gmp
        #pkgs.mpfr
        #pkgs.boost
        packages.x86_64-linux.vtk

        # this is for the shellhook portion
        #qt6.wrapQtAppsHook
        #makeWrapper
        #bashInteractive
      ];
      # set the environment variables that Qt apps expect
      # https://galowicz.de/2023/01/16/cpp-qt-qml-nix-setup/
      #shellHook = ''
      #  bashdir=$(mktemp -d)
      #  makeWrapper "$(type -p bash)" "$bashdir/bash" "''${qtWrapperArgs[@]}"
      #  exec "$bashdir/bash"
      #'';
    };
  };
}
