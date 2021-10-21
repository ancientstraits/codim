FFLIBS = libavutil libavformat libavfilter libavcodec libswresample libswscale freetype2
CFLAGS = -g -ggdb -Iinclude -Wall $(shell pkg-config --cflags $(FFLIBS))
LFLAGS = $(shell pkg-config --libs $(FFLIBS)) -lm
OBJS := $(patsubst  src/%.c, obj/%.o, $(wildcard src/*.c))
DEPS := $(wildcard include/*.h)
EXEC = main

all: $(EXEC)

obj:
	mkdir obj

obj/%.o: src/%.c $(DEPS) | obj
	$(CC) -o $@ -c $< $(CFLAGS)

$(EXEC): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LFLAGS)
