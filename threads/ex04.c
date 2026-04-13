//
//  main.c
//  Exemplos
//
//  Created by Paulo Roberto Oliveira Valim on 23/03/17.
//  Copyright © 2017 Paulo Roberto Oliveira Valim. All rights reserved.
//
//Description
/*
 
Neste exemplo, cada thread executa uma operação diferente na variável compartilhada, demonstrando como várias threads podem 
modificar uma variável compartilhada simultaneamente com diferentes operações aritméticas.
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 4

int variavel_compartilhada = 10; // Valor inicial da variável compartilhada

// Estrutura para armazenar informações sobre as operações das threads
typedef struct {
    int id;
    char operacao;
} ThreadInfo;

void *funcao_thread(void *arg) {
    ThreadInfo *info = (ThreadInfo *)arg;

    switch(info->operacao) {
        case '+':
            printf("Thread %d - Adicionando 5 à variável compartilhada\n", info->id);
            variavel_compartilhada += 5; // Adiciona 5 à variável compartilhada
            break;
        case '-':
            printf("Thread %d - Subtraindo 3 da variável compartilhada\n", info->id);
            variavel_compartilhada -= 3; // Subtrai 3 da variável compartilhada
            break;
        case '*':
            printf("Thread %d - Multiplicando a variável compartilhada por 2\n", info->id);
            variavel_compartilhada *= 2; // Multiplica a variável compartilhada por 2
            break;
        case '/':
            printf("Thread %d - Dividindo a variável compartilhada por 2\n", info->id);
            variavel_compartilhada /= 2; // Divide a variável compartilhada por 2
            break;
        default:
            printf("Operação inválida\n");
    }

    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    ThreadInfo infos[NUM_THREADS];
    int i;

    // Criação das threads com diferentes operações
    for (i = 0; i < NUM_THREADS; i++) {
        infos[i].id = i + 1;
        if (i % 4 == 0) {
            infos[i].operacao = '+';
        } else if (i % 4 == 1) {
            infos[i].operacao = '-';
        } else if (i % 4 == 2) {
            infos[i].operacao = '*';
        } else {
            infos[i].operacao = '/';
        }
        pthread_create(&threads[i], NULL, funcao_thread, &infos[i]);
    }

    // Aguarda a finalização das threads
    for (i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Variável compartilhada após as threads modificarem: %d\n", variavel_compartilhada);

    return 0;
}


