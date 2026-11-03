#define LINHAS 5
#define COLUNAS 5

#include <stdio.h>
#include <stdlib.h>
int **alocarMatriz(int m, int n){
    int **v; /* ponteiro para a matriz */
    int i; /* variável auxiliar */
    if (m < 1 || n < 1) { /* verifica parametros recebidos */
        printf ("** Erro: Parametro invalido **\n");
        return (NULL);
    }
    /* aloca as linhas da matriz */
    v = calloc(m, sizeof(int*)); /* Um vetor de m ponteiros para int */
    if (v == NULL) {
        printf("** Erro: Memoria Insuficiente **");
        return (NULL);
    }

    /* aloca as colunas da matriz */
    for ( i = 0; i < m; i++ ) {
        v[i] = calloc(n, sizeof(int)); /* m vetores de n ints */
        if (v[i] == NULL) {
            for (int j = 0; j < i; j++) free(v[j]); /* libera as linhas alocadas */
            free(v); /* libera a matriz (vetor de ponteiros) */
            printf("** Erro: Memoria Insuficiente **");
            return (NULL);
        }
    }

    return (v); /* retorna o ponteiro para a matriz */
}

int **liberarMatriz(int **matriz, int m, int n){
    int i; /* variável auxiliar */
    if (matriz == NULL) return (NULL);
    if (m < 1 || n < 1) { /* verifica parâmetros recebidos */
        printf ("** Erro: Parametro invalido **\n");
        return (matriz);
    }

    for (i=0; i<m; i++) free (matriz[i]); /* libera as linhas da matriz */
    free (matriz); /* libera a matriz (vetor de ponteiros) */

    return (NULL);
}

int main(){
    int **matriz;
    matriz = alocarMatriz(LINHAS, COLUNAS);
    if (matriz == NULL) {
        printf("Erro ao alocar a matriz.\n");
        return 1;
    }



    matriz = liberarMatriz(matriz, LINHAS, COLUNAS);
    return 0;
}