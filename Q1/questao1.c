/*===============================================
 * Autores: Edivaldo Ambrósio da Silva Filho (easf), Filipe Santos Chaves (fsc5), Pablo Nunes de Oliveira (pno)
 * Disciplina: Sistemas Operacionais
 * Data: 13/04/26 (data de entrega)
 *
 * Descrição: 
 * Questão 1 - Conversão de imagem .ppm para tons de cinza utilizando threads (pthreads).
 * O programa lê uma imagem no formato .ppm, converte cada pixel para tons de cinza utilizando a fórmula disponibilizada, e salva a nova imagem em um arquivo .ppm de saída. Cada pixel é processado por uma thread separada, e o programa imprime os valores dos pixels processados no console.
 * implementamos utilizando uma thread para cada pixel da imagem, pois priorizamos a rapidez na execução.
 * 
 *
 * Compilação:
 * gcc -pthread -o prog arquivo.c
 *==============================================*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct {
    int r, g, b;
} Pixel;

void* rotina(void* arg);

int main() {
    //Abrindo o arquivo ppm no modo leitura
    FILE *f = fopen("img.ppm", "r");
    //Conferindo se deu erro
    if (!f) {
        printf("Erro ao abrir arquivo\n");
        return 1;
    }
    //Lendo o tipo, linhas, colunas e max do arquivo ppm
    char tipo[3];
    int linhas, colunas, max;
    fscanf(f, "%2s", tipo);
    fscanf(f, "%d %d",  &colunas, &linhas);
    fscanf(f, "%d", &max);
    //alocando memória para o vetor que recebe as informações da imagem ppm
    Pixel *img = malloc(linhas * colunas * sizeof(Pixel));
    //atribuindo os valores de r, g e b para o vetor da struct img
    for (int i = 0; i < linhas * colunas; i++) {
        fscanf(f, "%d %d %d", &img[i].r, &img[i].g, &img[i].b);
    }
    fclose(f);
 
    //Parte das threads

    //Definindo quantidade de threads
    int num_threads = linhas * colunas;
    //Criando um vetor de threads e 
    pthread_t threads[num_threads];
    //Preenchendo o vetor e mandando argumentos para a rotina
    for(int i = 0 ; i < num_threads; i++){
        if (pthread_create(&threads[i], NULL, &rotina, &img[i]) != 0){
            return i;
        }
    }
    //Desabilitando as threads
    for(int i = 0 ; i < num_threads; i++){
        if (pthread_join(threads[i], NULL) != 0){
            return i;
        }
    }

    //Abrindo o arquivo ppm no modo de escrita
    FILE *f_out = fopen("img_gray.ppm", "w");
    //Escrevendo no arquivo ppm
    fprintf(f_out, "%s\n%d %d\n%d\n", tipo, colunas, linhas, max);
    for (int i = 0; i < linhas * colunas; i++) {
        fprintf(f_out, "%d %d %d\n", img[i].r, img[i].g, img[i].b);
    }
    //Fechando o f_out e liberando a imagem
    fclose(f_out);    
    free(img);

    return 0;
}

void* rotina(void* arg){
    //transformando um ponteiro de volta para o tipo pixel
    Pixel *img = (Pixel*)arg;
    //Convertendo
    int gray = (int)(0.30 * img->r + 0.59 * img->g + 0.11 * img->b);
    img->r = gray;
    img->g = gray;
    img->b = gray;
    //Printando os novos valores
    printf("Pixel processado: R=%d, G=%d, B=%d\n", img->r, img->g, img->b);
    
    return NULL;
}
