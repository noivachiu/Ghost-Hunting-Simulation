# Reused from A4 with modifications

# C compilers and flags
HOST_CC = gcc
CFLAGS = -Wall -Wextra -g

# Stores object files
OBJ = main.o house.o ghost.o hunter.o room.o evidence.o path.o helpers.o

# Links object files and creates the executable file (will need to include threads library later)
all: main.o house.o ghost.o hunter.o room.o evidence.o path.o helpers.o
	$(HOST_CC) $(CFLAGS) -o project $(OBJ) -lpthread

# Compiles and creates object files

main.o: main.c defs.h helpers.h
	$(HOST_CC) $(CFLAGS) -c main.c

house.o: house.c defs.h helpers.h
	$(HOST_CC) $(CFLAGS) -c house.c

ghost.o: ghost.c defs.h helpers.h
	$(HOST_CC) $(CFLAGS) -c ghost.c

hunter.o: hunter.c defs.h helpers.h
	$(HOST_CC) $(CFLAGS) -c hunter.c

room.o: room.c defs.h helpers.h
	$(HOST_CC) $(CFLAGS) -c room.c

evidence.o: evidence.c defs.h helpers.h
	$(HOST_CC) $(CFLAGS) -c evidence.c

path.o: path.c defs.h 
	$(HOST_CC) $(CFLAGS) -c path.c

helpers.o: helpers.c helpers.h
	$(HOST_CC) $(CFLAGS) -c helpers.c

# Cleans up object files, log files, and the executable file
clean:
	rm -f *.o project log_*.csv