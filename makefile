# compiler options
CC = gcc
CCFLAGS = -Wall -fsanitize=address -fsanitize=undefined -fsanitize=leak

# shared files (API)
COMMON = clientAPI.o labyrinthAPI.o

# default rule: build the program
all: Labyrinth

# header dependencies
clientAPI.o: clientAPI.h
labyrinthAPI.o: clientAPI.h labyrinthAPI.h
Labyrinth.o: labyrinthAPI.h Labyrinth.h ZabZabIA.h
ZabZabIA.o: labyrinthAPI.h Labyrinth.h ZabZabIA.h

# generic compile rule
%.o: %.c
	$(CC) $(CCFLAGS) -o $@ -c $<

# the program: main + PlayMove (Labyrinth.c) + alpha-beta (ZabZabIA.c)
Labyrinth: $(COMMON) Labyrinth.o ZabZabIA.o
	$(CC) $(CCFLAGS) -o $@ $^

# extra rules
clean:
	rm -f *.o
mproper:
	rm -f Labyrinth *.o
