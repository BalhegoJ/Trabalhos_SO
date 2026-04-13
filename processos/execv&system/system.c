// A função system() - Filho e Pai, sem crimes
// Assim como a função execv, a função system também executa um programa, na verdade ela simula o terminal, então é como se déssemos um comando no terminal.
// A diferença é que aqui a função filho não mata a pai, portanto, não há crimes ;)

// Vamos usar a função system para executar o mesmo arquivo "teste", isso é feito no// /terminal assim ./teste

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main(void)
{
    printf("pid do Pai: %d\n", getpid());
    system("./teste");
    printf("\nPrograma apos a funcao system\n");
}
