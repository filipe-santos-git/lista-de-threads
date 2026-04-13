/*===============================================
 * Autores: Edivaldo Ambrósio da Silva Filho (easf), Filipe Santos Chaves (fsc5), Pablo Nunes de Oliveira (pno)
 * Disciplina: Sistemas Operacionais
 * Data: 13/04/26 (data de entrega)
 *
 * Descrição:
 * Questão 4 - Simulação de um sistema de pedágio utilizando pthreads, mutexes e condições.
 * 
 * Modelo adotado:
 * - Cada carro é uma thread que tenta acessar uma cabine para pagamento.
 * - Mutexes são usados para garantir exclusão mútua no acesso às cabines.
 * - Condições são usadas para gerenciar a espera dos carros quando todas as cabines estão ocupadas.
 *
 * Compilação:
 * gcc -pthread -o prog arquivo.c
 *==============================================*/
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
// Definindo o número de carros e cabines
#define N_Carros 200
#define N_Cabines 10
// Mutex e condição para controle de acesso às cabines
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
// Variáveis para manipulação das cabines
int cabines_livres = N_Cabines;
int cabines_pos[N_Cabines] = {0};

//declaração de função
void* carro(void* arg);

int main(){
    // Criando as threads dos carros
    pthread_t carros[N_Carros];
    int ids[N_Carros];
    for(int i = 0; i < N_Carros; i++){
        ids[i] = i;
        if(pthread_create(&carros[i], NULL, carro, &ids[i]) != 0){
            perror("Erro ao criar a thread do carro");
            return 1;
        }
    }
    // garantindo a conclusão das threads dos carros
    for(int i = 0; i < N_Carros; i++){
        if(pthread_join(carros[i], NULL) != 0){
            perror("Erro ao aguardar a thread do carro");
            return 1;
        }
    }  
    // Liberação dos recursos e finalização do programa
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    return 0;
}
// Rotina para cada carro
void* carro(void* arg){
    // Obtendo o ID do carro a partir do argumento
    int id = *(int*)arg;
    // Variável para armazenar o ID da cabine que o carro irá usar,
    //inicializada como -1 para indicar que ainda não foi alocada.
    int id_cabine = -1;
    printf("Carro %d chegou no pedágio\n", id);
    // Tentando acessar uma cabine disponível, garantindo exclusão mútua e evitando espera ocupada.
    pthread_mutex_lock(&mutex);
    while(cabines_livres == 0){
        pthread_cond_wait(&cond, &mutex);
    }
    //procurar por uma cabine livre, marcar como ocupada e atualizar o número de cabines livres.
    for(int i = 0; i < N_Cabines; i++){
    if(cabines_pos[i] == 0){
        cabines_pos[i] = 1;
        id_cabine = i;
        cabines_livres--;
        break;
        }
    }
    pthread_mutex_unlock(&mutex);

    printf("Carro %d usando cabine %d\n", id, id_cabine + 1);
    sleep(1); // Simula o tempo de pagamento
    // Após o pagamento, o carro libera a cabine,
    // garantindo exclusão mútua e atualizando as variáveis de número de cabines e posições disponíveis.
    pthread_mutex_lock(&mutex);
    cabines_livres++;
    cabines_pos[id_cabine] = 0;
    // Sinaliza para uma thread que esteja esperando por uma cabine que agora há uma disponível.
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);

    printf("Carro %d terminou o pagamento e liberou cabine %d\n", id, id_cabine + 1);

    return NULL;
}