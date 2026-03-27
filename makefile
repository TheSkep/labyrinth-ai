# options de compilation
CC = gcc
CCFLAGS = -Wall -fsanitize=address -fsanitize=undefined -fsanitize=leak

# fichiers du projet
SRC = clientAPI.c labyrinthAPI.c Labyrinth.c
OBJ = $(SRC:.c=.o)
EXEC = Labyrinth


# règle initiale
all: $(EXEC)

# dépendance des .h
clientAPI.o: clientAPI.h 
labyrinthAPI.o: clientAPI.h labyrinthAPI.h
Labyrinth.o: labyrinthAPI.h Labyrinth.h

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