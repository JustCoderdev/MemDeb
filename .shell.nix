# C-Raylib shell #3

# # To build raylib
# cd raylib/src
# make clean
# make PLATFORM=PLATFORM_DESKTOP RAYLIB_LIBTYPE=SHARED GLFW_LINUX_ENABLE_WAYLAND=OFF

{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
	# nativeBuildInputs is tools you need to run
	nativeBuildInputs = with pkgs; [
		glibcInfo
		glibc

		arduino
		screen # serial console

		gnumake
		libGL
		gcc

		xorg.libXinerama
		xorg.libX11.dev
		xorg.libXft
		xorg.libXcursor
		xorg.libXrandr
		xorg.libXi.dev
	];

	#export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:${pkgs.glib.out}/lib"
	shellHook = ''
export PATH+=":${pkgs.arduino}/share/arduino/hardware/arduino/tools/avr/bin"
export LD_LIBRARY_PATH+=":$(pwd)/lib/raylib"
export LD_LIBRARY_PATH+=":$(pwd)/bin/"
rm -f .ldcache
ldconfig -C .ldcache

zsh
'';

}

