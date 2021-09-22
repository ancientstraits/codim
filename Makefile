CFLAGS = -g -ggdb -Iinclude -Wall
LFLAGS = 
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
