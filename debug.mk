CC = gcc
CFLAGS = -Wall -g -Wpedantic -lm -DDEBUG
LDFLAGS =
LBLIBS = -lpthread 
SRC =  $(wildcard *.c)
OBJ = $(SRC:.c=.o)
EXEC = server

all: $(EXEC)
	rm -f $(OBJ) 

$(EXEC): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $(OBJ) $(LBLIBS)

clean:
	rm -f $(OBJ) 
