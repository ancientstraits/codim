FFLIBS = libavutil libavformat libavfilter libavcodec libswresample libswscale freetype2 lua5.3 fontconfig
CFLAGS = -g -ggdb -Iinclude -Wall $(shell pkg-config --cflags $(FFLIBS))
LFLAGS = $(shell pkg-config --libs $(FFLIBS)) -lm
OBJS := $(patsubst  src/%.c, obj/%.o, $(wildcard src/*.c))
DEPS := $(wildcard include/*.h)
EXEC = codim

all: $(EXEC)

obj:
	[ -d obj ] ||  mkdir obj

obj/%.o: src/%.c $(DEPS) | obj
	$(CC) -o $@ -c $< $(CFLAGS)

$(EXEC): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LFLAGS)

.PHONY: clean install
clean: $(EXEC)
	rm -rf $(OBJS)
install: $(EXEC)
	install -m255 $(EXEC) /usr/local/bin/$(EXEC)

