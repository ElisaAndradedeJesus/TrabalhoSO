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

## Organização inicial

- `[SO 2026-2] Trabalho 1 (PThreads).pdf`: especificação do professor.
- `README.md`: apresentação do projeto e orientações de colaboração.
- `main.c` (a criar): único arquivo fonte do programa.
- O relatório será escrito em um arquivo Markdown separado e exportado para PDF para a entrega.

## Por onde começar

1. Ler a especificação e combinar a divisão de tarefas entre os integrantes.
2. Criar `main.c` com os parâmetros da matriz, alocação dinâmica com verificação de falhas, geração dos números com semente fixa e liberação da memória.
3. Implementar `ehPrimo` e a busca serial. Conferir o resultado com uma matriz pequena cujos primos possam ser contados manualmente.
4. Medir o tempo da busca serial e escolher uma matriz que leve mais de 5 segundos nessa busca, sem uso significativo de swap.
5. Implementar a distribuição de macroblocos e a busca com PThreads e mutexes. Comparar a contagem com a versão serial.
6. Realizar os experimentos exigidos nas duas máquinas e registrar os resultados para o relatório.

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

## Compilação

O programa ainda não foi implementado. Quando `main.c` estiver disponível, um comando de referência para Linux com GCC será:

```bash
gcc -Wall -Wextra -O2 -pthread main.c -o primos -lm
./primos
```

Atualizem esta seção quando forem definidos os modos de execução. Registrem o compilador e as opções usadas nos testes para permitir a reprodução dos resultados.

## Entrega

Entregar um ZIP ou RAR com o **relatório em PDF e o único arquivo fonte `.c`**, com a nomenclatura exigida:

```text
[SO 2026-2] Trabalho 1 - Aluno1, Aluno2, Aluno3
```

O Markdown do relatório serve para edição; a versão entregue deve ser PDF. Confirmem o prazo no AVA ou com o professor, pois a data indicada no PDF está inconsistente com o período 2026/2.
