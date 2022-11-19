/* ------------------------------------------------------------------------------ // 
//  Remove 0s de um vetor (MPI)                                                   //
//                                                                                //
//     Davi Queiroz Rodrigues e Rodrigo Seiti Koga Kikuta                         //
//                                                                                //
//     Para compilar: mpicc remove0_par.c -o remove0_par -Wall                    //
//     Para executar: mpirun -oversubscribe -np 10 remove0_par <entrada> <saída>  //
//                                                                                //
// ------------------------------------------------------------------------------ */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

int main(int argc, char **argv) {

	int pid, np,
		n, m,
		*vIn, *vOut;

	int i;

	FILE *in *out;

	if (argc != 3) {
		printf("O programa foi executado com argumentos incorretos.\n");
		printf("Uso: ./remove0_par <entrada> <saida>\n");
		exit(1);
	}

	in = fopen(argv[1], "rt");

	if (in == NULL) {
		printf("\nArquivo texto de entrada não encontrado\n");
		exit(1);
	}

	fscanf(in, "%d", &n);

	vIn = (int *) malloc(n * sizeof(int));
	vOut = (int *) malloc(n * sizeof(int));

	if ((vIn == NULL) || (vOut == NULL)) {
		printf("\nErro na alocação de estruturas\n");
	}

	for (i = 0; i < n; i++) {
		fscanf(in, "%d", &(vIn[i]));
	}

	// Inicialização do ambiente MPI
	MPI_Init(&argc, &argv);
	// Id do processo
	MPI_Comm_rank(MPI_COMM_WORLD, &pid);
	// Num. de processos
	MPI_Comm_size(MPI_COMM_WORLD, &np);



	// Finalização
	MPI_Finalize();

	return 0;
}