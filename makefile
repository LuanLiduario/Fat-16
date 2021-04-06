all: main

main: tpmain.c fat.o 
	gcc -O3 fat.o  tpmain.c -o main  -w

fat.o: fat.c tp.h
	gcc -O3 -c  fat.c  -w

clear:
	rm *.o