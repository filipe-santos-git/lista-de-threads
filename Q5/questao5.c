/*===============================================
 * Autores: Edivaldo Ambrósio da Silva Filho (easf), Filipe Santos Chaves (fsc5), Pablo Nunes de Oliveira (pno)
 * Disciplina: Sistemas Operacionais
 * Data: 13/04/26 (data de entrega)
 *
 * Descrição:
 * Questão 5 - Verificação de quadrados mágicos utilizando pthreads.
 * 
 * Modelo adotado:
 * - Cada linha, coluna e diagonal é verificada por uma thread separada.
 * - Mutex é utilizado para garantir exclusão mútua ao atualizar o status de "eh_magico".
 * - O programa compara a soma de cada linha, coluna e diagonal com a soma da primeira linha para determinar se a matriz é um quadrado mágico.
 *
 * Compilação:
 * gcc -pthread -o prog arquivo.c
 *==============================================*/

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

        if(pthread_create(&threads[i], NULL, rotina, linha) != 0){
            perror("Erro ao criar a thread da linha");
            return 2;
        }
    }

    // Separando as colunas e enviando para rotina
    for (int i = 0; i < lado; i++) {
        int* coluna = malloc(lado * sizeof(int));

        for (int j = 0; j < lado; j++) {
            coluna[j] = matriz[j][i];
        }

        if(pthread_create(&threads[lado + i], NULL, rotina, coluna) != 0){
            perror("Erro ao criar a thread da coluna");
            return 2;
        }
    }

    // Separando a diagonal principal e enviando para rotina
    int* diagonal1 = malloc(lado * sizeof(int));
    for (int i = 0; i < lado; i++) {
        diagonal1[i] = matriz[i][i];
    }
    if(pthread_create(&threads[2 * lado], NULL, rotina, diagonal1) != 0){
        perror("Erro ao criar a thread da diagonal principal");
        return 2;
    }

    // Separando a diagonal secundária e enviando para rotina
    int* diagonal2 = malloc(lado * sizeof(int));
    for (int i = 0; i < lado; i++) {
        diagonal2[i] = matriz[i][lado - 1 - i];
    }

    if (pthread_create(&threads[2 * lado + 1], NULL, rotina, diagonal2) != 0){
        perror("Erro ao criar a thread da diagonal secundária");
        return 2;
    }

    // garantindo a conclusão das threads
    for (int i = 0; i < (2 * lado) + 2; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("Erro em join na thread");
            return 3;
        }
    }

    // Imprimindo o resultado
    if (eh_magico) {
        printf("A matriz é um quadrado mágico!\n");
    } else {
        printf("A matriz não é um quadrado mágico.\n");
    }
    pthread_mutex_destroy(&mutex);
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