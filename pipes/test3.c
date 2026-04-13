//
//  main.c
//  Exemplos
//
//  Created by Paulo Roberto Oliveira Valim on 23/03/17.
//  Copyright © 2017 Paulo Roberto Oliveira Valim. All rights reserved.
//
//Description
/*
 
 Exemplo de redirecionamento com o comando dup2.
 Pesquisar o que faz o comando dup() e dup2()

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int fd[2];

    // Criação do pipe
    // fd[0] será o descritor de leitura
    // fd[1] será o descritor de escrita
    if (pipe(fd) == -1) {
        perror("pipe");
        exit(1);
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        // -------- Processo Filho --------
        // Queremos executar: wc -l

        // Redireciona entrada padrão (STDIN) para a extremidade de leitura do pipe
        dup2(fd[0], STDIN_FILENO);

        // Fecha descritores não usados
        close(fd[1]);  // filho não escreve no pipe
        close(fd[0]);  // já foi duplicado em STDIN

        // Substitui o processo filho pelo comando "wc -l"
        execlp("wc", "wc", "-l", NULL);

        // Só chega aqui se execlp falhar
        perror("execlp wc");
        exit(1);

    } else {
        // -------- Processo Pai --------
        // Queremos executar: ls

        // Redireciona saída padrão (STDOUT) para a extremidade de escrita do pipe
        dup2(fd[1], STDOUT_FILENO);

        // Fecha descritores não usados
        close(fd[0]);  // pai não lê do pipe
        close(fd[1]);  // já foi duplicado em STDOUT

        // Substitui o processo pai pelo comando "ls"
        execlp("ls", "ls", NULL);

        // Só chega aqui se execlp falhar
        perror("execlp ls");
        exit(1);
    }

    return 0;
}