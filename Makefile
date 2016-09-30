CC=	gcc
COPT=	-Wall

all: orth gorth

orth: orth.c
	$(CC) $(COPT) orth.c -o orth

gorth: orth.c
	$(CC) -DDEBUGGER $(COPT) orth.c -o gorth

clean:
	rm -f orth gorth *.bak *~ core a.out *.o

dist: clean
	cd .. ; tar czvf orthagonal.tar.gz orthagonal/
