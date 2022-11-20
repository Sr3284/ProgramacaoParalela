// ----------------------------------------------------------------------------
// Remove 0s de um vetor
// Davi Queiroz Rodrigues e Rodrigo Seiti Koga Kikuta
// Para compilar: mpicc remove0_par.c -o remove0_par -Wall
// Para executar: mpirun -oversubscribe -np 10 remove0_par <entrada> <saída>

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>
#define WORLD MPI_COMM_WORLD

// ----------------------------------------------------------------------------

int *aloca_vetor(int n); 
void remove0(int n, int *vIn, int *m, int *vOut);

// ----------------------------------------------------------------------------
int main(int argc, char **argv){

	int n,			// Número de elementos do vetor de entrada (com 0s)
		 m,			// Número de elementos do vetor de saída (sem 0s)
		 *vIn,		// Vetor de entrada de n elementos
		 *vOut,		// Vetor de saída de no máximo n elementos
		 i;
	int numProc, idProc, nP;

	MPI_Init(&argc, &argv);

	MPI_Comm_size(WORLD, &numProc); // Get number of processes
    MPI_Comm_rank(WORLD, &idProc); // Get own ID

	if(idProc == 0){// Processo Pai
	
		FILE *arqIn,	// Arquivo texto de entrada
			 *arqOut;	// Arquivo texto de saída
	
		// -------------------------------------------------------------------------
		// Inicialização

		// Abre arquivo de entrada
		arqIn = fopen(argv[1], "rt") ;

		if (arqIn == NULL)
		{
			printf("\nArquivo texto de entrada não encontrado\n") ;
			exit(1) ;
		}

		// Lê tamanho do vetor de entrada
		fscanf(arqIn, "%d", &n) ;

		// Aloca vetores de entrada e saída
		
		vIn  = aloca_vetor(n);
		vOut = aloca_vetor(n);

		// Lê vetor do arquivo de entrada
		for (i = 0; i < n; i++){

			fscanf(arqIn, "%d", &(vIn[i]));
		}

		// Fecha arquivo de entrada
		fclose(arqIn) ;

		// -------------------------------------------------------------------------
		// Corpo principal do programa

		// Mede instante de tempo inicial

	    nP = n / numProc; // numero de elementos em cada processo
	    double tinit= MPI_Wtime();

	    MPI_Bcast(&nP, 1, MPI_INT, idProc, WORLD);
	    MPI_Scatter(vIn, nP, MPI_INT, MPI_IN_PLACE, nP, MPI_INT, idProc, WORLD);
		// Remove 0s do vetor de entrada, produzindo vetor de saída
		remove0(nP, vIn, &m, vOut);

		int *vTam, *vPos;
		vTam = aloca_vetor(numProc);
		vPos = aloca_vetor(numProc);

		MPI_Gather(&m, 1, MPI_INT, vTam , 1, MPI_INT, 0, WORLD);

		vPos[0] = 0;

		for (int i = 1; i < numProc; ++i){
			// printf("Debuger %d  proc %d 1\n", i, idProc);
			vPos[i] = vPos[i-1] + vTam[i-1];
			// printf("Debuger %d  proc %d 2\n", i, idProc);
			//printf("m - %d, i = %d, vTam = %d", m, i, vTam[i]);
			m += vTam[i];
			// printf("Debuger %d  proc %d 3\n", i, idProc);
		}
		MPI_Gatherv(NULL, 0, MPI_INT, vOut, vTam, vPos, MPI_INT, idProc, WORLD);

		// Mede instante de tempo final
		double tfim = MPI_Wtime();
		// Tempo de execução em milissegundos
		printf("Tempo=%.2lfms\n", (tfim-tinit) * 1000);
		// -------------------------------------------------------------------------
		// Finalização

		// Cria arquivo de saída
		arqOut = fopen(argv[2], "wt") ;

		// Escreve tamanho do vetor de saída
		fprintf(arqOut, "%d\n", m) ;

		// Escreve vetor do arquivo de saída
		for (i = 0; i < m; i++)
		{
			fprintf(arqOut, "%d ", vOut[i]);
		}
		fprintf(arqOut, "\n");

		// Fecha arquivo de saída
		fclose(arqOut) ;

		// Libera vetores de entrada e saída
		free(vTam);
		free(vPos);
		free(vIn);		
		free(vOut);

	}
	else{

		MPI_Bcast(&nP, 1, MPI_INT, 0, WORLD);
		vIn  = aloca_vetor(nP);
		vOut = aloca_vetor(nP);

	    MPI_Scatter(NULL, nP, MPI_INT, vIn, nP, MPI_INT, 0, WORLD);

	    remove0(nP, vIn, &m, vOut);

	    MPI_Gather(&m, 1, MPI_INT, NULL , nP, MPI_INT, 0, WORLD);

		MPI_Gatherv(vOut, m, MPI_INT, NULL, NULL, NULL, MPI_INT, 0, WORLD);

		free(vIn);		
		free(vOut);
	}

	

	MPI_Finalize();
	// printf("Debuger\n");

	return 0;
}
// ----------------------------------------------------------------------------
 
int *aloca_vetor(int n){
	int *seq;

	seq = (int *) malloc((n) * sizeof(int));
	if (seq == NULL)
	{
		printf("\nErro na alocação de estruturas\n") ;
		exit(1) ;
	}
	return seq;
}
void remove0(int n, int *vIn, int *m, int *vOut){
	int i,
		 c = 0;

	for (i = 0; i < n; i++)
	{
		if (vIn[i] != 0)
		{
			vOut[c] = vIn[i];
			c++;
		}
	}
	*m = c;
}