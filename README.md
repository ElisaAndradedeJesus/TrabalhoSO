# Trabalho 1 — Sistemas Operacionais

Repositório de colaboração do grupo para o Trabalho 1 de Sistemas Operacionais (2026/2): **Introdução à Programação Multithread com PThreads**.

O objetivo é desenvolver um programa em C que conte números primos em uma matriz, primeiro de forma serial e depois de forma paralela, e analisar o desempenho das duas abordagens.

## Objetivo do repositório

Centralizar o código e as orientações do trabalho, permitindo que os integrantes acompanhem as alterações, discutam decisões e contribuam com a implementação e os testes.

As regras completas estão no arquivo `[SO 2026-2] Trabalho 1 (PThreads).pdf`. Em caso de dúvida, consultem a especificação e as orientações do professor.

## O que precisamos desenvolver

- Um único arquivo fonte `.c`, organizado em funções e usando PThreads.
- Uma matriz global alocada dinamicamente, com dimensões configuradas por `#define` e números de 0 a 31999 gerados com semente fixa.
- A função `int ehPrimo(int n)` e a contagem serial, exibindo o total de primos e o tempo gasto.
- A contagem paralela com distribuição dinâmica de macroblocos de mesmo tamanho, configurados por `#define`, sem copiar a matriz.
- Mutexes para proteger o contador global de primos e o controle global de distribuição dos blocos.
- Uma forma simples de executar a busca serial, a paralela ou ambas, mostrando tempos e speedup.
- Testes de desempenho em dois computadores diferentes, variando macroblocos e quantidade de threads, incluindo os experimentos exigidos com muitas threads e sem mutexes.
- Um relatório com configurações das máquinas, tabelas, gráficos, análises e conclusão.

As threads devem ser criadas uma vez por busca paralela e buscar novos blocos até esgotar o trabalho. Não é permitido usar um vetor de resultados por macrobloco para somar ao final.

## Status atual

A geração da matriz e a contagem serial estão implementadas. A busca paralela está em desenvolvimento e ainda não realiza contagem.

### Implementado

- Matriz global alocada dinamicamente com `calloc`, tratamento de falhas e liberação da memória.
- Dimensões configuradas por `#define`: atualmente `79056 × 79056`.
- Geração de valores de 0 a 31999 usando semente fixa `67`.
- Medição separada do tempo de geração da matriz e da contagem serial.
- Contagem serial com tabela de consulta preparada pelo Crivo de Eratóstenes.
- Menu com tratamento de entrada inválida e opções para serial, paralela, ambas e saída.
- Makefile com suporte a PThreads e `.gitignore` para o executável `primos`.

### Em desenvolvimento

- `contarPrimosParalelo()` contém apenas o esboço de um laço; ainda não cria threads.
- `macroBloco()` está vazia.
- As constantes `MAX_THREADS`, `MAX_LINHAS_POR_THREAD` e `MAX_COLUNAS_POR_THREAD` foram declaradas. A primeira aparece no laço do esboço; as dimensões ainda não controlam a divisão em blocos.
- `controleMacroBlocos` está declarado, mas ainda não distribui trabalho.

### Pendências para atender ao enunciado

- Substituir a tabela do crivo pela função `int ehPrimo(int n)` com teste de divisores até a raiz, conforme solicitado pelo professor. Atualmente `ehPrimo` é o nome de um vetor, não de uma função.
- Calcular as coordenadas dos macroblocos sem copiar a matriz, permitindo variar altura e largura por `#define` e garantindo blocos de mesmo tamanho.
- Criar as threads, distribuir blocos dinamicamente e aguardar a conclusão com `pthread_join()`.
- Proteger com mutexes a reserva de blocos e a atualização do total global. Cada thread deve contar localmente por bloco e somar ao global ao terminar esse bloco.
- Rever os tipos dos contadores e dos cálculos: com a matriz atual, blocos `1 × 1` totalizam 6.249.851.136 blocos, além da capacidade usual de um `int` de 32 bits.
- Conectar a busca paralela ao menu e implementar a opção de executar ambas, comparar resultados e mostrar o speedup (`tempo serial / tempo paralelo`).
- Validar primeiro com matrizes pequenas, realizar os experimentos em duas máquinas e elaborar o relatório.

**Limitação atual do menu:** somente a opção `1` executa uma contagem. A opção `2` exibe tempo e total sem chamar uma busca, portanto seus resultados não representam desempenho paralelo. A opção `3` ainda não executa nenhuma busca.

## Parâmetros e memória

A matriz atual tem 6.249.851.136 elementos. Considerando `int` de 4 bytes, são aproximadamente **25 GB apenas para os elementos**, além dos ponteiros e do custo das alocações. Ajustem `LINHAS` e `COLUNAS` em `main.c` para a memória disponível em cada computador e evitem uso significativo de swap durante as medições.

O tamanho final de benchmark deve ser reavaliado após adequar a função de primalidade: o enunciado sugere `10000 × 10000` como início e uma busca serial superior a 5 segundos. Para desenvolver e conferir resultados, usem matrizes pequenas.

## Organização dos arquivos

- `[SO 2026-2] Trabalho 1 (PThreads).pdf`: especificação do professor.
- `[SO] Introdução à Programação Concorrente com PThreads.pdf`: material sobre threads e mutexes.
- `[Apostila] Programação em C (v2.0.0).pdf`: material de apoio de C.
- `main.c`: único arquivo fonte, com geração, busca serial e esboços da busca paralela.
- `Makefile`: compilação, execução e limpeza do executável.
- `.gitignore`: impede que o executável `primos` seja incluído normalmente nos commits.
- `README.md`: apresentação, status e orientações de colaboração.

O relatório ainda não está presente no repositório. A proposta é escrevê-lo em Markdown separado e exportá-lo para PDF para a entrega.

## Como contribuir

Depois que o repositório estiver no GitHub, cada integrante deve clonar o projeto usando a URL do repositório:

```bash
git clone <URL_DO_REPOSITORIO>
cd <PASTA_DO_REPOSITORIO>
```

Antes de iniciar uma tarefa, combinem quem ficará responsável por ela. Como o código precisa ficar em um único `.c`, alterações simultâneas nas mesmas funções podem gerar conflitos.

Partindo da branch principal atualizada, criem uma branch para cada tarefa. O exemplo abaixo considera que a branch principal se chama `main`:

```bash
git switch main
git pull --ff-only
git switch -c feat/busca-serial
```

Façam alterações pequenas e commits com mensagens que expliquem a mudança. Adicionem apenas os arquivos relacionados à tarefa:

```bash
git add main.c
git commit -m "Implementa contagem serial de primos"
git push -u origin feat/busca-serial
```

No GitHub, abram um Pull Request para a branch principal, descrevendo o que mudou e como foi verificado. Peçam a outro integrante para revisar antes de integrar a alteração.

- Confiram os avisos do compilador e a igualdade das contagens serial e paralela ao alterar o programa.
- Registrem os parâmetros usados e a configuração da máquina junto dos resultados de cada experimento.
- Não incluam executáveis, arquivos temporários da IDE ou dados pessoais no repositório.
- Mantenham a versão com mutexes funcionando; identifiquem claramente a variante experimental sem proteção.
- Todos devem compreender o código e conseguir explicar as decisões e os resultados, conforme as exigências da disciplina.

## Compilação e execução

Em Linux, com GCC e Make disponíveis:

```bash
make
```

Esse comando compila quando necessário, executa o programa e remove o executável se a execução terminar com sucesso. A matriz é gerada antes de aparecer o menu.

Outras opções:

```bash
make primos  # Apenas compila
make run     # Compila quando necessário e executa, mantendo o executável
make clean   # Remove o executável
```

Para compilar e executar diretamente:

```bash
gcc -Wall -Wextra -O2 -pthread main.c -o primos -lm
./primos
```

A implementação paralela ainda está incompleta; a presença de `-pthread` habilita o suporte na compilação, mas não significa que o programa já crie threads. Registrem o compilador e as opções usados nos testes para permitir a reprodução dos resultados.

## Entrega

Entregar um ZIP ou RAR com o **relatório em PDF e o único arquivo fonte `.c`**, com a nomenclatura exigida:

```text
[SO 2026-2] Trabalho 1 - Aluno1, Aluno2, Aluno3
```

O Markdown do relatório serve para edição; a versão entregue deve ser PDF. Confirmem o prazo no AVA ou com o professor, pois a data indicada no PDF está inconsistente com o período 2026/2.
