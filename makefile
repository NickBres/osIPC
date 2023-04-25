all: stnc

stnc: stnc.c stnc.h
	gcc -o stnc stnc.c

clean:
	rm -f stnc *.o