# options de compilation
CC = gcc
CCFLAGS = -Wall -fsanitize=address -fsanitize=undefined -fsanitize=leak

# fichiers du projet
SRC = clientAPI.c labyrinthAPI.c Labyrinthe.c
OBJ = $(SRC:.c=.o)
EXEC = Labyrinthe


# règle initiale
all: $(EXEC)

# dépendance des .h
clientAPI.o: clientAPI.h 
labyrinthAPI.o: clientAPI.h labyrinthAPI.h
Labyrinthe.o: labyrinthAPI.h

# règles de compilation
%.o: %.c
	$(CC) $(CCFLAGS) -o $@ -c $<
	
# règles d'édition de liens
$(EXEC): $(OBJ)
	$(CC) $(CCFLAGS) -o $@ $^ $(LIBS) $(LIBSDIR)

# règles supplémentaires
clean:
	rm -f *.o
mproper:
	rm -f $(EXEC) *.o