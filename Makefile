CC = gcc
CFLAGS = -Wall -Wextra -O2 -pthread
LDLIBS = -lm

TARGET = primos

.PHONY: all run clean

# "make" executa o primeiro alvo (all), que depende de run.
# Primeiro compila quando necessário e executa o programa.
# Se run terminar com sucesso, $(MAKE) clean remove o executável.
# A limpeza acontece depois da execução, mesmo usando make -j.
all: run
	$(MAKE) clean

# Regra de compilação usada por run: gera primos a partir de main.c.
# Só recompila se o executável não existir ou se main.c for mais recente.
# $@ representa o nome do alvo: primos.
$(TARGET): main.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) main.c -o $@ $(LDLIBS)

# "make run" executa este alvo diretamente.
# Primeiro verifica a compilação de primos e depois executa ./primos.
# Quando chamado diretamente, mantém o executável (não chama clean).
run: $(TARGET)
	./$(TARGET)

# "make clean" executa apenas este alvo: remove o executável primos.
# $(RM) equivale a rm -f; o código-fonte main.c é mantido.
clean:
	$(RM) $(TARGET)
