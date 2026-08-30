#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int calcular_pixel(double parte_real, double parte_imaginaria, int max_iteracoes){
    double real = 0.0;
    double imaginaria = 0.0;
    double nova_real;
    double nova_imaginaria;
    int iteracoes = 0;

    while (real * real + imaginaria * imaginaria <= 4.0 && iteracoes < max_iteracoes){
        nova_real = real * real - imaginaria * imaginaria + parte_real;
        nova_imaginaria = 2.0 * real * imaginaria + parte_imaginaria;

        real = nova_real;
        imaginaria = nova_imaginaria;

        iteracoes++;
    }

    return (iteracoes * 255) / max_iteracoes;
}

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

    if (largura <= 0 || altura <= 0 || max_iteracoes <= 0 || quantidade_threads <= 0){
        fprintf(stderr, "erro. os argumentos devem ser maiores que zero.\n");
        return 1;
    }

    return 0;
}