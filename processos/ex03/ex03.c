//
// Cada processo cria apenas um filho, formando uma cadeia (não uma árvore). Cada filho espera o próprio filho 
// terminar antes de imprimir sua mensagem. Como as mensagens geradas são ordenadas?
//
//  Created by Paulo Roberto Oliveira Valim on 23/03/17.
//  Copyright © 2017 Paulo Roberto Oliveira Valim. All rights reserved.
//


#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
int main (int argc, char *argv[]) {
    pid_t childpid;
    int i, n;
    pid_t waitreturn;
    if (argc != 2){ /* check number of command-line arguments */
        fprintf(stderr, "Usage: %s processes\n", argv[0]);
        return 1;
    }
    n = atoi(argv[1]);
    for (i = 1; i < n; i++)
        if ((childpid = fork()) != 0)
            break;
    while(childpid != (waitreturn = wait(NULL)))
        if ((waitreturn == -1) && (errno != EINTR))
            break;
    fprintf(stderr, "I am process %ld, my parent is %ld\n",
            (long)getpid(), (long)getppid());
    return 0;
}