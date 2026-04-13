//
//  main.c
//  Exemplos
//
//  Created by Paulo Roberto Oliveira Valim on 23/03/17.
//  Copyright © 2017 Paulo Roberto Oliveira Valim. All rights reserved.
//
//Description
/*
   Exemplo mostrando como escrever em um FIFO (named pipe), diferente do pipe() anônimo que existe 
   apenas entre processos relacionados.

   obs: o pipe deve ser criado antes de executar o leitor:
    $ mkfifo meu_pipe


*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

int main() {
    const char *fifo_name = "meu_pipe";

    // Abre o FIFO para leitura
    // O_RDONLY → modo somente leitura
    // Essa chamada pode BLOQUEAR até que algum processo abra o FIFO para escrita
    int fd = open(fifo_name, O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(1);
    }

    char buf[100];

    // Lê dados do FIFO
    // read() bloqueia até que haja dados disponíveis
    ssize_t n = read(fd, buf, sizeof(buf));
    if (n == -1) {
        perror("read");
        close(fd);
        exit(1);
    }

    // Garante que a string está terminada em '\0'
    if (n < sizeof(buf)) {
        buf[n] = '\0';
    } else {
        buf[sizeof(buf) - 1] = '\0';
    }

    // Mostra a mensagem recebida
    printf("Recebido: %s\n", buf);

    // Fecha o FIFO
    close(fd);

    return 0;
}
