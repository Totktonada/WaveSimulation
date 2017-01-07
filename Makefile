SRCMODULES = \
	utils.c \
	camera.c \
	scene.c \
	matrix.c \
	texture.c \
	shaders_errors.c \
	shaders.c \
	world_lexer.c \
	mesh.c \
	world.c \
	water.c \
	main.c

ifeq (win, $(MAKECMDGOALS))
OBJ_EXT = .obj
EXE_EXT = .exe
OS_CFLAGS = -Iwin-libs/glew/include -Iwin-libs/glfw/include -DGLEW_STATIC
OS_CFLAGS += -std=c99  # -Wimplicit-function-declaration: sinf/cosf/tanf/sqrtf
CC = i686-mingw32-gcc
LD = i686-mingw32-ld
AR = i686-mingw32-ar
OS_LDFLAGS = -static -Lwin-libs/glew/lib -Lwin-libs/glfw/lib-mingw
OS_GL_NAME = opengl32
OS_GLEW_NAME = glew32
OS_GLFW_NAME = glfw3
OS_LD_GDI = -lgdi32
else
OBJ_EXT = .o
OS_GL_NAME = GL
OS_GLEW_NAME = GLEW
OS_GLFW_NAME = glfw
endif

OBJMODULES = $(SRCMODULES:.c=$(OBJ_EXT))
HEADERS = $(SRCMODULES:.c=.h)
EXEC_FILE = WaveSimulation$(EXE_EXT)

DEBUG_CFLAGS = -g -DDEBUG=
RELEASE_CFLAGS = -O2

WARNINGS = -Wall -Wextra
DEFINE = -D_POSIX_C_SOURCE=200112L

CFLAGS = -ansi -pedantic $(WARNINGS) $(RELEASE_CFLAGS) $(OS_CFLAGS) $(DEFINE)
LDFLAGS = $(OS_LDFLAGS) -lm -l$(OS_GLEW_NAME) -l$(OS_GLFW_NAME) $(OS_LD_GDI) -l$(OS_GL_NAME)

default: $(EXEC_FILE)
win: $(EXEC_FILE)

%$(OBJ_EXT): %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

$(EXEC_FILE): $(OBJMODULES)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

ifneq (clean, $(MAKECMDGOALS))
ifneq (clang_analyze_clean, $(MAKECMDGOALS))
-include deps.mk
endif
endif

deps.mk: $(SRCMODULES) $(HEADERS)
	$(CC) -MM $^ > $@

clean:
	rm -f *.o *.obj WaveSimulation WaveSimulation.exe deps.mk *.core core

clang_analyze_clean:
	rm -f *.h.gch *.plist
