//
//  main.c
//  Exemplos
//
//  Created by Paulo Roberto Oliveira Valim on 23/03/17.
//  Copyright © 2017 Paulo Roberto Oliveira Valim. All rights reserved.
//
//Description
/*
 O exemplo de código a seguir demonstra troca de mensagem entre pai e filho 
 Pergunta: as mensagem observadas durante a execução poderiam estar em outra ordem?

*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main() {
    int fd1[2], fd2[2];

    // Cria o primeiro pipe (pai -> filho)
    if (pipe(fd1) == -1) {
        perror("pipe fd1");
        exit(1);
    }

    // Cria o segundo pipe (filho -> pai)
    if (pipe(fd2) == -1) {
        perror("pipe fd2");
        exit(1);
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        // -------- Processo Filho --------
        // Fecha descritores que não vai usar:
        // - Não vai escrever em fd1 (pai->filho), só ler
        // - Não vai ler de fd2 (filho->pai), só escrever
        close(fd1[1]);
        close(fd2[0]);

        char buf[100];
        // Lê a mensagem enviada pelo pai
        read(fd1[0], buf, sizeof(buf));
        printf("Filho recebeu: %s\n", buf);

        // Responde ao pai
        char msg[] = "Tudo bem, pai!";
        write(fd2[1], msg, strlen(msg) + 1);

        // Fecha descritores abertos
        close(fd1[0]);
        close(fd2[1]);

        exit(0);

    } else {
        // -------- Processo Pai --------
        // Fecha descritores que não vai usar:
        // - Não vai ler de fd1 (pai->filho), só escrever
        // - Não vai escrever em fd2 (filho->pai), só ler
        close(fd1[0]);
        close(fd2[1]);

        // Envia mensagem ao filho
        char msg[] = "Oi filho!";
        write(fd1[1], msg, strlen(msg) + 1);

        // Lê a resposta do filho
        char buf[100];
        read(fd2[0], buf, sizeof(buf));
        printf("Pai recebeu: %s\n", buf);

        // Fecha descritores abertos
        close(fd1[1]);
        close(fd2[0]);

        // Espera o filho terminar para evitar processo zumbi
        wait(NULL);
    }

    return 0;
}
