//
// O pai cria um filho; o filho substitui sua imagem de processo para executar um comando passado na linha de comando 
// (ls, date, etc.). O pai espera o término.
//
//  Created by Paulo Roberto Oliveira Valim on 23/03/17.
//  Copyright © 2017 Paulo Roberto Oliveira Valim. All rights reserved.


#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
int main(int argc, char *argv[]) {
    pid_t childpid;
    if (argc < 2){ /* check for valid number of command-line arguments */
        fprintf (stderr, "Usage: %s command arg1 arg2 ...\n", argv[0]);
        return 1;
    }
    childpid = fork();
    if (childpid == -1) {
        perror("Failed to fork");
        return 1;
    }
    if (childpid == 0) { /* child code */
        execvp(argv[1], &argv[1]);
        perror("Child failed to execvp the command");
        return 1;
    }
    if (childpid != wait(NULL)) { /* parent code */
        perror("Parent failed to wait");
        return 1;
    }
    return 0;
}