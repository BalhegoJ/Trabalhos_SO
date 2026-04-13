//
//  main.c
//  Exemplos
//
//  Created by Paulo Roberto Oliveira Valim on 23/03/17.
//  Copyright © 2017 Paulo Roberto Oliveira Valim. All rights reserved.
//
//Description
/*
 
 O exemplo de código a seguir demonstra que o espaço de endereçamento das variáveis globais do 
 processo que cria as threads são compartilhadas pela threads. O mesmo já não acontece para as 
 variáveis locais pois estas são alocadas na pilha e cada thread tem sua própria pilha.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

int variavel_compartilhada = 10;

void *funcao_thread(void *arg) {
    printf("Thread - ID: %ld\n", pthread_self());
    printf("Variável compartilhada na thread: %d\n", variavel_compartilhada);
    variavel_compartilhada = 20; // Modifica a variável compartilhada
    printf("Variável compartilhada modificada na thread: %d\n", variavel_compartilhada);
    return NULL;
}

int main() {
    pthread_t thread;
    printf("Processo principal - ID: %d\n", getpid());
    printf("Variável compartilhada no processo principal: %d\n", variavel_compartilhada);

    pthread_create(&thread, NULL, funcao_thread, NULL);
    pthread_join(thread, NULL); // Aguarda a finalização da thread

    printf("Variável compartilhada no processo principal após a thread modificar: %d\n", variavel_compartilhada);

    return 0;
}

