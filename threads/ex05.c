//
//  main.c
//  Exemplos
//
//  Created by Paulo Roberto Oliveira Valim on 23/03/17.
//  Copyright © 2017 Paulo Roberto Oliveira Valim. All rights reserved.
//
//Description
/*
 
Neste exemplo, cada thread imprime o valor da variável antes de depois da modificação. Execute o programa várias vezes e observe
o resultado gerado e responda:
- as mensagens de saída são sempre as mesmas? (sim/não e porque)
- caso você tenha observado alguma inconsistência nas mensagens, referentes ao valor da variável anterior e posterior a modificação, 
  o resultado final também está inconsistente? (sim/não e porque)
- modifique o código acrescentando uma variável local a thread que receba o valor da variável compartilhada logo no inicio
  de sua execução e use este valor para fazer a modificação (obs.: a modificação deve ser feita após o envio da mensagem 
  do valor inicial.  
- nas condições acima, como você resolveria o problema de forma a garantir que o resultado final seja o esperado
 
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 3

int variavel_compartilhada = 0;

void *funcao_thread(void *arg) {
    int *id = (int *)arg;
    
    printf("Thread %d - Antes da modificação: %d\n", *id, variavel_compartilhada);
    variavel_compartilhada += 10; // Incrementa a variável compartilhada em 10
    printf("Thread %d - Depois da modificação: %d\n", *id, variavel_compartilhada);

    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    int ids[NUM_THREADS];
    int i;

    for (i = 0; i < NUM_THREADS; i++) {
        ids[i] = i + 1;
        pthread_create(&threads[i], NULL, funcao_thread, &ids[i]);
    }

    for (i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Variável compartilhada após as threads modificarem: %d\n", variavel_compartilhada);

    return 0;
}


