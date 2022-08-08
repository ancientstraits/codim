FFLIBS = libavcodec libavformat libavutil libswresample libswscale # freetype2 luajit libavfilter fontconfig
CFLAGS = -g -ggdb -Iinclude -Wall $(shell pkg-config --cflags $(FFLIBS))
LFLAGS = $(shell pkg-config --libs $(FFLIBS)) -lm -lGL -lEGL -lepoxy
OBJS := $(patsubst  src/%.c, obj/%.o, $(wildcard src/*.c))
DEPS := $(wildcard include/*.h)
CC = cc
EXEC = main

all: $(EXEC)

obj:
	[ -d obj ] || mkdir obj

obj/%.o: src/%.c $(DEPS) | obj
	$(CC) -o $@ -c $< $(CFLAGS)

$(EXEC): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LFLAGS)
