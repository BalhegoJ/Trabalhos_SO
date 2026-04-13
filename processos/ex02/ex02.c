//
// O pai cria um filho. O pai usa wait() para esperar a finalização, mas pode ser interrompido por sinais. 
// Se isso acontecer, wait não retorna o PID esperado e o programa informa que foi interrompido.
//
//  Created by Paulo Roberto Oliveira Valim on 23/03/17.
//  Copyright © 2017 Paulo Roberto Oliveira Valim. All rights reserved.


#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
int main (void) {
    pid_t childpid;
    /* set up signal handlers here ... */
    childpid = fork();
    if (childpid == -1) {
        perror("Failed to fork");
        return 1;
    }
    if (childpid == 0)
        fprintf(stderr, "I am child %ld\n", (long)getpid());
    else if (wait(NULL) != childpid)
        fprintf(stderr, "A signal must have interrupted the wait!\n");
    else
        fprintf(stderr, "I am parent %ld with child %ld\n", (long)getpid(),
                (long)childpid);
    return 0;
}