/* ----------------------------------------------------------------------------
	Roteamento usando algoritmo de Lee

Autores: Davi Queiroz Rodrigues e Rodrigo Seiti Koga

Para compilar: gcc rotpar.c -o rotParalelo -Wall -pedantic
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
	 *ini_caminho;	// Ponteiro para início do caminho encontrado

int currLevel;

// ----------------------------------------------------------------------------
// Funções

void inicializacao(char* arq) {}

void finalizacao(char* arq) {}

bool expand() {}

void traceback() {}