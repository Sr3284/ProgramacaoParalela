rotSequencial: rotseq.o
	gcc rotseq.o -g -o rotSequencial -Wall -pedantic
rotseq.o: rotseq.c
	gcc -c rotseq.c -g

rotParalelo: rotpar.o
	gcc rotpar.o -fopenmp -g -o rotParalelo -Wall -pedantic
rotpar.o: rotpar.c
	gcc -c rotpar.c -g

clean:
	rm *.o