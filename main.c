#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

// DEFINIÇÃO DE CONSTANTES
#define LINHAS 79056
#define COLUNAS 79056
#define SEMENTE_FIXA 67 // Define a semente fixa para o gerador de números aleatórios
#define MAX_NUM_PRIMO 31999 // Define o limite máximo para os números primos
#define MAX_THREADS 4 // Define o número máximo de threads

//VARIÁVEIS GLOBAIS
int **matriz = NULL;
uint64_t totalPrimos = 0;
int controleMacroBlocos = 0;
bool ehPrimo[MAX_NUM_PRIMO + 1]; // Tabela de consulta para números primos

/* Visou-se construir o algoritmo mais eficiente possível para verificar 
   se um número é primo. Como o limite máximo do valor dos elementos é conhecido, 
   escolheu-se combinar o método do Crivo de Eratóstenes com uma tabela de consulta. 

   O Crivo de Eratóstenes é usado para construir a tabela de consulta 
   que permite descobrir imediatamente se um número é primo ou não, 
   apresentando o melhor caso sempre O(1) (para qualquer caso). */

void precomputarPrimos() {
    // Inicializa todos como verdadeiros, como se todos os números fossem primos.
    for (int i = 0; i <= MAX_NUM_PRIMO ; i++) {
        ehPrimo[i] = true; // Assume que todos os números são primos inicialmente
    }

    ehPrimo[0] = false;
    ehPrimo[1] = false;

    // Implementa o Crivo de Eratóstenes para marcar os números não primos
    for (int p = 2; p * p <= MAX_NUM_PRIMO; p++) {
        if (ehPrimo[p]) {
            for (int i = p * p; i <= MAX_NUM_PRIMO; i += p) {
                ehPrimo[i] = false; // Marca múltiplos de p como não primos
            }
        }
    }
}

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

void preencherMatriz(int **matriz){
    if (matriz == NULL) return;
    for (int i = 0; i < LINHAS; i++) {
        for (int j = 0; j < COLUNAS; j++) {
            matriz[i][j] = rand() % 32000; // Preenche a matriz com números aleatórios de 0 a 31999
        }
    }
}

void imprimirMatriz(int **matriz){
    if (matriz == NULL) return;
    for (int i = 0; i < LINHAS; i++) {
        for (int j = 0; j < COLUNAS; j++) {
            printf("%5d ", matriz[i][j]);
        }
        printf("\n");
    }
}

void contarPrimosSerial(int **matriz) { 
    for (int i = 0; i < LINHAS; i++) {
        for (int j = 0; j < COLUNAS; j++) {
            if (ehPrimo[matriz[i][j]]) {
                totalPrimos++;
            }
        }
    }
}
typedef struct {
    int **matriz;
    int inicio;
    int fim;
    uint64_t total;
} TrabalhoThread;

void *contarFaixa(void *argumento) {
    TrabalhoThread *trabalho = argumento;
    uint64_t total = 0;
    for (int i = trabalho->inicio; i < trabalho->fim; i++) {
        for (int j = 0; j < COLUNAS; j++) {
            if (ehPrimo[trabalho->matriz[i][j]]) total++;
        }
    }
    trabalho->total = total;
    return NULL;
}

void contarPrimosParalelo(int **matriz) {
    long processadores = sysconf(_SC_NPROCESSORS_ONLN);
    int quantidade = processadores > 0 ?
        (processadores > LINHAS ? LINHAS : (int)processadores) : 1;
    pthread_t *threads = calloc(quantidade, sizeof(*threads));
    TrabalhoThread *trabalhos = calloc(quantidade, sizeof(*trabalhos));
    if (threads == NULL || trabalhos == NULL) {
        free(threads);
        free(trabalhos);
        fprintf(stderr, "Sem memoria para as threads; usando contagem serial.\n");
        contarPrimosSerial(matriz);
        return;
    }

    int criadas = 0;
    for (int t = 0; t < quantidade; t++) {
        trabalhos[t] = (TrabalhoThread) {
            .matriz = matriz,
            .inicio = (int)((int64_t)t * LINHAS / quantidade),
            .fim = (int)((int64_t)(t + 1) * LINHAS / quantidade),
            .total = 0
        };
        int erro = pthread_create(&threads[t], NULL, contarFaixa, &trabalhos[t]);
        if (erro != 0) {
            fprintf(stderr, "Falha ao criar thread: %s. Restante na thread principal.\n",
                    strerror(erro));
            trabalhos[t].fim = LINHAS;
            contarFaixa(&trabalhos[t]);
            totalPrimos += trabalhos[t].total;
            break;
        }
        criadas++;
    }
    for (int t = 0; t < criadas; t++) {
        int erro = pthread_join(threads[t], NULL);
        if (erro != 0) {
            fprintf(stderr, "Falha ao aguardar thread: %s\n", strerror(erro));
            exit(EXIT_FAILURE);
        }
        totalPrimos += trabalhos[t].total;
    }
    free(threads);
    free(trabalhos);
}

void saida(struct timespec inicio, struct timespec fim, const char* metodo) {
    double tempo = (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1e9;
    printf("%s - Total de números primos: %" PRIu64 "\n", metodo, totalPrimos);
    printf("Tempo de execução: %.6f segundos\n", tempo);
}
void menu(int** matriz){
    int opcao = -1;
    while (opcao!= 0){
        totalPrimos = 0; // Resetar o contador de primos antes de cada contagem
        struct timespec inicio, fim;

        printf("\n1 - Contagem serial\n");
        printf("2 - Contagem paralela\n");
        printf("3 - Ambas\n");
        printf("0 - Sair\n");
        printf("Escolha: ");

        if (scanf("%d", &opcao) != 1) {
            printf("Entrada invalida. Digite um numero.\n");
            int caractere;

            while ((caractere = getchar()) != '\n' && caractere != EOF) {
                // Descarta a entrada invalida.
            }
            if (caractere == EOF) {
                break; // Encerra o menu se a entrada terminar.
            }
            continue; // Volta ao inicio do while do menu.
        }
        
        switch (opcao){
            case 1:
                // Chamar função de contagem serial
                clock_gettime(CLOCK_MONOTONIC, &inicio); // Inicia a contagem do tempo  
                contarPrimosSerial(matriz);
                clock_gettime(CLOCK_MONOTONIC, &fim); // Finaliza a contagem do tempo
                saida(inicio, fim, "CONTAGEM SERIAL");
                break;
            case 2:
                clock_gettime(CLOCK_MONOTONIC, &inicio);
                contarPrimosParalelo(matriz);
                clock_gettime(CLOCK_MONOTONIC, &fim);
                saida(inicio, fim, "CONTAGEM PARALELA");
                break;
            case 3:
                clock_gettime(CLOCK_MONOTONIC, &inicio);
                contarPrimosSerial(matriz);
                clock_gettime(CLOCK_MONOTONIC, &fim);
                saida(inicio, fim, "CONTAGEM SERIAL");
                uint64_t totalSerial = totalPrimos;
                totalPrimos = 0;
                clock_gettime(CLOCK_MONOTONIC, &inicio);
                contarPrimosParalelo(matriz);
                clock_gettime(CLOCK_MONOTONIC, &fim);
                saida(inicio, fim, "CONTAGEM PARALELA");
                printf("Resultados %s.\n", totalSerial == totalPrimos ? "iguais" : "diferentes");
                break;
            case 0:
                printf("Saindo...\n");
                break;
            default:
                printf("Opção inválida. Tente novamente.\n");
        }
    }
}



int main(){
    // Inicializando tabela de consulta de números primos
    precomputarPrimos();
    srand(SEMENTE_FIXA); // Inicializa o gerador de números aleatórios com a semente fixa

    printf("Gerando matriz de %d x %d...\n", LINHAS, COLUNAS);
    struct timespec inicio, fim;
    clock_gettime(CLOCK_MONOTONIC, &inicio); // Inicia a contagem do tempo para gerar a matriz

    matriz = alocarMatriz(LINHAS, COLUNAS);
    if (matriz == NULL) {
        printf("Erro ao alocar a matriz.\n");
        return 1;
    }
    
    preencherMatriz(matriz);
    // imprimirMatriz(matriz);
    clock_gettime(CLOCK_MONOTONIC, &fim); // Finaliza a contagem do tempo para gerar a matriz
    printf("Matriz gerada em %.6f segundos.\n", (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1e9);

    menu(matriz); // Chama o menu para interação com o usuário

    matriz = liberarMatriz(matriz, LINHAS, COLUNAS);
    return 0;
}
