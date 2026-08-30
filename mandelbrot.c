#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

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

void calcular_openmp(int largura, int altura, int max_iteracoes, int quantidade_threads, int *imagem){
    int x;
    int y;
    double parte_real;
    double parte_imaginaria;

    omp_set_num_threads(quantidade_threads);

    #pragma omp parallel for
    for (y = 0; y < altura; y++){
        parte_imaginaria = 1.5 - (3.0 * y / (altura - 1));

        for (x = 0; x < largura; x++){
            parte_real = -2.0 + (3.0 * x / (largura - 1));
            imagem[y * largura + x] = calcular_pixel(parte_real, parte_imaginaria, max_iteracoes);
        }
    }
}

int main(int quantidade_argumentos, char *argumentos[]){
    int largura;
    int altura;
    int max_iteracoes;
    int quantidade_threads;
    int intensidade;
    FILE *arquivo_serial;
    FILE *arquivo_tempo;
    FILE *arquivo_openmp;
    clock_t inicio;
    clock_t fim;
    int x;
    int y;
    double parte_real;
    double parte_imaginaria;
    int *imagem_openmp;

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

    imagem_openmp = malloc(largura * altura * sizeof(int));

    if (imagem_openmp == NULL){
        fprintf(stderr, "erro. não foi possível reservar memória para a imagem openmp.\n");
        return 1;
    }

    arquivo_serial = fopen("mandelbrot_gllb_serial.pgm", "w");

    if (arquivo_serial == NULL){
        fprintf(stderr, "erro. não foi possível criar o arquivo serial.\n");
        return 1;
    }

    arquivo_openmp = fopen("mandelbrot_gllb_openmp.pgm", "w");

    if (arquivo_openmp == NULL){
        fprintf(stderr, "erro. não foi possível criar o arquivo openmp.\n");
        fclose(arquivo_serial);
        free(imagem_openmp);
        return 1;
    }

    arquivo_tempo = fopen("times.txt", "a");

    if (arquivo_tempo == NULL){
        fprintf(stderr, "erro. não foi possível criar o arquivo de tempo.\n");
        fclose(arquivo_serial);
        fclose(arquivo_openmp);
        free(imagem_openmp);
        return 1;
    }

    inicio = clock();

    for (y = 0; y < altura; y++){
        parte_imaginaria = 1.5 - (3.0 * y / (altura - 1));

        for (x = 0; x < largura; x++){
            parte_real = -2.0 + (3.0 * x / (largura - 1));
            intensidade = calcular_pixel(parte_real, parte_imaginaria, max_iteracoes);
            fprintf(arquivo_serial, "%d ", intensidade);
        }
        fprintf(arquivo_serial, "\n");
    }

    fim = clock();

    fprintf(arquivo_tempo, "serial: %.8f segundos\n", (double)(fim - inicio) / CLOCKS_PER_SEC);

    fclose(arquivo_serial);
    fclose(arquivo_tempo);

    return 0;
}