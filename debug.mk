CC = gcc
CFLAGS = -Wall -g -Wpedantic -lm -DDEBUG
LDFLAGS =
LBLIBS = -lpthread 
SRC =  $(wildcard *.c)
OBJ = $(SRC:.c=.o)
EXEC = server
OUTDIR = bin/debug

all: $(EXEC)
	rm -rf ./$(OUTDIR)
	mkdir -p ./$(OUTDIR)
	mv $(EXEC) ./$(OUTDIR)/$(EXEC)
	rm -f $(OBJ) 

$(EXEC): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $(OBJ) $(LBLIBS)

clean:
	rm -f $(OBJ) 
