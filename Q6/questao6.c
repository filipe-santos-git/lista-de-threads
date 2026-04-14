/*===============================================
 * Autores: Edivaldo Ambrósio da Silva Filho (easf), Filipe Santos Chaves (fsc5), Pablo Nunes de Oliveira (pno)
 * Disciplina: Sistemas Operacionais
 * Data: 13/04/26 (data de entrega)
 *
 * Descrição:
 * Questão 6 - função mergesort concorrente
 *
 * Modelo adotado:
 * - Criação recursiva de threads para realocar cada posição do vetor concorrentemente
 *
 * Compilação:
 * gcc -pthread -o prog arquivo.c
 *==============================================*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// Tamanho do vetor que será ordenado
#define TAMANHO 8

// Vetor global que será manipulado pelas threads
int vetor[TAMANHO] = {38, 27, 43, 3, 9, 82, 10, 19};

// Struct usada para passar os limites da porção do vetor
// que cada thread deverá ordenar
typedef struct {
    int esquerda;
    int direita;
} ParametrosSort;

// Função merge padrão do mergesort.
// Recebe os limites esquerdo, meio e direito e intercala
// as duas metades já ordenadas no vetor original.
void merge(int esquerda, int meio, int direita) {
    int i, j, k;
    int n1 = meio - esquerda + 1;
    int n2 = direita - meio;

    // Vetores temporários para armazenar as duas metades
    int L[n1], R[n2];

    // Copia a metade esquerda para o vetor auxiliar L
    for (i = 0; i < n1; i++){
        L[i] = vetor[esquerda + i];
    }

    // Copia a metade direita para o vetor auxiliar R
    for (j = 0; j < n2; j++) {
        R[j] = vetor[meio + 1 + j];
    }

    // Intercala os dois vetores auxiliares de volta para o vetor original
    i = 0;
    j = 0;
    k = esquerda;

    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            vetor[k] = L[i];
            i++;
        } else {
            vetor[k] = R[j];
            j++;
        }
        k++;
    }

    // Copia os elementos restantes da metade esquerda, se houver
    while (i < n1) {
        vetor[k] = L[i];
        i++;
        k++;
    }

    // Copia os elementos restantes da metade direita, se houver
    while (j < n2) {
        vetor[k] = R[j];
        j++;
        k++;
    }
}

// Rotina executada pelas threads.
// Cada thread ordena recursivamente uma faixa do vetor.
void* sort_paralelo(void* arg) {
    // Desempacotando a struct com os limites da faixa
    ParametrosSort* params = (ParametrosSort*) arg;
    int esquerda = params->esquerda;
    int direita = params->direita;

    printf("Ordenando os indices de intervalo [%d, %d]\n", esquerda, direita);

    // Condição de parada: se a faixa tem apenas um elemento,
    // ela já está ordenada
    if (esquerda == direita) {
        pthread_exit(NULL);
        return NULL;
    }

    // Calcula o ponto médio da faixa
    int meio = esquerda + (direita - esquerda) / 2;

    // Prepara os parâmetros das duas metades
    ParametrosSort params_esq = {esquerda, meio};
    ParametrosSort params_dir = {meio + 1, direita};

    pthread_t thread_esq, thread_dir;

    // Cria duas novas threads:
    // uma para ordenar a metade esquerda
    // outra para ordenar a metade direita
    pthread_create(&thread_esq, NULL, sort_paralelo, &params_esq);
    pthread_create(&thread_dir, NULL, sort_paralelo, &params_dir);

    // Aguarda as duas metades terminarem antes de fazer o merge
    pthread_join(thread_esq, NULL);
    pthread_join(thread_dir, NULL);

    // Só realiza o merge quando as duas subpartes já estiverem ordenadas
    merge(esquerda, meio, direita);

    // Exibe a porção já ordenada após o merge
    printf("porcao ordenada dos indices de intervalo [%d, %d]: ", esquerda, direita);
    for(int i = esquerda; i <= direita; i++){
        printf("%d ", vetor[i]);
        if(i == direita){
            printf("\n");
        }
    }

    pthread_exit(NULL);
    return NULL;
}

int main() {
    printf("Vetor original: \n");
    for (int i = 0; i < TAMANHO; i++) {
        printf("%d ", vetor[i]);
    }

    printf("\n\n- Iniciando Mergesort paralelo -\n");

    // Criação da thread inicial.
    // A partir dela, o mergesort será executado recursivamente com novas threads.
    ParametrosSort params_iniciais = {0, TAMANHO - 1};
    pthread_t thread_principal;

    pthread_create(&thread_principal, NULL, sort_paralelo, &params_iniciais);
    pthread_join(thread_principal, NULL);

    // Após a thread principal da ordenação terminar,
    // a main volta a executar e imprime o vetor final
    printf("\n- Mergesort finalizado -\n");
    printf("Vetor final ordenado: \n");
    for (int i = 0; i < TAMANHO; i++) {
        printf("%d ", vetor[i]);
    }

    return 0;
}
