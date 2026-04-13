/*===============================================
 * Autores: Edivaldo Ambrósio da Silva Filho (easf), Filipe Santos Chaves (fsc5), Pablo Nunes de Oliveira (pno)
 * Disciplina: Sistemas Operacionais
 * Data: 13/04/26 (data de entrega)
 *
 * Descrição:
 * Questão 3 -
 *
 * Modelo adotado:
 *
 * Compilação:
 * gcc -pthread -o prog arquivo.c
 *==============================================*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// Número de posições da tabela hash
#define num_chave 10

// Nó da lista encadeada usada em cada posição da tabela
typedef struct Node {
    int value;
    struct Node* next;
} Node;

// Estrutura do hashset.
// Cada posição da tabela possui sua própria lista encadeada
// e seu próprio mutex, permitindo exclusão mútua refinada.
typedef struct {
    Node* chave[num_chave];
    pthread_mutex_t locks[num_chave];
} HashSet;

// Instância global da tabela hash
HashSet hashmap;

// Função hash básica.
// Define em qual bucket o valor será armazenado.
int hash_function(int value) {
    return abs(value) % num_chave;
}

// Função de inserção concorrente.
// Cada thread trava apenas o bucket correspondente ao valor,
// permitindo que outras threads acessem buckets diferentes ao mesmo tempo.
void inserir(HashSet* set, int value) {
    int index = hash_function(value);

    pthread_mutex_lock(&set->locks[index]);

    Node* node = set->chave[index];

    // Verifica se o valor já existe no bucket
    while (node != NULL) {
        if (node->value == value) {
            pthread_mutex_unlock(&set->locks[index]);
            return;
        }
        node = node->next;
    }

    // Se o valor não existir, cria um novo nó e insere no início da lista
    Node* new_node = (Node*)malloc(sizeof(Node));
    new_node->value = value;
    new_node->next = set->chave[index];
    set->chave[index] = new_node;

    pthread_mutex_unlock(&set->locks[index]);
}

// Busca segura.
// Retorna 1 se o valor for encontrado e 0 caso contrário.
int search(HashSet* set, int value) {
    int index = hash_function(value);
    int achou = 0;

    pthread_mutex_lock(&set->locks[index]);

    Node* node = set->chave[index];
    while (node != NULL) {
        if (node->value == value) {
            achou = 1;
            break;
        }
        node = node->next;
    }

    pthread_mutex_unlock(&set->locks[index]);

    return achou;
}

// Libera todos os nós alocados dinamicamente e destrói os mutexes.
// A limpeza é feita bucket por bucket.
void limpeza(HashSet* set) {
    for (int i = 0; i < num_chave; i++) {
        pthread_mutex_lock(&set->locks[i]);

        Node* node = set->chave[i];
        while (node != NULL) {
            Node* temp = node;
            node = node->next;
            free(temp);
        }
        set->chave[i] = NULL;

        pthread_mutex_unlock(&set->locks[i]);
        pthread_mutex_destroy(&set->locks[i]);
    }
}

// Vetor com os valores que serão inseridos concorrentemente no hashset
int dados_para_inserir[15] = {12, 45, 87, 23, 56, 89, 34, 67, 90, 11, 44, 77, 22, 55, 88};

// Rotina executada por cada thread.
// Cada thread insere 5 valores, a partir de um índice inicial diferente.
void* thread_trabalhadora(void* arg) {
    int indice_inicio = *((int*)arg);

    for (int i = 0; i < 5; i++) {
        int valor = dados_para_inserir[indice_inicio + i];
        int indice = hash_function(valor);

        printf("Inserindo valor: %d | indice: %d\n", valor, indice);
        inserir(&hashmap, valor);
    }

    pthread_exit(NULL);
    return NULL;
}

int main() {
    // Inicializa o hashmap e os mutexes de cada bucket
    for(int i = 0; i < num_chave; i++) {
        hashmap.chave[i] = NULL;
        pthread_mutex_init(&hashmap.locks[i], NULL);
    }

    pthread_t threads[3];
    int indices_inicio[3] = {0, 5, 10};

    printf("-INICIANDO INSERCOES CONCORRENTES-\n");

    // Cria as 3 threads trabalhadoras
    for (int i = 0; i < 3; i++) {
        pthread_create(&threads[i], NULL, thread_trabalhadora, &indices_inicio[i]);
    }

    // Aguarda a conclusão de todas as threads
    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }

    // Testes de busca no conjunto
    printf("\n-BUSCANDO VALORES-\n");

    if (search(&hashmap, 67)) {
        printf("O valor 67 esta no conjunto.\n");
    } else {
        printf("Erro. O valor 67 n foi encontrado.\n");
    }

    if (search(&hashmap, 13)) {
        printf("O valor 13 esta no conjunto.\n");
    } else {
        printf("Erro. O valor 13 n foi encontrado.\n");
    }

    limpeza(&hashmap);
    return 0;
}