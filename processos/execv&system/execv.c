// A função execv() - Filho matando o Pai
// A função execv() é usada para executar um programa, em ambientes Linux.
// Ou seja, ela cria um processo novo, que é o programa que vai executar.
//
// Para ver a execv em ação é preciso gerar o programa de nome "teste" que simplesmente exibe a PID desse arquivo "teste"

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

int main(void)
{
        // argv precisa terminar em NULL
    char *args[] = { "teste", NULL };

    printf("pid do Pai: %d\n", getpid());
    execv("./teste", args);
    printf("EU TENHO UM SEGREDO PRA CONTAR\n");
}


