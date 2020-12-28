OBJ = edit.o map.o txt.o term.o

all: seqt

seqt: seqt.c $(OBJ)
	$(CC) $(CFLAGS) -o $@ seqt.c $(OBJ)

$(OBJ): seqt.h

clean:
	$(RM) seqt $(OBJ)
