//
//  main.c
//  Exemplos
//
//  Created by Paulo Roberto Oliveira Valim on 23/03/17.
//  Copyright © 2017 Paulo Roberto Oliveira Valim. All rights reserved.
//
//Description
/*
 
 O exemplo de código a seguir demonstra o envio de mensagem do pai para o filho via PIPE.
 - fazer uma versão na qual o filho envia mensagem para o pai
 - pesquisar e responder as seguintes questões:
   a) o que acontece quando o buffer de leitura é menor que a mensagem disponível?
   b) é possível verificar quantos bytes estão disponíveis antes de efetuar a leitura?
   c) é possível ler/escrever outros dados que não sejam string: inteiros, estruturas etc? Qual o cuidado 
      que se deve ter?

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main() {
    int fd[2];  
    // fd[0] será o descritor de leitura do pipe
    // fd[1] será o descritor de escrita do pipe

    // Criação do pipe: gera dois descritores de arquivo conectados
    if (pipe(fd) == -1) {
        perror("pipe");   // A função perror serve para imprimir na saída de erro padrão (stderr) uma mensagem 
                          // de erro associada ao valor da variável global errno (que é configurada pelas 
                          // chamadas de sistema quando algo dá errado).
        exit(1);
    }

    // Criação de um novo processo usando fork()
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");   // Se falhar ao criar o processo
        exit(1);
    }

    if (pid == 0) {
        // ---------- Processo Filho ----------
        // O filho só precisa ler, então fecha o descritor de escrita
        close(fd[1]);

        char buf[3];
        // Lê do pipe (do descritor de leitura fd[0])
        // O read() vai bloquear até o pai escrever alguma coisa
        read(fd[0], buf, sizeof(buf));

        printf("Filho recebeu: %s\n", buf);

        // Fecha o descritor de leitura após o uso
        close(fd[0]);

        // Termina o processo filho
        exit(0);

    } else {
        // ---------- Processo Pai ----------
        // O pai só vai escrever, então fecha o descritor de leitura
        close(fd[0]);

        char msg[] = "Olá filho!";
        // Escreve a mensagem no pipe
        // O "+1" é para incluir o caractere nulo '\0' do final da string
        write(fd[1], msg, strlen(msg) + 1);

        // Fecha o descritor de escrita após enviar a mensagem
        close(fd[1]);

        // O pai espera o filho terminar para evitar processo zumbi
        wait(NULL);
    }

    return 0;
}

