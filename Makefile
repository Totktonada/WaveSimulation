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

OBJMODULES = $(SRCMODULES:.c=.o)
HEADERS = $(SRCMODULES:.c=.h)
EXEC_FILE = WaveSimulation

DEBUG_CFLAGS = -g -DDEBUG=
RELEASE_CFLAGS = -O2

WARNINGS = -Wall -Wextra
DEFINE = -D_POSIX_C_SOURCE=200112L

CFLAGS = -ansi -pedantic $(WARNINGS) $(RELEASE_CFLAGS) $(DEFINE)
LDFLAGS = -lm -lGL -lGLEW -lglfw

default: $(EXEC_FILE)

%.o: %.c %.h
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
	rm -f *.o $(EXEC_FILE) deps.mk *.core core

clang_analyze_clean:
	rm -f *.h.gch *.plist
