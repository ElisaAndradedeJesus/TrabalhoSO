#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>
#include <stdint.h>
#include <inttypes.h>

// DEFINIÇÃO DE CONSTANTES
#define LINHAS 79056
#define COLUNAS 79056
/*  Define o tamanho da matriz 
    As dimensões devem ser positivas e dividir exatamente a Matriz sem deixar resto*/

#define SEMENTE_FIXA 67 // Define a semente fixa para o gerador de números aleatórios SIXXX SEVEENNNN
#define MAX_NUM_PRIMO 31999 // Define o limite máximo para os números primos

#define MAX_THREADS 4 // Define o número de threads
#define LINHAS_POR_BLOCO 24
#define COLUNAS_POR_BLOCO 24
#define MAX_LINHAS_POR_BLOCO  COLUNAS / COLUNAS_POR_BLOCO// Define o número máximo de linhas por thread (macrobloco)
#define MAX_COLUNAS_POR_BLOCO LINHAS / LINHAS_POR_BLOCO // Define o número máximo de colunas por thread (macrobloco)



//VARIÁVEIS GLOBAIS
int **matriz = NULL;
uint64_t totalPrimos = 0;
uint64_t controleMacroBlocos = 0;
uint64_t totalBlocos = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

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

void contarPrimosSerial() { 
    for (int i = 0; i < LINHAS; i++) {
        for (int j = 0; j < COLUNAS; j++) {
            if (ehPrimo[matriz[i][j]]) {
                totalPrimos++;
            }
        }
    }
}

void *contarPrimosThread(void *arg) {
    // Implementação da função que cada thread irá executar
    // Esta função deve contar os primos em uma parte da matriz
    // e atualizar o contador global de primos de forma segura (usando mutex, se necessário).
    uint64_t blocosPorLinha = COLUNAS / COLUNAS_POR_BLOCO;
    
    //Pega o próximo bloco a ser processado
    pthread_mutex_lock(&mutex);
    if (controleMacroBlocos >= totalBlocos) {
        pthread_mutex_unlock(&mutex);
        return NULL; // Não há mais trabalho.
    }
    uint64_t meuBloco = controleMacroBlocos;
    controleMacroBlocos++;
    // Contagem de primos na parte da matriz
    pthread_mutex_unlock(&mutex);

    uint64_t primosPorBloco = 0; // Contador local de primos para esta thread
    
    while(totalBlocos > meuBloco){
        

        uint64_t linhaInicial = (meuBloco / blocosPorLinha) * LINHAS_POR_BLOCO;
        uint64_t colunaInicial =(meuBloco % blocosPorLinha) * COLUNAS_POR_BLOCO;

        for (int i = linhaInicial;
            i < linhaInicial + LINHAS_POR_BLOCO;
            i++) {

            for (int j = colunaInicial;
                j < colunaInicial + COLUNAS_POR_BLOCO;
                j++) {

                // Usar matriz[i][j].
                if (ehPrimo[matriz[i][j]]) {
                    primosPorBloco++;
                }
            }
        }
        pthread_mutex_lock(&mutex);
        if (controleMacroBlocos >= totalBlocos) {
            pthread_mutex_unlock(&mutex);
            break; // Não há mais trabalho.
        }
        meuBloco = controleMacroBlocos;
        controleMacroBlocos++;
        // Contagem de primos na parte da matriz
        pthread_mutex_unlock(&mutex);

        pthread_mutex_lock(&mutex);
        totalPrimos += primosPorBloco; // Atualiza o contador global de primos
        pthread_mutex_unlock(&mutex);

        primosPorBloco = 0; // Reseta o contador local para a próxima iteração
    }

    return NULL;
}

void contarPrimosParalelo() {
    // Implementação da contagem paralela de números primos
    // Esta função deve ser implementada usando threads para dividir o trabalho
    // entre múltiplas threads, cada uma contando os primos em uma parte da matriz.

    pthread_t threads[MAX_THREADS];
    bool threadsCriadas[MAX_THREADS]; // Array para rastrear quais threads foram criadas com sucesso
    int threadsUsadas = 0; // Contador de threads usadas
    for (int i = 0; i < MAX_THREADS; i++) {
        int erro = pthread_create(&threads[i], NULL, contarPrimosThread, NULL);
        // Cada thread deve contar os primos em sua parte da matriz
        if(erro != 0 ){
            printf("Erro ao criar %d°thread \n", i);
            threadsCriadas[i] = false; // Marca a thread como não criada
            
        } else {
            threadsUsadas++; // Incrementa o contador de threads usadas
            threadsCriadas[i] = true; // Marca a thread como criada com sucesso
        }
    }
    for (int i = 0; i < MAX_THREADS; i++) {
        if (threadsCriadas[i]) {
            int erro = pthread_join(threads[i], NULL);
            if(erro != 0){
                printf("Erro ao esperar %d°thread \n", i);
            }

        }
    }
    printf("Total de blocos processados: %" PRIu64 "\n", controleMacroBlocos);
    printf("Total de Threads criadas: %d\n", threadsUsadas);
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
        controleMacroBlocos = 0;
        struct timespec inicioParalelo, fimParalelo, inicioSerial, fimSerial;

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
                clock_gettime(CLOCK_MONOTONIC, &inicioSerial); // Inicia a contagem do tempo  
                contarPrimosSerial();
                clock_gettime(CLOCK_MONOTONIC, &fimSerial); // Finaliza a contagem do tempo
                saida(inicioSerial, fimSerial, "CONTAGEM SERIAL");
                break;
            case 2:
                clock_gettime(CLOCK_MONOTONIC, &inicioParalelo); // Inicia a contagem do tempo  
                contarPrimosParalelo();
                clock_gettime(CLOCK_MONOTONIC, &fimParalelo); // Finaliza a contagem do tempo
                saida(inicioParalelo, fimParalelo, "CONTAGEM PARALELA");
                break;
            case 3:
                // Chamar ambas as funções
                clock_gettime(CLOCK_MONOTONIC, &inicioSerial); // Inicia a contagem do tempo
                contarPrimosSerial();
                clock_gettime(CLOCK_MONOTONIC, &fimSerial); // Finaliza a contagem do tempo
                saida(inicioSerial, fimSerial, "CONTAGEM SERIAL");

                totalPrimos = 0; // Resetar o contador de primos antes da contagem paralela

                clock_gettime(CLOCK_MONOTONIC, &inicioParalelo); // Inicia a contagem do tempo
                contarPrimosParalelo();
                clock_gettime(CLOCK_MONOTONIC, &fimParalelo); // Finaliza a contagem do tempo
                saida(inicioParalelo, fimParalelo, "CONTAGEM PARALELA");

                printf("Speedup: %.2f\n", ((fimSerial.tv_sec - inicioSerial.tv_sec) + (fimSerial.tv_nsec - inicioSerial.tv_nsec) / 1e9) /
                    ((fimParalelo.tv_sec - inicioParalelo.tv_sec) + (fimParalelo.tv_nsec - inicioParalelo.tv_nsec) / 1e9)); 
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

    /* Validação em caso de que acidentalmente caguemos o código*/
    // Impedir dimensões zero ou negativas.
    if (LINHAS <= 0 || COLUNAS <= 0 ||
        LINHAS_POR_BLOCO <= 0 || COLUNAS_POR_BLOCO <= 0) {
        printf("As dimensoes da matriz e dos blocos devem ser positivas.\n");
        return 1;
    }

    // Verificar se os blocos cabem sem sobras.
    if (LINHAS % LINHAS_POR_BLOCO != 0 ||
        COLUNAS % COLUNAS_POR_BLOCO != 0) {
        printf("Os blocos precisam dividir a matriz sem sobras.\n");
        return 1;
    }
    totalBlocos = (uint64_t)(LINHAS / LINHAS_POR_BLOCO) * (COLUNAS / COLUNAS_POR_BLOCO);
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
