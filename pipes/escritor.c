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
#include <string.h>

int main() {
    const char *fifo_name = "meu_pipe";

    // Cria o FIFO (named pipe) com permissões 0666 (leitura e escrita para todos)
    // Se já existir, não há problema (verificamos com -1 e errno == EEXIST)
    if (mkfifo(fifo_name, 0666) == -1) {
        perror("mkfifo");
        // não damos exit(1) aqui, porque pode ser apenas que o FIFO já existe
    }

    // Abre o FIFO para escrita
    // O_WRONLY → modo somente escrita
    // Esse open() pode BLOQUEAR até que outro processo abra o FIFO para leitura
    int fd = open(fifo_name, O_WRONLY);
    if (fd == -1) {
        perror("open");
        exit(1);
    }

    // Mensagem a ser enviada
    char msg[] = "Mensagem via FIFO";

    // Escreve no FIFO
    // Aqui estamos enviando 18 bytes (17 caracteres + '\0')
    if (write(fd, msg, strlen(msg) + 1) == -1) {
        perror("write");
        close(fd);
        exit(1);
    }

    printf("Escrevi no FIFO: %s\n", msg);

    // Fecha o FIFO
    close(fd);

    return 0;
}

