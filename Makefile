OS = $(shell uname)

LIBS = libavcodec libavformat libavutil libswresample libswscale epoxy freetype2 cglm luajit
ifneq ($(OS),Linux)
	LIBS += glfw3
endif

CFLAGS = -g -ggdb -Iinclude -Wall $(shell pkg-config --cflags $(LIBS))
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

OBJS := $(patsubst  src/%.c, obj/%.o, $(wildcard src/*.c))
DEPS := $(wildcard include/*.h)
CC = clang
EXEC = main

all: $(EXEC)

obj:
	[ -d obj ] || mkdir obj

obj/%.o: src/%.c $(DEPS) | obj
	$(CC) -o $@ -c $< $(CFLAGS)

$(EXEC): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LFLAGS)
