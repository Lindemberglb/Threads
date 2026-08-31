#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <pthread.h>
#include <limits.h>

typedef struct thread{
    int inicio;
    int fim;
    int intervalo;
    int largura;
    int altura;
    int max_iteracoes;
    int *imagem;
}thread;

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

void *calcular_pthreads1(void *dados){
    thread *informacoes = (thread *)dados;
    int i;
    int x;
    int y;
    double parte_real;
    double parte_imaginaria;

    for (i = informacoes->inicio; i < informacoes->fim; i++){
        y = i / informacoes->largura;
        x = i % informacoes->largura;

        if (informacoes->altura == 1){
            parte_imaginaria = 0.0;
        }
        else{
            parte_imaginaria = 1.5 - (3.0 * y / (informacoes->altura - 1));
        }

        if (informacoes->largura == 1){
            parte_real = -0.5;
        }
        else{
            parte_real = -2.0 + (3.0 * x / (informacoes->largura - 1));
        }

        informacoes->imagem[i] = calcular_pixel(parte_real, parte_imaginaria, informacoes->max_iteracoes);
    }
    return NULL;
}

void *calcular_pthreads2(void *dados){
    thread *informacoes = (thread *)dados;
    int y;
    int x;
    int i;

    for (y = informacoes->inicio; y < informacoes->altura; y += informacoes->intervalo){
        for (x = 0; x < informacoes->largura; x++){
            i = y * informacoes->largura + x;

            double parte_imaginaria;
            double parte_real;

            if (informacoes->altura == 1){
                parte_imaginaria = 0.0;
            }
            else{
                parte_imaginaria = 1.5 - (3.0 * y / (informacoes->altura - 1));
            }

            if (informacoes->largura == 1){
                parte_real = -0.5;
            }
            else{
                parte_real = -2.0 + (3.0 * x / (informacoes->largura - 1));
            }

            informacoes->imagem[i] = calcular_pixel(parte_real, parte_imaginaria, informacoes->max_iteracoes);
        }
    }
    return NULL;
}

void calcular_openmp(int largura, int altura, int max_iteracoes, int quantidade_threads, int *imagem){
    int y;

    omp_set_num_threads(quantidade_threads);

    #pragma omp parallel for
    for (y = 0; y < altura; y++){
        double parte_imaginaria;

        if (altura == 1){
            parte_imaginaria = 0.0;
        }
        else{
            parte_imaginaria = 1.5 - (3.0 * y / (altura - 1));
        }

        for (int x = 0; x < largura; x++){
            double parte_real;

            if (largura == 1){
                parte_real = -0.5;
            }
            else{
                parte_real = -2.0 + (3.0 * x / (largura - 1));
            }

            imagem[y * largura + x] = calcular_pixel(parte_real, parte_imaginaria, max_iteracoes);
        }
    }
}

double tempo_decorrido(struct timespec inicio, struct timespec fim){
    return (fim.tv_sec - inicio.tv_sec) + (fim.tv_nsec - inicio.tv_nsec) / 1e9;
}

int ler_inteiro_positivo(const char *texto, int *valor){
    char *fim;
    long numero = strtol(texto, &fim, 10);

    if (*fim != '\0' || numero <= 0 || numero > INT_MAX){
        return 0;
    }

    *valor = (int)numero;
    return 1;
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
    struct timespec inicio;
    struct timespec fim;
    int x;
    int y;
    double parte_real;
    double parte_imaginaria;
    int *imagem_openmp;
    int *imagem_pthreads1;
    int *imagem_pthreads2;
    pthread_t *threads;
    thread *informacoes_threads;
    FILE *arquivo_pthreads1;
    FILE *arquivo_pthreads2;

    if (quantidade_argumentos != 5){
        fprintf(stderr, "erro. quantidade de argumentos inválida.\n");
        return 1;
    }

    if (!ler_inteiro_positivo(argumentos[1], &largura)){
        fprintf(stderr, "erro. largura inválida.\n");
        return 1;
    }

    if (!ler_inteiro_positivo(argumentos[2], &altura)){
        fprintf(stderr, "erro. altura inválida.\n");
        return 1;
    }

    if (!ler_inteiro_positivo(argumentos[3], &max_iteracoes)){
        fprintf(stderr, "erro. número de iterações inválido.\n");
        return 1;
    }

    if (!ler_inteiro_positivo(argumentos[4], &quantidade_threads)){
        fprintf(stderr, "erro. número de threads inválido.\n");
        return 1;
    }

    imagem_openmp = malloc(largura * altura * sizeof(int));

    if (imagem_openmp == NULL){
        fprintf(stderr, "erro. não foi possível alocar a imagem openmp.\n");
        return 1;
    }

    imagem_pthreads1 = malloc(largura * altura * sizeof(int));

    if(imagem_pthreads1 == NULL){
        fprintf(stderr, "erro. não foi possível alocar a imagem pthreads1.\n");
        free(imagem_openmp);
        return 1;
    }

    imagem_pthreads2 = malloc(largura * altura * sizeof(int));

    if(imagem_pthreads2 == NULL){
        fprintf(stderr, "erro. não foi possível alocar a imagem pthreads2.\n");
        free(imagem_openmp);
        free(imagem_pthreads1);
        return 1;
    }

    threads = malloc(quantidade_threads * sizeof(pthread_t));
    
    if(threads == NULL){
        fprintf(stderr, "erro. não foi possível alocar threads.\n");
        free(imagem_openmp);
        free(imagem_pthreads1);
        free(imagem_pthreads2);
        return 1;
    }

    informacoes_threads = malloc(quantidade_threads * sizeof(thread));

    if(informacoes_threads == NULL){
        fprintf(stderr, "erro. nao foi possível alocar informacoes_threads.\n");
        free(imagem_openmp);
        free(imagem_pthreads1);
        free(imagem_pthreads2);
        free(threads);
        return 1;
    }

    arquivo_serial = fopen("mandelbrot_gllb_serial.pgm", "w");

    if (arquivo_serial == NULL){
        fprintf(stderr, "erro. não foi possível criar o arquivo serial.\n");
        free(imagem_openmp);
        free(imagem_pthreads1);
        free(imagem_pthreads2);
        free(threads);
        free(informacoes_threads);
        return 1;
    }

    arquivo_openmp = fopen("mandelbrot_gllb_openmp.pgm", "w");

    if (arquivo_openmp == NULL){
        fprintf(stderr, "erro. não foi possível criar o arquivo openmp.\n");
        fclose(arquivo_serial);
        free(imagem_openmp);
        free(imagem_pthreads1);
        free(imagem_pthreads2);
        free(threads);
        free(informacoes_threads);
        return 1;
    }

    arquivo_pthreads1 = fopen("mandelbrot_gllb_pthreads1.pgm", "w");

    if (arquivo_pthreads1 == NULL){
        fprintf(stderr, "erro. não foi possível criar o arquivo pthreads 1.\n");
        fclose(arquivo_serial);
        fclose(arquivo_openmp);
        free(imagem_openmp);
        free(imagem_pthreads1);
        free(imagem_pthreads2);
        free(threads);
        free(informacoes_threads);
        return 1;
    }

    arquivo_pthreads2 = fopen("mandelbrot_gllb_pthreads2.pgm", "w");

    if (arquivo_pthreads2 == NULL){
        fprintf(stderr, "erro. não foi possível criar o arquivo pthreads 2.\n");
        fclose(arquivo_serial);
        fclose(arquivo_openmp);
        fclose(arquivo_pthreads1);
        free(imagem_openmp);
        free(imagem_pthreads1);
        free(imagem_pthreads2);
        free(threads);
        free(informacoes_threads);
        return 1;
    }

    arquivo_tempo = fopen("times.txt", "w");

    if (arquivo_tempo == NULL){
        fprintf(stderr, "erro. não foi possível criar o arquivo de tempo.\n");
        fclose(arquivo_serial);
        fclose(arquivo_openmp);
        fclose(arquivo_pthreads1);
        fclose(arquivo_pthreads2);
        free(imagem_openmp);
        free(imagem_pthreads1);
        free(imagem_pthreads2);
        free(threads);
        free(informacoes_threads);
        return 1;
    }

    clock_gettime(CLOCK_MONOTONIC, &inicio);

    for (y = 0; y < altura; y++){
        if (altura == 1){
            parte_imaginaria = 0.0;
        }
        else{
            parte_imaginaria = 1.5 - (3.0 * y / (altura - 1));
        }

        for (x = 0; x < largura; x++){
            if (largura == 1){
                parte_real = -0.5;
            }
            else{
                parte_real = -2.0 + (3.0 * x / (largura - 1));
            }
            intensidade = calcular_pixel(parte_real, parte_imaginaria, max_iteracoes);
            fprintf(arquivo_serial, "%d ", intensidade);
        }
        fprintf(arquivo_serial, "\n");
    }

    clock_gettime(CLOCK_MONOTONIC, &fim);

    fprintf(arquivo_tempo, "serial: %.8f segundos\n", tempo_decorrido(inicio, fim));

    clock_gettime(CLOCK_MONOTONIC, &inicio);

    calcular_openmp(largura, altura, max_iteracoes, quantidade_threads, imagem_openmp);

    clock_gettime(CLOCK_MONOTONIC, &fim);

    for (y = 0; y < altura; y++){
        for (x = 0; x < largura; x++){
            fprintf(arquivo_openmp, "%d ", imagem_openmp[y * largura + x]);
        }
        fprintf(arquivo_openmp, "\n");
    }

    fprintf(arquivo_tempo, "openmp: %.8f segundos\n", tempo_decorrido(inicio, fim));
    
    clock_gettime(CLOCK_MONOTONIC, &inicio);

    for (int i = 0; i < quantidade_threads; i++){
        informacoes_threads[i].inicio = i * (largura * altura) / quantidade_threads;
        informacoes_threads[i].fim = (i + 1) * (largura * altura) / quantidade_threads;
        informacoes_threads[i].largura = largura;
        informacoes_threads[i].altura = altura;
        informacoes_threads[i].max_iteracoes = max_iteracoes;
        informacoes_threads[i].imagem = imagem_pthreads1;

        if (pthread_create(&threads[i], NULL, calcular_pthreads1, &informacoes_threads[i]) != 0){
            fprintf(stderr, "erro. não foi possível criar a thread %d.\n", i);
            for (int j = 0; j < i; j++){
                pthread_join(threads[j], NULL);
            }
            fclose(arquivo_serial);
            fclose(arquivo_openmp);
            fclose(arquivo_pthreads1);
            fclose(arquivo_pthreads2);
            fclose(arquivo_tempo);
            free(imagem_openmp);
            free(imagem_pthreads1);
            free(imagem_pthreads2);
            free(threads);
            free(informacoes_threads);
            return 1;
        }
    }

    for (int i = 0; i < quantidade_threads; i++){
        if (pthread_join(threads[i], NULL) != 0){
            fprintf(stderr, "erro. não foi possível esperar a thread %d.\n", i);
            fclose(arquivo_serial);
            fclose(arquivo_openmp);
            fclose(arquivo_pthreads1);
            fclose(arquivo_pthreads2);
            fclose(arquivo_tempo);
            free(imagem_openmp);
            free(imagem_pthreads1);
            free(imagem_pthreads2);
            free(threads);
            free(informacoes_threads);
            return 1;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &fim);

    for (y = 0; y < altura; y++){
        for (x = 0; x < largura; x++){
            fprintf(arquivo_pthreads1, "%d ", imagem_pthreads1[y * largura + x]);
        }
        fprintf(arquivo_pthreads1, "\n");
    }

    fprintf(arquivo_tempo, "pthreads1: %.8f segundos\n", tempo_decorrido(inicio, fim));

    clock_gettime(CLOCK_MONOTONIC, &inicio);

    for (int i = 0; i < quantidade_threads; i++){
        informacoes_threads[i].inicio = i;
        informacoes_threads[i].intervalo = quantidade_threads;
        informacoes_threads[i].largura = largura;
        informacoes_threads[i].altura = altura;
        informacoes_threads[i].max_iteracoes = max_iteracoes;
        informacoes_threads[i].imagem = imagem_pthreads2;

        if (pthread_create(&threads[i], NULL, calcular_pthreads2, &informacoes_threads[i]) != 0){
            fprintf(stderr, "erro. não foi possível criar a thread %d.\n", i);
            for (int j = 0; j < i; j++){
                pthread_join(threads[j], NULL);
            }
            fclose(arquivo_serial);
            fclose(arquivo_openmp);
            fclose(arquivo_pthreads1);
            fclose(arquivo_pthreads2);
            fclose(arquivo_tempo);
            free(imagem_openmp);
            free(imagem_pthreads1);
            free(imagem_pthreads2);
            free(threads);
            free(informacoes_threads);
            return 1;
        }
    }

    for (int i = 0; i < quantidade_threads; i++){
        if (pthread_join(threads[i], NULL) != 0){
            fprintf(stderr, "erro. não foi possível esperar a thread %d.\n", i);
            fclose(arquivo_serial);
            fclose(arquivo_openmp);
            fclose(arquivo_pthreads1);
            fclose(arquivo_pthreads2);
            fclose(arquivo_tempo);
            free(imagem_openmp);
            free(imagem_pthreads1);
            free(imagem_pthreads2);
            free(threads);
            free(informacoes_threads);
            return 1;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &fim);

    for (y = 0; y < altura; y++){
        for (x = 0; x < largura; x++){
            fprintf(arquivo_pthreads2, "%d ", imagem_pthreads2[y * largura + x]);
        }
        fprintf(arquivo_pthreads2, "\n");
    }

    fprintf(arquivo_tempo, "pthreads2: %.8f segundos\n", tempo_decorrido(inicio, fim));

    fclose(arquivo_serial);
    fclose(arquivo_openmp);
    fclose(arquivo_pthreads1);
    fclose(arquivo_pthreads2);
    fclose(arquivo_tempo);
    free(imagem_openmp);
    free(imagem_pthreads1);
    free(imagem_pthreads2);
    free(threads);
    free(informacoes_threads);

    return 0;
}