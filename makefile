# options de compilation
CC = gcc
CCFLAGS = -Wall -fsanitize=address -fsanitize=undefined -fsanitize=leak

# fichiers communs (API)
COMMON = clientAPI.o labyrinthAPI.o

# regle initiale : on construit le programme
all: Labyrinth

# dependance des .h
clientAPI.o: clientAPI.h
labyrinthAPI.o: clientAPI.h labyrinthAPI.h
Labyrinth.o: labyrinthAPI.h Labyrinth.h ZabZabIA.h
ZabZabIA.o: labyrinthAPI.h Labyrinth.h ZabZabIA.h

# regle generique de compilation
%.o: %.c
	$(CC) $(CCFLAGS) -o $@ -c $<

# le programme : main + PlayMove (Labyrinth.c) + alpha-beta (ZabZabIA.c)
Labyrinth: $(COMMON) Labyrinth.o ZabZabIA.o
	$(CC) $(CCFLAGS) -o $@ $^

# regles supplementaires
clean:
	rm -f *.o
mproper:
	rm -f Labyrinth *.o
