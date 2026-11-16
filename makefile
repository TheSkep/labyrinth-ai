# compiler options; `make debug` adds the sanitizers
CC = gcc
CCFLAGS = -Wall -O2
DEBUGFLAGS = -g -fsanitize=address -fsanitize=undefined -fsanitize=leak

# shared files (API)
COMMON = clientAPI.o labyrinthAPI.o

# default rule: build the program
all: Labyrinth

# same program with the sanitizers, for testing (run `make mrproper` first)
debug: CCFLAGS += $(DEBUGFLAGS)
debug: Labyrinth

# header dependencies
clientAPI.o: clientAPI.h
labyrinthAPI.o: clientAPI.h labyrinthAPI.h
Labyrinth.o: labyrinthAPI.h Labyrinth.h ZabZabAI.h
ZabZabAI.o: labyrinthAPI.h Labyrinth.h ZabZabAI.h

# generic compile rule
%.o: %.c
	$(CC) $(CCFLAGS) -o $@ -c $<

# the program: main + PlayMove (Labyrinth.c) + alpha-beta (ZabZabAI.c)
Labyrinth: $(COMMON) Labyrinth.o ZabZabAI.o
	$(CC) $(CCFLAGS) -o $@ $^

# extra rules
.PHONY: all debug clean mrproper
clean:
	rm -f *.o
mrproper:
	rm -f Labyrinth *.o
