{
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  outputs = {
    self,
    nixpkgs,
  }: let
    pkgs = nixpkgs.legacyPackages.x86_64-linux;
  in rec {
    packages.x86_64-linux = {
      open3d = pkgs.llvmPackages.libcxxStdenv.mkDerivation rec {
        pname = "open3d";
        version = "0.18.0";

        src = pkgs.fetchFromGitHub {
          owner = "isl-org";
          repo = "Open3D";
          rev = "v${version}";
          hash = "sha256-VMykWYfWUzhG+Db1I/9D1GTKd3OzmSXvwzXwaZnu8uI=";
        };

        nativeBuildInputs = with pkgs; [cmake ninja];
        buildInputs = with pkgs; [
          curl.dev
          openssl
          assimp
          eigen
          embree
          fmt
          glew
          glfw
          imgui
          libjpeg
          jsoncpp
          lzfse
          msgpack
          nanoflann
          openssl
          libpng
          qhull
          tbb
          tinyobjloader
          vtk
          zeromq
          llvmPackages.libcxx
          llvmPackages.openmp
          msgpack-cxx
          boost
        ];
        cmakeFlags = [
          "-DBUILD_SHARED_LIBS=ON"
          # disable unneeded stuff
          "-DBUILD_EXAMPLES=OFF"
          "-DBUILD_PYTHON_MODULE=OFF"
          "-DBUILD_JUPYTER_EXTENSION=OFF"
          "-DBUILD_CUDA_MODULE=OFF"
          "-DBUILD_ISPC_MODULE=OFF"
          "-DENABLE_HEADLESS_RENDERING=OFF"
          # enable stuff
          "-DBUILD_GUI=OFF" # wants to download mesa :(
          "-DWITH_OPENMP=ON"
          "-DWITH_IPPICV=ON"
          "-DBUILD_FILAMENT_FROM_SOURCE=ON"
          # use system stuff
          "-DUSE_SYSTEM_ASSIMP=ON"
          "-DUSE_SYSTEM_CURL=OFF" # problems with openssl
          "-DUSE_SYSTEM_CUTLASS=OFF" # not packaged
          "-DUSE_SYSTEM_EIGEN3=ON"
          "-DUSE_SYSTEM_EMBREE=ON"
          "-DUSE_SYSTEM_FILAMENT=OFF" # not packaged
          "-DUSE_SYSTEM_FMT=ON"
          "-DUSE_SYSTEM_GLEW=ON"
          "-DUSE_SYSTEM_GLFW=ON"
          #"-DUSE_SYSTEM_GOOGLETEST=ON"
          "-DUSE_SYSTEM_IMGUI=ON"
          "-DImGui_LIBRARY=${pkgs.imgui}/lib/libimgui"
          "-DCPP_LIBRARY=${pkgs.llvmPackages.libcxx}/lib/libc++"
          "-DCPPABI_LIBRARY=${pkgs.llvmPackages.libcxx}/lib/libc++abi"
          "-DUSE_SYSTEM_JPEG=ON"
          "-DUSE_SYSTEM_JSONCPP=ON"
          "-DUSE_SYSTEM_LIBLZF=OFF" # not packaged
          "-DUSE_SYSTEM_MSGPACK=ON"
          "-DUSE_SYSTEM_NANOFLANN=ON"
          "-DUSE_SYSTEM_OPENSSL=OFF" # problems with curl
          "-DUSE_SYSTEM_PNG=ON"
          "-DUSE_SYSTEM_OPENGL=ON"
          #"-DUSE_SYSTEM_PYBIND11=ON"
          "-DUSE_SYSTEM_QHULLCPP=ON"
          "-DUSE_SYSTEM_STDGPU=ON"
          "-DUSE_SYSTEM_TBB=ON"
          "-DUSE_SYSTEM_TINYGLTF=OFF"
          "-DUSE_SYSTEM_TINYOBJLOADER=ON"
          "-DUSE_SYSTEM_VTK=ON"
          "-DUSE_SYSTEM_ZEROMQ=ON"
          "-DUVATLAS_LIBRARIES=/tmp/dummy"
        ];
      };

      mesh-voxelization = pkgs.stdenv.mkDerivation rec {
        pname = "mesh-voxelization";
        version = "2022-02-10";

        src = pkgs.fetchFromGitHub {
          owner = "davidstutz";
          repo = pname;
          rev = "81a237c3b345062e364b180a8a4fc7ac98e107a4";
          hash = "sha256-HSSV3KajrTnX7oWtC+JYXuuXu8pkcVz63wFm+ADx2rk=";
        };
        nativeBuildInputs = with pkgs; [cmake ninja];
        buildInputs = with pkgs; [
          hdf5
          boost
          eigen
        ];
      };

      vtk = pkgs.vtk.overrideAttrs rec {
        postFixup = ''
          # hack together includes
          cp -rv $out/include/vtk/* $out/include/
        '';
      };

      vtkbool = pkgs.stdenv.mkDerivation rec {
        pname = "vtkbool";
        version = "3.0.1";

        src = pkgs.fetchFromGitHub {
          owner = "zippy84";
          repo = pname;
          rev = "v${version}";
          hash = "sha256-3yMu8GiqRSsgjyI+oKmO7ekvQVSRJlf8G/FxR9JaPHM=";
        };

        nativeBuildInputs = with pkgs; [cmake ninja];
        buildInputs = with pkgs; [vtk];

        cmakeFlags = [
          "-DCMAKE_INSTALL_LIBDIR=lib"
          "-DCMAKE_INSTALL_INCLUDEDIR=include"
          "-DCMAKE_INSTALL_BINDIR=bin"
        ];

        postFixup = ''
          cp ${src}/Utilities.h $out/include
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
        #packages.x86_64-linux.vtkbool
        pkgs.cgal

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
