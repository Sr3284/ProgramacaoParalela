/* ----------------------------------------------------------------------------
	Roteamento usando algoritmo de Lee

Autores: Davi Queiroz Rodrigues e Rodrigo Seiti Koga Kikuta

Para compilar: gcc rotpar.c -fopenmp -o rotParalelo -Wall -pedantic
			   make rotParalelo

Para executar: ./rotParalelo <nome arquivo entrada> <nome arquivo saída>
---------------------------------------------------------------------------- */

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>

// ----------------------------------------------------------------------------
// Tipos

typedef struct {    	//posição da celula no grid
	int i, j;
} t_celula;

typedef struct no { 	//nó da fila de celulas
	int i, j;
	struct no *prox;
} t_no;

// ----------------------------------------------------------------------------
// Variáveis globais

int nLinhas,  		// No. de linhas do grid
	nColunas, 		// No. de colunas do grid
	**dist,   		// Matriz com distância da origem até cada célula do grid
	dist_min;		// Distância do caminho mínimo de origem a destino

t_celula origem,	// Células origem e destino no grid
		 destino;

t_no *ini_fila,		// Ponteiros para início e fim da fila de células a serem tratadas (fila FIFO)
	 *fim_fila,
	 *ini_aux,
	 *fim_aux,
	 *ini_caminho;	// Ponteiro para início do caminho encontrado

int currLevel;

// ----------------------------------------------------------------------------
// Funções

void inicializacao(char* nome_arq_entrada)
{
	FILE *arq_entrada;	// Arquivo texto de entrada
	int n_obstaculos, 	// Número de obstáculos do grid
		n_linhas_obst,
		n_colunas_obst;

	t_celula obstaculo;

	arq_entrada = fopen(nome_arq_entrada, "rt");

	if (arq_entrada == NULL)
	{
		printf("\nArquivo texto de entrada não encontrado\n");
		exit(1);
	}

	fscanf(arq_entrada, "%d %d", &nLinhas, &nColunas);
	fscanf(arq_entrada, "%d %d", &origem.i, &origem.j);
	fscanf(arq_entrada, "%d %d", &destino.i, &destino.j);
	fscanf(arq_entrada, "%d", &n_obstaculos);

	// Aloca grid
	dist = malloc(nLinhas * sizeof (int*));
	for (int i = 0; i < nLinhas; i++)
		dist[i] = malloc(nColunas * sizeof (int));
	// Checar se conseguiu alocar

	// Inicializa grid
	for (int i = 0; i < nLinhas; i++)
		for (int j = 0; j < nColunas; j++)
			dist[i][j] = INT_MAX;

	dist[origem.i][origem.j] = 0; // Distância da origem até ela mesma é 0

	// Lê obstáculos do arquivo de entrada e preenche grid
	for (int k = 0; k < n_obstaculos; k++)
	{
		fscanf(arq_entrada, "%d %d %d %d", &obstaculo.i, &obstaculo.j, &n_linhas_obst, &n_colunas_obst);

		for (int i = obstaculo.i; i < obstaculo.i + n_linhas_obst; i++)
			for (int j = obstaculo.j; j < obstaculo.j + n_colunas_obst; j++)
				dist[i][j] = -1;
	}

	fclose(arq_entrada);

	// Inicializa fila vazia
	ini_fila = NULL;
	fim_fila = NULL;
	ini_aux = NULL;
	fim_aux = NULL;

	// Inicializa caminho vazio
	ini_caminho = NULL;
}

// ----------------------------------------------------------------------------

void finalizacao(char* nome_arq_saida)
{
	FILE *arq_saida;	// Arquivo texto de saída
	t_no *no;

	arq_saida = fopen(nome_arq_saida, "wt");

	// Imprime distância mínima no arquivo de saída
	fprintf(arq_saida, "%d\n", dist_min);

	// Imprime caminho mínimo no arquivo de saída
	while (ini_caminho != NULL)
	{
		fprintf(arq_saida, "%d %d\n", ini_caminho->i, ini_caminho->j);

		no = ini_caminho;
		ini_caminho = ini_caminho->prox;

		// Libera nó do caminho
		free(no);
	}

	fclose(arq_saida);

	// Libera grid
	for (int i = 0; i < nLinhas; i++)
		free(dist[i]);
	free(dist);
}

// ----------------------------------------------------------------------------
// Insere célula no fim da fila de células a serem tratadas (fila FIFO)

void insere_fila(t_celula celula, int num)
{
	t_no *no = malloc(sizeof(t_no));
	// Checar se conseguiu alocar

	no->i = celula.i;
	no->j = celula.j;
	no->prox = NULL;

	if(num == 0)
	{
		if (ini_fila == NULL)
			ini_fila = no;
		else
			fim_fila->prox = no;
		fim_fila = no;
	}
	else if(num == 1)
	{
		if(ini_aux == NULL)
			ini_aux = no;
		else
			fim_aux->prox = no;
		fim_aux = no;
	}
}

// ----------------------------------------------------------------------------
// Remove célula do início da fila de células a serem tratadas (fila FIFO)

t_celula remove_fila()
{
	t_celula celula;
	t_no *no;

	no = ini_fila;

	celula.i = no->i;
	celula.j = no->j;

	ini_fila = no->prox;

	if (ini_fila == NULL)
		fim_fila = NULL;
	
	free(no);

	return celula;
}
void remove_aux()
{
	t_no *no;

	no = ini_fila;

	if (ini_fila == NULL)
		fim_fila = NULL;

	free(no);
}

// ----------------------------------------------------------------------------
// Insere célula no inicio do caminho

void insere_caminho(t_celula celula)
{
	t_no *no = malloc(sizeof(t_no));
	// Checar se conseguiu alocar

	no->i = celula.i;
	no->j = celula.j;
	no->prox = ini_caminho;

	ini_caminho = no;
}

// ----------------------------------------------------------------------------

bool expand() {

	bool achou = false;
	t_celula atual, vizinho;

	insere_fila(origem, 0);

	while (ini_fila != NULL && !achou) {

		#pragma omp parallel num_threads(4) 
		{
			#pragma omp master 
			{
				atual = remove_fila();
			}

			if (atual.i == destino.i && atual.j == destino.j)
				achou = true;
			else {

				#pragma omp sections 
				{
					#pragma omp section
						if (((atual.i - 1) >= 0) && (dist[atual.i - 1][atual.j] == INT_MAX)) {
							#pragma omp critical 
							{
								vizinho.i = atual.i - 1;
								vizinho.j = atual.j;

								dist[vizinho.i][vizinho.j] = dist[atual.i][atual.j] + 1;
								insere_fila(vizinho);
							}
						}
					#pragma omp section
						if (((atual.i + 1) < nLinhas) && (dist[atual.i + 1][atual.j] == INT_MAX)) {
							#pragma omp critical 
							{
								vizinho.i = atual.i + 1;
								vizinho.j = atual.j;

								dist[vizinho.i][vizinho.j] = dist[atual.i][atual.j] + 1;
								insere_fila(vizinho);
							}
						}
					#pragma omp section
						if (((atual.j - 1) >= 0) && (dist[atual.i][atual.j - 1] == INT_MAX)) {
							#pragma omp critical 
							{
								vizinho.i = atual.i;
								vizinho.j = atual.j - 1;

								dist[vizinho.i][vizinho.j] = dist[atual.i][atual.j] + 1;
								insere_fila(vizinho);
							}
						}
					#pragma omp section
						if (((atual.j + 1) < nColunas) && (dist[atual.i][atual.j + 1] == INT_MAX)) {
							#pragma omp critical 
							{
								vizinho.i = atual.i;
								vizinho.j = atual.j + 1;

								dist[vizinho.i][vizinho.j] = dist[atual.i][atual.j] + 1;
								insere_fila(vizinho);
							}
						}
				}
			}
		}

		//insere_fila(fim_fila, remove_fila(aux))
	}

}

// ----------------------------------------------------------------------------

void traceback() {

	t_celula celula, vizinho ;

	// Constrói caminho mínimo, com células do destino até a origem
	
	// Inicia caminho com célula destino
	insere_caminho(destino) ;

	celula.i = destino.i ;
	celula.j = destino.j ;

	// Enquanto não chegou na origem
	while (celula.i != origem.i || celula.j != origem.j)
	{
		// Determina se célula anterior no caminho é vizinho norte, sul, oeste ou leste
		// e insere esse vizinho no início do caminho
		
		vizinho.i = celula.i - 1 ; // Norte
		vizinho.j = celula.j ;

		if ((vizinho.i >= 0) && (dist[vizinho.i][vizinho.j] == dist[celula.i][celula.j] - 1))
			insere_caminho(vizinho) ;
		else
		{
			vizinho.i = celula.i + 1 ; // Sul
			vizinho.j = celula.j ;

			if ((vizinho.i <nLinhas) && (dist[vizinho.i][vizinho.j] == dist[celula.i][celula.j] - 1))
				insere_caminho(vizinho) ;
			else
			{
				vizinho.i = celula.i ; // Oeste
				vizinho.j = celula.j - 1 ;

				if ((vizinho.j >= 0) && (dist[vizinho.i][vizinho.j] == dist[celula.i][celula.j] - 1))
					insere_caminho(vizinho) ;
				else
				{
					vizinho.i = celula.i ; // Leste
					vizinho.j = celula.j + 1 ;

					if ((vizinho.j < nColunas) && (dist[vizinho.i][vizinho.j] == dist[celula.i][celula.j] - 1))
						insere_caminho(vizinho) ;
				}
			}
		}
		celula.i = vizinho.i ;
		celula.j = vizinho.j ;
	}
}

// ----------------------------------------------------------------------------
// Programa principal

int main(int argc, char** argv)
{
	char nome_arq_entrada[100],
		  nome_arq_saida[100] ;
	bool achou ;

	if(argc != 3)
	{
		printf("O programa foi executado com argumentos incorretos.\n") ;
		printf("Uso: ./rotParalelo <nome arquivo entrada> <nome arquivo saída>\n") ;
		exit(1) ;
	}

	// Obtém nome dos arquivos de entrada e saída
	strcpy(nome_arq_entrada, argv[1]) ;
	strcpy(nome_arq_saida, argv[2]) ;

	// Lê arquivo de entrada e inicializa estruturas de dados
	inicializacao (nome_arq_entrada) ;

	// Fase de expansão: calcula distância da origem até demais células do grid
	achou = expand();

	// Se não encontrou caminho de origem até destino
	if (! achou)
		dist_min = -1 ;
	else
	{
		// Obtém distância do caminho mínimo da origem até destino
		dist_min = dist[destino.i][destino.j] ;

		// Fase de traceback: obtém caminho mínimo
		traceback();
	}

	// Finaliza e escreve arquivo de saida
	finalizacao(nome_arq_saida) ;
	
	return 0 ;
}
