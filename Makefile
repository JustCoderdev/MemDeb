# JustCoderdev Makefile for C projects v4 (no semv)

PROGRAM_NAME = memdeb

# $(wildcard *.c)
SRC_FILES = main.c hotreload.c
OBJ_FILES = $(patsubst %.c,%.o,${SRC_FILES})
CORE_FILES = lib/core/core_bit.c \
		 lib/core/core_buff.c \
		 lib/core/core_net.c \
		 lib/core/core_str.c \
		 lib/core/core_memdeb.c

CC = gcc
CCFLAGS = -xc -std=c89 -Wall -Wextra -Werror -Wno-pedantic \
		 -Wno-unused -g
IFLAGS = -I. -I./lib/include
LDFLAGS = -Wl,-rpath=./bin/ -L./bin/
RAYFLAGS = -lGL -lm -lpthread -ldl -lX11 \
		-Wl,-rpath=./lib/raylib/ -L./lib/raylib -lraylib
#-lXrandr -lXinerama -lXi -lXcursor

#-DDEBUG_MEMDEB_ENABLE=1
#-DDEBUG_STRING_ENABLE=1
DFLAGS = -DDEBUG_ENABLE=1
FLAGS = $(CCFLAGS) $(IFLAGS) $(LDFLAGS) $(RAYFLAGS) $(DFLAGS)

local: build_so build run
build_so:
	@echo "Compiling... (auto)"
	@mkdir -p bin
	$(CC) $(CCFLAGS) $(IFLAGS) $(RAYFLAGS) $(DFLAGS) -fPIC -shared hotfile.c $(CORE_FILES) -o bin/libhotfile.so

build:
	@echo "Compiling... "
	@mkdir -p bin
	@cp -f memdump bin/memdump
	$(CC) $(FLAGS) $(SRC_FILES) $(CORE_FILES) -o bin/${PROGRAM_NAME}

run:
	@echo "Running... "
	@chmod +x bin/$(PROGRAM_NAME)
	@cd bin && ./$(PROGRAM_NAME)

.PHONY: clean
clean:
	@echo "Cleaning..."
	@rm -rf ./bin


# raycompile: rayclean raybuild
# rayclean:
# 	cd lib/sources/raylib/src && make clean

# raybuild:
# 	cd lib/sources/raylib/src && make PLATFORM=PLATFORM_DESKTOP GLFW_LINUX_ENABLE_WAYLAND=OFF

# rayshared:
# 	cd lib/sources/raylib/src && make PLATFORM=PLATFORM_DESKTOP RAYLIB_LIBTYPE=SHARED GLFW_LINUX_ENABLE_WAYLAND=OFF
