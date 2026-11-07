#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>

// Controle automático de biblioteca de tempo (Windows vs Linux)
#ifdef _WIN32
    #include <windows.h>
#else
    #include <time.h>
#endif

// DEFINIÇÃO DE CONSTANTES
#define LINHAS 19200
#define COLUNAS 19200
/*  Define o tamanho da matriz 
    As dimensões devem ser positivas e dividir exatamente a Matriz sem deixar resto*/

#define SEMENTE_FIXA 67 // Define a semente fixa para o gerador de números aleatórios SIXXX SEVEENNNN
#define MAX_NUM_PRIMO 31999 // Define o limite máximo para os números primos

#define MAX_THREADS 4 // Define o número de threads
#define LINHAS_POR_BLOCO 24
#define COLUNAS_POR_BLOCO 24

//VARIÁVEIS GLOBAIS
int **matriz = NULL;
uint64_t totalPrimos = 0;
uint64_t controleMacroBlocos = 0;
uint64_t totalBlocos = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// FUNÇÃO UNIVERSAL DE TEMPO
double obterTempoAtual() {
#ifdef _WIN32
    LARGE_INTEGER frequencia, contador;
    QueryPerformanceFrequency(&frequencia);
    QueryPerformanceCounter(&contador);
    return (double)contador.QuadPart / frequencia.QuadPart;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec) + (ts.tv_nsec / 1e9);
#endif
}

// FUNÇÃO PARA VERIFICAR PRIMO
int ehPrimo(int n) {
    if (n <= 1) return 0;
    if (n == 2) return 1;
    if (n % 2 == 0) return 0;
    
    // O cálculo da raiz fica fora do laço para não repetir operações desnecessárias
    int limite = (int)sqrt(n); 
    
    for (int i = 3; i <= limite; i += 2) {
        if (n % i == 0) {
            return 0; // Não é primo
        }
    }
    return 1; // É primo
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

void contarPrimosSerial() { 
    for (int i = 0; i < LINHAS; i++) {
        for (int j = 0; j < COLUNAS; j++) {
            if (ehPrimo(matriz[i][j])) {
                totalPrimos++;
            }
        }
    }
}

void *contarPrimosThread(void *arg) {
    // Implementação da função que cada thread irá executar
    // Esta função deve contar os primos em uma parte da matriz
    // e atualizar o contador global de primos de forma segura (usando mutex, se necessário).
    (void)arg;
    uint64_t blocosPorLinha = COLUNAS / COLUNAS_POR_BLOCO;
    
    while(1) {
        uint64_t meuBloco;
        //Pega o próximo bloco a ser processado
        pthread_mutex_lock(&mutex);
        if (controleMacroBlocos >= totalBlocos) {
            pthread_mutex_unlock(&mutex);
            break; // Não há mais trabalho.
        }
        meuBloco = controleMacroBlocos;
        controleMacroBlocos++; // Só incrementa se houver bloco disponível
        // Contagem de primos na parte da matriz
        pthread_mutex_unlock(&mutex);

        // Cálculo das coordenadas do bloco na matriz
        uint64_t linhaInicial = (meuBloco / blocosPorLinha) * LINHAS_POR_BLOCO;
        uint64_t colunaInicial = (meuBloco % blocosPorLinha) * COLUNAS_POR_BLOCO;

        uint64_t primosPorBloco = 0; // Contador local de primos para esta thread
    
        // Processamento local do bloco
        for (uint64_t i = linhaInicial;
            i < linhaInicial + LINHAS_POR_BLOCO;
            i++) {

            for (uint64_t j = colunaInicial;
                j < colunaInicial + COLUNAS_POR_BLOCO;
                j++) {
                if (ehPrimo(matriz[i][j])) primosPorBloco++;
            }
        }

        // Somar ao total global
        pthread_mutex_lock(&mutex);
        totalPrimos += primosPorBloco; 
        pthread_mutex_unlock(&mutex);
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
            printf("Erro ao criar %d°thread \n", i + 1);
            threadsCriadas[i] = false; // Marca a thread como não criada
            
        } else {
            threadsUsadas++; // Incrementa o contador de threads usadas
            threadsCriadas[i] = true; // Marca a thread como criada com sucesso
        }
    }
  
    if (threadsUsadas < MAX_THREADS) {
        printf(">>> AVISO: Rodada com criacao incompleta. Apenas %d de %d threads processaram.\n", threadsUsadas, MAX_THREADS);
    }

    for (int i = 0; i < MAX_THREADS; i++) {
        if (threadsCriadas[i]) {
            int erro = pthread_join(threads[i], NULL);
            if(erro != 0){
                printf("Erro ao realizar o join da %d°thread \n", i + 1);
            }

        }
    }
    printf("Total de blocos processados: %" PRIu64 " de %" PRIu64 "\n", controleMacroBlocos, totalBlocos);
    printf("Total de Threads criadas: %d\n", threadsUsadas);
}


void saida(double tempo, const char* metodo) {
    printf("%s Total de números primos: %" PRIu64 "\n", metodo, totalPrimos);
    printf("Tempo de execução: %.6f segundos\n", tempo);
}

void menu(){
    int opcao = -1;
    while (opcao!= 0){
        totalPrimos = 0; // Resetar o contador de primos antes de cada contagem
        controleMacroBlocos = 0;
        struct timespec inicioParalelo, fimParalelo, inicioSerial, fimSerial;

        printf("\n=================================");
        printf("\n1 - Contagem serial\n");
        printf("2 - Contagem paralela\n");
        printf("3 - Ambas \n");
        printf("0 - Sair\n");
        printf("Escolha: ");

        if (scanf("%d", &opcao) != 1) {
            printf("Entrada invalida. Digite um numero.\n");
            int caractere;

            while ((caractere = getchar()) != '\n' && caractere != EOF) {
                // Descarta a entrada invalida.
            }
            if (caractere == EOF) break; // Encerra o menu se a entrada terminar.
            continue; // Volta ao inicio do while do menu.
        }
        
        switch (opcao){
            case 1:
                // Chamar função de contagem serial
                totalPrimos = 0; // Resetar o contador de primos antes da contagem 
                double inicioSerial = obterTempoAtual(); // Inicia a contagem do tempo  
                contarPrimosSerial();
                double fimSerial = obterTempoAtual(); // Finaliza a contagem do tempo
                saida(fimSerial - inicioSerial, "CONTAGEM SERIAL");
                break;
            case 2:
                totalPrimos = 0; // Resetar o contador de primos antes da contagem 
                controleMacroBlocos = 0; // Reinicia o contador de blocos
                double inicioParalelo = obterTempoAtual(); // Inicia a contagem do tempo  
                contarPrimosParalelo();
                double fimParalelo = obterTempoAtual(); // Finaliza a contagem do tempo
                saida(fimParalelo - inicioParalelo, "CONTAGEM PARALELA");
                break;
            case 3:
                // Chamar ambas as funções
                totalPrimos = 0;
                inicioSerial = obterTempoAtual(); // Inicia a contagem do tempo
                contarPrimosSerial();
                fimSerial = obterTempoAtual(); // Finaliza a contagem do tempo
                saida(fimSerial - inicioSerial, "CONTAGEM SERIAL");

                totalPrimos = 0; // Resetar o contador de primos antes da contagem 
                controleMacroBlocos = 0; // Reinicia o contador de blocos
                inicioParalelo = obterTempoAtual(); // Inicia a contagem do tempo
                contarPrimosParalelo();
                fimParalelo = obterTempoAtual(); // Finaliza a contagem do tempo
                saida(fimParalelo - inicioParalelo, "CONTAGEM PARALELA");

                double tSerial = fimSerial - inicioSerial;
                double tParalelo = fimParalelo - inicioParalelo;

                if(tParalelo > 0) {
                    printf("\n>>> SPEEDUP: %.2fx <<<\n", tSerial / tParalelo); 
                } else {
                    printf("\n>>> SPEEDUP: Incalculavel (tempo paralelo proximo a zero) <<<\n");
                }
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
    // Impedir dimensões zero ou negativas.
    if (LINHAS <= 0 || COLUNAS <= 0 || LINHAS_POR_BLOCO <= 0 || COLUNAS_POR_BLOCO <= 0) {
        printf("As dimensoes da matriz e dos blocos devem ser positivas.\n");
        return 1;
    }

    // Verificar se os blocos cabem sem sobras.
    if (LINHAS % LINHAS_POR_BLOCO != 0 || COLUNAS % COLUNAS_POR_BLOCO != 0) {
        printf("Os blocos precisam dividir a matriz sem sobras.\n");
        return 1;
    }

    totalBlocos = (uint64_t)(LINHAS / LINHAS_POR_BLOCO) * (COLUNAS / COLUNAS_POR_BLOCO);
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
    double fimMatriz = obterTempoAtual(); // Finaliza a contagem do tempo para gerar a matriz
    printf("Matriz gerada em %.6f segundos.\n", (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1e9);

    menu(); // Chama o menu para interação com o usuário

    matriz = liberarMatriz(matriz, LINHAS, COLUNAS);
    return 0;
}
