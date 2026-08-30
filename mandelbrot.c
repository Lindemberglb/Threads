#include <stdio.h>
#include <stdlib.h>

int main(int quantidade_argumentos, char *argumentos[]){
    int largura;
    int altura;
    int max_iteracoes;
    int quantidade_threads;

    if (quantidade_argumentos != 5){
        fprintf(stderr, "erro. quantidade de argumentos inválida.\n");
        return 1;
    }

    largura = atoi(argumentos[1]);
    altura = atoi(argumentos[2]);
    max_iteracoes = atoi(argumentos[3]);
    quantidade_threads = atoi(argumentos[4]);

    return 0;
}