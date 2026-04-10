#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

//Definindo o tamanho do lado do quadrado que vamos utilizar
#define lado 4

//Matriz usada nos testes
int matriz[lado][lado] = {
    {16, 2, 3, 13},
    {5, 11, 10, 8},
    {9, 7, 6, 12},
    {4, 14, 15, 1}
};

//Iniciando o mutex
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

//Declarando a rotina
void* rotina(void* arg);

//Criando as variáveis de controle
int compare = 0;
int eh_magico = 1;

int main() {

    // Somando os valores de uma linha para usar como referência
    for (int a = 0; a < lado; a++) {
        compare += matriz[0][a];
    }

    pthread_t threads[(2 * lado) + 2];

    // Separando as linhas e enviando para rotina
    for (int i = 0; i < lado; i++) {
        int* linha = malloc(lado * sizeof(int));

        for (int j = 0; j < lado; j++) {
            linha[j] = matriz[i][j];
        }

        pthread_create(&threads[i], NULL, rotina, linha);
    }

    // Separando as colunas e enviando para rotina
    for (int i = 0; i < lado; i++) {
        int* coluna = malloc(lado * sizeof(int));

        for (int j = 0; j < lado; j++) {
            coluna[j] = matriz[j][i];
        }

        pthread_create(&threads[lado + i], NULL, rotina, coluna);
    }

    // Separando a diagonal principal e enviando para rotina
    int* diagonal1 = malloc(lado * sizeof(int));
    for (int i = 0; i < lado; i++) {
        diagonal1[i] = matriz[i][i];
    }
    if(pthread_create(&threads[2 * lado], NULL, rotina, diagonal1) != 0){

        return 11;
    }

    // Separando a diagonal secundária e enviando para rotina
    int* diagonal2 = malloc(lado * sizeof(int));
    for (int i = 0; i < lado; i++) {
        diagonal2[i] = matriz[i][lado - 1 - i];
    }

    if (pthread_create(&threads[2 * lado + 1], NULL, rotina, diagonal2) != 0){
        return 12;
    }

    // Liberando as threads
    for (int i = 0; i < (2 * lado) + 2; i++) {
        pthread_join(threads[i], NULL);
    }

    // Imprimindo o resultado
    if (eh_magico) {
        printf("A matriz é um quadrado mágico!\n");
    } else {
        printf("A matriz não é um quadrado mágico.\n");
    }

    return 0;
}

void* rotina(void* arg) {

    //transformando o argumento em um tipo int
    int* dados = (int*)arg;
    int sum = 0;

    //Somando
    for (int i = 0; i < lado; i++) {
        sum += dados[i];
    }

    //Conferindo se é mágico
    if (sum != compare) {
        pthread_mutex_lock(&mutex);
        eh_magico = 0;
        pthread_mutex_unlock(&mutex);
    }

    //Liberando os dados alocados 
    free(dados);

    return NULL;
}