OS = $(shell uname)

LIBS = libavcodec libavformat libavutil libswresample libswscale epoxy freetype2 luajit portaudio-2.0
ifneq ($(OS),Linux)
	LIBS += glfw3
endif

LPATH=-L/usr/local/lib

CFLAGS = -g -ggdb -Iinclude -I. -Wall -UCGLM_SIMD $(shell pkg-config --cflags $(LIBS))
LFLAGS = $(shell pkg-config --libs $(LIBS)) -lm
ifeq ($(OS),Linux)
	LFLAGS += -lEGL -lGL
else
	ifeq ($(OS),Darwin)
		LFLAGS += -framework OpenGL
	else
		LFLAGS += -lopengl32 
	endif
endif

OBJS := $(patsubst src/%.c, obj/%.o, $(wildcard src/*.c))
DEPS := $(patsubst src/%.c,obj/%.d,$(wildcard src/*.c))
CC = clang
EXEC = main

all: $(DEPS) $(EXEC)

obj:
	[ -d obj ] || mkdir obj

# obj/codim.d: $(wildcard src/*.c)
# 	$(CC) -MM $^ $(CFLAGS) > $@
# include obj/codim.d
obj/%.d: src/%.c | obj
	$(CC) $(CFLAGS) -MM -MG -MF $@ -MT $@ -MT $(patsubst src/%.c,obj/%.o,$<) $<
include $(DEPS)

obj/luagen.h: $(wildcard src/lua/*.lua) | obj
	bash src/lua/gen.bash

# obj/%.o: src/%.c $(DEPS) | obj
obj/%.o: src/%.c | obj
	$(CC) -o $@ -c $< $(CFLAGS)

$(EXEC): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LFLAGS)
