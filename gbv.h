#ifndef GBV_H
#define GBV_H

#include <time.h>

#define MAX_NAME 256
#define BUFFER_SIZE 512   // tamanho fixo do buffer em bytes

// Estrutura de metadados de cada documento
typedef struct Document
{
    char name[MAX_NAME];   // nome do documento
    long size;             // tamanho em bytes
    time_t date;           // data de inserção
    long offset;           // posição no container
} Document;

typedef struct 
{
    long offset;
    int count;
} Superblock;

// Estrutura que representa a biblioteca (diretório em memória)
typedef struct 
{
    Document **vector;        // vetor dinâmico de documentos
    Superblock *superblock;
    FILE* archive;
                 // número de documentos
} Library;

// Funções que voce deve implementar em gbv.c

//Cria a bibilioteca e insere o superbloco na mesma
int gbv_create(Library *lib, const char *filename);

//Caso não exista, cria a biblioteca
//Caso já exista, abre a biblioteca e recupera o superbloco e o vetor de metadados
int gbv_open(Library *lib, const char *filename);

//Insere novos arquivos a biblioteca
//Caso o arquivo já exista, atualiza seu conteúdo
int gbv_add(Library *lib, const char *archive, const char *docname);

//Remove do vetor de medatadados os metadados de determinado arquivo
int gbv_remove(Library *lib, const char *docname);

//Lista os metadados dos arquivos presentes na biblioteca
int gbv_list(Library *lib);

//Vizualiza conteudo de determinado arquivo em blocos 
int gbv_view(const Library *lib, const char *docname);

//Não implementado
int gbv_order(Library *lib, const char *archive, const char *criteria);

//Libera a memória de cada
void gbv_close(Library *lib);

#endif

