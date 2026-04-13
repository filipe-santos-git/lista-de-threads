/*===============================================
 * Autores: Edivaldo Ambrósio da Silva Filho (easf), Filipe Santos Chaves (fsc5), Pablo Nunes de Oliveira (pno)
 * Disciplina: Sistemas Operacionais
 * Data: 13/04/26 (data de entrega)
 *
 * Descrição:
 * Questão 2 -Implementação de um sistema concorrente utilizando pthreads
 * para simular atualização de um painel (display) com múltiplas linhas, 
 * a partir da leitura de arquivos de entrada.
 *
 * Modelo adotado:
 * Produtor-consumidor, onde:
 * - rotina_read: produz requisições
 * - rotina_print: consome e exibe no display
 *
 * Compilação:
 * gcc -pthread -o prog arquivo.c
 *==============================================*/


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

// Códigos ANSI para cores no terminal
#define RESET "\033[0m"
#define CLEAR "\033[2J"
#define HOME  "\033[H"
#define BG_RED     "\033[41m"
#define BG_YELLOW  "\033[43m"
#define BG_BLUE    "\033[44m"
#define BG_MAGENTA "\033[45m"
#define BG_GREEN   "\033[42m"
#define BG_CYAN    "\033[46m"

// Cores para cada linha do metrô

char* cores[7] = {
    BG_RED,
    BG_GREEN,
    BG_YELLOW,
    BG_BLUE,
    BG_MAGENTA,
    BG_CYAN,
    "\033[47m"   // branco
};

/*======================================================================
 * Número de threads de impressão.
 *
 * Decidimos utilizar 20 threads para garantir velocidade e fluidez na atualização.
 *
 * Além disso, essa escolha também evidencia que não há conflitos de acesso ao display,
 * mesmo quando várias threads recebem informações para uma mesma linha.
 *======================================================================*/
#define num_threads 20

// Lista de arquivos a serem lidos
#define total_arquivos 3
char *arquivos[] = {"Att", "Att2", "Att3"};

// Índice de iteração sobre a lista de arquivos a serem lidos
int proximo_arquivo = 0;

// Mutexes para controle de acesso às linhas do display, à fila de requests, acesso ao terminal e à fila de arquivos

pthread_mutex_t mutex_display[7];
pthread_mutex_t mutex_fila = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_queue = PTHREAD_MUTEX_INITIALIZER;
/*===============================================================
 * Esse mutex_tela não é responsável pela exclusão mútua por linha,
 * pois isso já é garantido por mutex_display.
 *
 * Sua função é limitar a região crítica do terminal somente ao
 * momento da impressão. Assim, o terminal é liberado logo após
 * o printf, enquanto a linha continua bloqueada por 2 segundos
 * por meio de mutex_display.
 *
 * Dessa forma, outras threads podem continuar atualizando outras
 * linhas do display sem ficarem presas durante o sleep(2).
 *===============================================================*/
pthread_mutex_t mutex_tela = PTHREAD_MUTEX_INITIALIZER;
//variavel de condição para sinalizar a presença de novos requests na fila e evitar espera ocupada.
pthread_cond_t cond_fila = PTHREAD_COND_INITIALIZER;


// Estrutura para armazenar os dados de cada request
typedef struct {
    int linha;
    char codigo[7];
    char cidade[15];
    char horario[6];
} request;

// declaração das funções

char* get_next_file();
void adicionar_request(request novo);
void* rotina_read(void* arg);
void* rotina_print(void* arg);

// Fila de requests
request *fila;

// Variáveis para controle da fila 

int tam_fila = 0;
int inicio_fila = 0;
int fim_fila = 0;
int leitura_concluida = 0;

int main(){
    //clear do terminal para garantir que o painel seja exibido corretamente
    printf(CLEAR HOME);
    fflush(stdout);

    // Inicialização da fila de requests
    /*=================================================================
    * Decidimos usar alocação dinâmica para a fila,
    * pois dentro da função adicionar_request podemos aumentar a quantidade
    * de espaços na memória utilizados, a depender da demanda.
    *=================================================================*/
fila = (request *)malloc(sizeof(request) * 5);
    fila = (request *)malloc(sizeof(request) * 5);
    if (fila == NULL) {
        perror("Erro ao alocar memória para a fila");
        return 1;
    }

    // Decidimos que uma thread para ler os arquivos seria suficiente para garantir a fluidez do programa.
    // O programa segue o modelo produtor-consumidor, em que a thread de leitura é a produtora
    // e as threads de impressão são as consumidoras.
    pthread_t leitura;
    pthread_t impressao[num_threads];

    // Inicialização dos mutexes para cada linha
    for (int i = 0; i < 7; i++){pthread_mutex_init(&mutex_display[i], NULL);}
    // Criação da thread de leitura
    if(pthread_create(&leitura, NULL, rotina_read, NULL) != 0){
        perror("Erro ao criar a thread de leitura");
        return 2;
    }

    // Criação das threads de impressão
    
    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(&impressao[i], NULL, rotina_print, NULL) != 0) {
            perror("Erro ao criar a thread de impressão");
            return 3;
        }
    }

    // Aguardar a conclusão das threads de leitura e impressão em conjunto

    if(pthread_join(leitura, NULL) != 0) {
        perror("Erro ao aguardar a thread de leitura");
        return 1;
    }
    for (int i = 0; i < num_threads; i++) {
        if (pthread_join(impressao[i], NULL) != 0) {
            perror("Erro ao aguardar a thread de impressão");
            return 2;
        }
    }

    // Liberação dos recursos e finalização do programa

    // Esse print é responsável por pular uma linha abaixo do painel,
    // para que a mensagem de finalização do programa não sobrescreva o display.
    printf("\033[8;1H");   // vai pra linha abaixo do painel (7 + 1)
    printf("\033[0m\n");   // reseta cor e pula linha
    fflush(stdout);
    // Liberação da memória alocada para a fila
    free(fila);
    // Destruição dos mutexes
    for (int i = 0; i < 7; i++){pthread_mutex_destroy(&mutex_display[i]);}
    pthread_mutex_destroy(&mutex_fila);
    pthread_mutex_destroy(&mutex_queue);   
    pthread_mutex_destroy(&mutex_tela);
    pthread_cond_destroy(&cond_fila);
    return 0;
}

//Rotina de leitura dos arquivos e adição dos requests na fila

void* rotina_read(void* arg){
    
    while (1) {
        /*======================================================================
        * Obtém o próximo arquivo a ser lido.
        *
        * Como utilizamos somente uma thread para leitura, não há risco atual de
        * race condition no acesso aos arquivos. Mesmo assim, para garantir maior
        * segurança e escalabilidade do programa, utilizamos um mutex para controlar
        * o acesso à lista de arquivos.
        *======================================================================*/
        char *arquivo = get_next_file();
        //caso não haja mais arquivos para ler, a função get_next_file retorna NULL, e o loop é interrompido.
        if (arquivo == NULL) {
            break;
        }

        FILE *file = fopen(arquivo, "r");
        if (file == NULL) {
            perror("Erro ao abrir o arquivo");
            continue;
        }
        //leitura do arquivo e adição dos requests na fila
        request r;
        while (fscanf(file, "%d %6s %14s %5s", &r.linha, r.codigo, r.cidade, r.horario) == 4) {
             //adiciona o request lido na fila, que garante a exclusão mútua no acesso à fila.
            adicionar_request(r);
        }

        fclose(file);
        

    }
    // leitura_concluida indica que a leitura de todos os arquivos foi concluída.
    /* ===================================================
    * Sua principal função é impedir que as threads de impressão
    * encerrem prematuramente caso a fila esteja vazia, mas a leitura
    * ainda não tenha terminado.
    *=====================================================*/
    pthread_mutex_lock(&mutex_queue);
    leitura_concluida = 1;
    pthread_cond_broadcast(&cond_fila);
    pthread_mutex_unlock(&mutex_queue);

    return NULL;
}

// Função para obter o próximo arquivo a ser lido, garantindo que apenas uma thread acesse a fila de arquivos por vez

char* get_next_file() {
    char* file_name = NULL;

    pthread_mutex_lock(&mutex_fila);
    if (proximo_arquivo < total_arquivos) {
        file_name = arquivos[proximo_arquivo];
        proximo_arquivo++; // Incrementa para que a próxima thread pegue o seguinte
    }
    pthread_mutex_unlock(&mutex_fila);
    return file_name; // Retorna NULL se não houver mais arquivos
}

// Adiciona um novo request à fila compartilhada, com redimensionamento dinâmico se necessário.
void adicionar_request(request novo) { 
    pthread_mutex_lock(&mutex_queue);

    request *tmp = realloc(fila, sizeof(request) * (tam_fila + 1));
    if (tmp == NULL) {
        perror("Erro no realloc da fila");
        pthread_mutex_unlock(&mutex_queue);
        return;
    }

    fila = tmp;
    fila[tam_fila] = novo;
    tam_fila++;

    pthread_cond_signal(&cond_fila);
    pthread_mutex_unlock(&mutex_queue);
}

void* rotina_print(void* arg){
    while (1){
        pthread_mutex_lock(&mutex_queue);
        // Enquanto a fila estiver vazia e a leitura ainda não tiver sido concluída,
        // a thread permanece bloqueada sem espera ocupada.
        while (tam_fila == 0 && !leitura_concluida){
            pthread_cond_wait(&cond_fila, &mutex_queue);
        }
         // Se a fila estiver vazia e a leitura já tiver sido concluída, a thread pode encerrar.
        if (tam_fila == 0 && leitura_concluida){
            pthread_mutex_unlock(&mutex_queue);
            return NULL;
        }
        // Remove da fila o primeiro request disponível para processamento.
        request req = fila[0];
        for (int i = 1; i < tam_fila; i++){
            fila[i - 1] = fila[i];
        }
        tam_fila--;

        pthread_mutex_unlock(&mutex_queue);

        int linha = req.linha - 1;
        // Garante exclusão mútua na linha correspondente do display.
        pthread_mutex_lock(&mutex_display[linha]);
        // Garante exclusão mútua no uso do terminal e dos códigos ANSI.
        pthread_mutex_lock(&mutex_tela);

        printf("\033[%d;1H", req.linha);
        printf("\033[2K");
        printf("%s%-7s %-15s %-6s%s",
               cores[linha],
               req.codigo,
               req.cidade,
               req.horario,
               RESET);
        fflush(stdout);

        pthread_mutex_unlock(&mutex_tela);
         // Mantém a linha ocupada por 2 segundos, conforme exigido no enunciado.
        sleep(2);

        pthread_mutex_unlock(&mutex_display[linha]);
    }
}