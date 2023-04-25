all: stnc test

stnc: stnc.c stnc.h
	gcc -o stnc stnc.c

test: test.o partB.o
	gcc -o test test.o partB.o

%.o:%.c
	$(CC) $(CFLAGS) -c $^ -o $@	

clean:
	rm -f stnc test *.o