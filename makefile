CC = gcc

all: stnc

stnc: partB.o stnc.o
	gcc -o stnc stnc.o partB.o
test: test.o partB.o
	gcc -o test test.o partB.o

%.o:%.c
	$(CC)  -c $^ -o $@	

clean:
	rm -f stnc test *.o *.txt