//
//  main.c
//  Exemplos
//
//  Created by Paulo Roberto Oliveira Valim on 23/03/17.
//  Copyright © 2017 Paulo Roberto Oliveira Valim. All rights reserved.
//
//Description
/*
 
 O exemplo de código a seguir demonstra que um processo filho corresponde a uma imagem (cópia) do 
 processo pai, incluindo variáveis declaradas e seus valores, porém um um outro espaço de endereçamento
 Observar no código que as mudanças realizadas nas variáveis pela execução do código do processo filho
 não alteram o valor das variáveis do processo pai.
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
    int variavel_pai = 10;
    pid_t pid = fork();  // cria um processo filho

    if (pid == -1) {
        // Erro ao criar processo filho
        perror("Erro ao criar processo filho");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Processo filho
        printf("Processo filho - ID: %d\n", getpid());
        printf("Variável do processo filho: %d\n", variavel_pai);
        variavel_pai = 20; // Modifica a variável do processo filho
        printf("Variável modificada do processo filho: %d\n", variavel_pai);
    } else {
        // Processo pai
        printf("Processo pai - ID: %d\n", getpid());
        printf("Variável do processo pai: %d\n", variavel_pai);
        sleep(1); // Aguarda um segundo para permitir que o filho execute
        printf("Variável do processo pai após o filho modificar: %d\n", variavel_pai);
    }

    return 0;
}
