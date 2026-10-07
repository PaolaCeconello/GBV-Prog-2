#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include "gbv.h"
#include "util.h"

//Lê da biblioteca os dados do vetor
int load_vector(Library *lib)
{
    if (lib == NULL)
        return(1);
    
    fseek(lib-> archive, lib-> superblock-> offset, SEEK_SET);
    
    lib-> vector = realloc(lib->vector, sizeof(Document*)*lib-> superblock-> count);
        
    if (lib-> vector == NULL)
    {    
        fclose(lib-> archive);
        return(1);
    }
    
    for(int i = 0; i < lib-> superblock-> count; i++)
    {    
        lib-> vector[i] = malloc(sizeof(Document));
        if(lib-> vector[i] == NULL)
            return(1);
    }
        
    for(int i = 0; i < lib-> superblock-> count; i++)
    {
       if (fread(lib-> vector[i], sizeof(Document), 1, lib-> archive) != 1)
        {
            fclose(lib-> archive);
            return(1);
        }
    }
    
    return(0);
}

//Escreve os novos dados do vetor na biblioteca
int update_vector(Library *lib)
{
    if (lib == NULL)
        return(1);
    
    if (fseek(lib-> archive, lib-> superblock-> offset, SEEK_SET) != 0)
    {
        fclose(lib-> archive);
        return(1);
    }
    
    for(int i = 0; i < lib-> superblock-> count; i++)
    {
        if (fwrite(lib-> vector[i], sizeof(Document), 1, lib-> archive) != 1)
        {
            fclose(lib-> archive);
            return(1);
        }
    }
    
    return(0);
}

//Lê da biblioteca os dados do superbloco
int load_superblock(Library *lib)
{
    if (lib == NULL)
        return(1);
    
    if (fseek(lib-> archive, 0, SEEK_SET) != 0)
    {
        fclose(lib-> archive);
        return(1);
    }
    
    if (fread(lib-> superblock, sizeof(Superblock), 1, lib-> archive) != 1)
        {
            fclose(lib-> archive);
            return(1);
        }

    return(0);
}

//Escreve na biblioteca os dados do superbloco
int update_superblock(Library *lib)
{
    if (lib == NULL)
        return(1);
    
    if (fseek(lib-> archive, 0, SEEK_SET) != 0)
    {
        fclose(lib-> archive);
        return(1);
    }
    
    if (fwrite(lib-> superblock, sizeof(Superblock), 1, lib-> archive) != 1)
    {
        fclose(lib-> archive);
        return(1);
    }

    return(0);
}

int gbv_create(Library *lib, const char *filename)
{
    if (lib == NULL)
        return(1);    
    
    lib-> archive = fopen(filename, "wb");
    
    if (lib-> archive == NULL)
        return(1);
    
    lib-> superblock-> offset = sizeof(Superblock);
    lib-> superblock-> count = 0;
   
    if (fwrite(lib-> superblock, sizeof(Superblock), 1, lib-> archive) != 1)
    {    
        fclose(lib-> archive);
        return(1);
    }
    
    return(0);
}

int gbv_open(Library *lib, const char *filename)
{
    FILE* fileCheck;
    
    fileCheck = fopen(filename,"r+b");
    
    //Cria uma biblioteca, caso não consiga abrir
    if (fileCheck == NULL )
    {
        lib-> superblock = calloc(1, sizeof(Superblock));
        lib-> vector = calloc(1, sizeof(Document));
    
        if (gbv_create(lib, filename) != 0)
            return(1);
    }
    //Abre o arquivo já existente e lê da biblioteca os dados do superbloco e do vetor
    else
    {
        lib-> superblock = calloc(1, sizeof(Superblock));
        lib-> vector = NULL;
        lib-> archive = fileCheck;
        
        if (load_superblock(lib) != 0)
            return(1);
        
        if (load_vector(lib) != 0)
            return(1);
    }
    
    return(0);
}

int gbv_add(Library *lib, const char *archive, const char *docname)
{
    FILE* newDocname;
    newDocname = fopen(docname, "rb");
    
    //Caso a biblioteca esteja fechada, reabre-a
    if(lib-> archive == NULL)
        lib-> archive = fopen(archive, "r+b");
  
    if (newDocname == NULL || lib-> archive == NULL)
        return(1);
    
    //Fecha o programa caso tente inserir a biblioteca
    if (strcmp(archive, docname) == 0)
    {
        fclose(newDocname);
        fclose(lib-> archive);
        return(1);
    }
   
    Document *newDoc;
  
    struct stat statbuf;
    
    newDoc = malloc(sizeof(Document)); 
    
    if (newDoc == NULL)
        return(1);
    
    if (stat(docname,&statbuf) != 0)
    {
        free(newDoc);
        fclose(newDocname);
        fclose(lib-> archive);
        return(1);
    }
    
    //Configura os metadados do no arquivo inserido
    strncpy(newDoc-> name, docname, MAX_NAME - 1);
    newDoc-> name[MAX_NAME - 1] = '\0';
    newDoc-> size = statbuf.st_size;
    newDoc-> date = time(NULL);
    newDoc-> offset = lib-> superblock-> offset;
        
    
    //Confere se o arquivo a ser inserido já existe dentro da biblioteca
    int if_replace = 0;
    int replaced;
    
    if (lib-> superblock-> count > 0)
    {
        int i;
        int n;
        int dif;

        i = 0;
        while (i < lib-> superblock-> count)
        {
            //Percorre os arquivos buscando repetições pelo nome
            if (strcmp(lib-> vector[i]-> name, docname) == 0)
            {
                if_replace = 1;
                
                dif = (newDoc-> size - lib-> vector[i]-> size);
                
                //Caso o novo arquivo seja maior do que o já existente na biblioteca
                if(newDoc-> size > lib-> vector[i]-> size)  
                {    
                    for (n = (lib-> superblock-> count) -1; n > i; n--)
                    {
                        char buffer [lib-> vector[n]-> size];

                        if (fseek(lib-> archive, lib-> vector[n]-> offset, SEEK_SET) != 0)
                            return(1);
    
                        
                        if (fread(buffer, 1, lib-> vector[n]-> size, lib-> archive) != lib-> vector[n]-> size)
                        {
                            free(newDoc);
                            fclose(newDocname);
                            fclose(lib-> archive);
                            return(1);
                        }
    
                        lib-> vector[n]-> offset = lib-> vector[n]-> offset + dif;

                        if (fseek(lib-> archive, lib-> vector[n]-> offset, SEEK_SET) != 0)
                            return(1);
    
                        
                        if (fwrite(buffer, 1, lib-> vector[n]-> size, lib-> archive) != lib-> vector[n]-> size)
                        {
                            free(newDoc);
                            fclose(newDocname);
                            fclose(lib-> archive);
                            return(1);
                        }
                    }
                }
                //Caso o novo arquivo seja menor do que o já existente na biblioteca
                else 
                {
                    for (n = i + 1; n < lib-> superblock-> count; n++)
                    {
                         char buffer [lib-> vector[n]-> size];

                        if (fseek(lib-> archive, lib-> vector[n]-> offset, SEEK_SET) != 0)
                            return(1);
    
                        
                        if (fread(buffer, 1, lib-> vector[n]-> size, lib-> archive) != lib-> vector[n]-> size)
                        {
                            free(newDoc);
                            fclose(newDocname);
                            fclose(lib-> archive);
                            return(1);
                        }
    
                        lib-> vector[n]-> offset = lib-> vector[n]-> offset + dif;

                        if (fseek(lib-> archive, lib-> vector[n]-> offset, SEEK_SET) != 0)
                            return(1);
    
                        
                        if (fwrite(buffer, 1, lib-> vector[n]-> size, lib-> archive) != lib-> vector[n]-> size)
                        {
                            free(newDoc);
                            fclose(newDocname);
                            fclose(lib-> archive);
                            return(1);
                        }
                    }
                }
                    
                //Atualiza os metadados no novo arquivo e o superbloco
                newDoc-> offset = lib-> vector[i]-> offset;
                lib-> superblock-> offset = lib-> superblock-> offset + dif;
                
                replaced = i;
                
                lib-> superblock-> count--;
            }
                
          i++;      
        }
    }
    
    //Lê e escreve os arquivos em blocos de tamanho BUFFER_SIZE
    char buffer[BUFFER_SIZE];
    int remainingBytes;
    int toBeRead;
    
    remainingBytes = newDoc-> size;
    
    if (fseek(lib-> archive, newDoc-> offset, SEEK_SET) != 0)
            return(1);
    
    while (remainingBytes > 0)
    {
        if (remainingBytes < BUFFER_SIZE)
            toBeRead = remainingBytes;
        else
            toBeRead = BUFFER_SIZE;
        
        if (fread(buffer, 1, toBeRead, newDocname) != toBeRead)
        {
            free(newDoc);
            fclose(newDocname);
            fclose(lib-> archive);
            return(1);
        }
    
        if (fwrite(buffer, 1, toBeRead, lib-> archive) != toBeRead)
        {
            free(newDoc);
            fclose(newDocname);
            fclose(lib-> archive);
            return(1);
        }
    
        remainingBytes = remainingBytes - toBeRead;
    }
    
    //Caso o novo arquivo não seja uma subtituição, atualiza o offset do superbloco
    if(if_replace == 0)
        lib-> superblock-> offset = lib-> superblock-> offset + newDoc-> size;
    
    lib-> superblock-> count++;
    
    //Atualiza o superbloco
    if (update_superblock(lib) != 0)
        return(1);
    
    int count = lib-> superblock-> count;
    
    lib-> vector = realloc(lib-> vector, sizeof(Document*)*count);
    
    if (lib-> vector == NULL)
    {
        free(newDoc);
        fclose(newDocname);
        fclose(lib-> archive);
        return(1);
    }
    
    //Caso o arquivo não seja uma substituição, coloca os metadados ao final do vetor
    if(if_replace == 0)
        lib-> vector[count-1] = newDoc;
    
    //Caso o arquivo seja uma substituição, coloca os metadados no lugar dos metadados do arquivo substituido
    else 
    {
        free(lib-> vector[replaced]);
        lib-> vector[replaced] = newDoc;
    }
        
    //Atualiza o vetor
    if (update_vector(lib) != 0)
        return(1);
    
    fclose(newDocname);
    fclose(lib-> archive);
    lib-> archive = NULL;
    
    return(0);
}

int gbv_remove(Library *lib, const char *docname)
{
    int i;
    int deleted;
    int count;
    
    count = lib-> superblock-> count;
    
    //Procura o indicie do vetor do arquivo a ser removido
    i = 0;
    while (i < count && strcmp(lib-> vector[i]-> name, docname) != 0)
    {
        i++;
    }
    
    if (i == count)
        return(1);
    
    deleted = i;

    free(lib-> vector[deleted]);
    
    //Remove do vetor de metadados os metadados do arquivo
    for (i = deleted; i < count-1; i++)
    {
       lib-> vector[i] = lib-> vector[i+1];
    }
    
    lib-> vector[lib-> superblock-> count-1] = NULL;
    lib-> superblock-> count--;
  
    //Atualiza o superbloco e o vetor
    if (update_superblock(lib) != 0)
        return(1);
            
    if (update_vector(lib) != 0)
        return(1);
    
    fclose(lib-> archive);
    return(0);
}

int gbv_list(Library *lib)
{
    int i;
    int count;

    count = lib-> superblock-> count;
    
    if (fseek(lib-> archive,lib-> superblock-> offset,SEEK_SET) != 0)
    {
        fclose(lib-> archive);
        return(1);
    }
    
    //Percorre o vetor de metadados imprimindo as informações
    for (i = 0; i < count; i++)
    {
        printf("Nome do arquivo:\t");
        fputs(lib-> vector[i]-> name,stdout);
        printf("\n");
        
        printf("Tamanho do arquivo:\t");
        printf("%ld", lib-> vector[i]-> size);
        printf("\n");

        printf("Data:\t");
        printf("\t");
        printf("\t");
        printf("%s",ctime(&lib-> vector[i]-> date));
        printf("\n");

        printf("Offset do arquivo:\t");
        printf("%ld", lib-> vector[i]-> offset);
        printf("\n");           
        printf("---------------\n"); 
    }

    fclose(lib->archive);
    return(0);
}

int gbv_view(const Library *lib, const char *docname)
{
    int i;
    int count;
    
    count = lib-> superblock-> count;
    
    //Procura o indice do vetor do arquivo a ser visualizado
    i = 0;
    while(i < count && strcmp(lib-> vector[i]-> name, docname) != 0)
        i++;
    
    if (i == count)
        return(1);
    
    int remainingBytes;
    int readBytes;
    int toBeRead;
    char buffer[BUFFER_SIZE];

    //Define as opções para percorrer o arquivo
    char option = ' ';
    char quit = 'q';
    char next = 'n';
    char prev = 'p';
    
    toBeRead = 0;
    readBytes = 0;
    remainingBytes = lib-> vector[i]-> size;
    
    while (option != quit)
    {
        //Define a quatidade de bytes a serem lidos no bloco atual
        if(remainingBytes < BUFFER_SIZE)
            toBeRead = remainingBytes;
        else 
            toBeRead = BUFFER_SIZE;

        if (fseek(lib-> archive, lib-> vector[i]-> offset + readBytes, SEEK_SET) != 0)
        {
            fclose(lib-> archive);
            return(1);
        }
        
        //Lê o bloco do arquivo e o escreve no arquivo de saida padrão
        if(fread(buffer, 1, toBeRead, lib-> archive) != toBeRead)
        {
            fclose(lib-> archive);
            return(1);
        }
        
        if(fwrite(buffer, 1, toBeRead, stdout) != toBeRead)
        {
            fclose(lib->archive);
            return(1);
        }
        
        printf("\n");
        printf("----------------------------------\n");
        printf("Escolha uma opção:\n");
        printf("n- next\n");
        printf("p- prev\n");
        printf("q- quit\n");
        
        scanf(" %c", &option);
        
        if (option == next)
        {
            //Caso o proximo bloco a ser lido ultrapasse o tamanho do arquivo
            if(readBytes + BUFFER_SIZE >= lib-> vector[i]-> size)
            {    
                printf("Fim do arquivo\n");
                printf("----------------------------------\n");
                printf("Escolha uma opção:\n");
                printf("p- prev\n");
                printf("q- quit\n");
                
                scanf(" %c", &option);
            }
            //Atualiza o numero de bytes a serem lidos e o numero de bytes que já foram lidos
            else 
            {
                remainingBytes = remainingBytes - toBeRead;
                readBytes = readBytes + toBeRead;
            }
        }
        
        else if (option == prev)
        {
            //Caso o proximo bloco a ser lido tenha dados de um arquivo anterior
            if(readBytes <= 0)
            {
                printf("Começo do arquivo\n");
                printf("----------------------------------\n");
                printf("Escolha uma opção:\n");
                printf("n- next\n");
                printf("q- quit\n");
                scanf(" %c", &option);
            }
            //Atualiza o tamanho dos  bytes a serem lidos e os bytes que já foram lidos
            else 
            {
                remainingBytes = remainingBytes + toBeRead;
                readBytes = readBytes - toBeRead;

                if(readBytes < 0)
                    readBytes = 0;
            }
        }
        
        else if (option != quit)
        {
            printf("Opção invalida\n");
            printf("----------------------------------\n");
            printf("Escolha uma opção:\n");
            printf("n- next\n");
            printf("p- prev\n");
            printf("q- quit\n");
            
            scanf(" %c", &option);
        }
    }
    
    fclose(lib->archive);
    
    return(0);
}
/*int gbv_order(Library *lib, const char *archive, const char *criteria)
{
    return(0);
}*/

void gbv_close (Library *lib)
{
   int i;

   //Libera a memoria dos indices do vetor de metadados 
   for(i = 0; i < lib-> superblock-> count; i++)
        free(lib-> vector[i]);

    //Libera a memoria do vetor e do superbloco
    free(lib-> vector);
    free(lib-> superblock);
    
    return;
}
