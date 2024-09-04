#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

// um arquivo de texto com um float por linha, sz em bytes
// retorna quantidade de bytes lidos
unsigned LoadData(FILE *file, float *array, unsigned sz);

// Objeto para compartilhar entre a thread main e a thread de trabalho
typedef struct _asyncState_t
{
    pthread_t thread; //  para o objeto de thread da biblioteca
    FILE *file; // os argumentos originais da 
    float *data; // os argumentos originais d
    unsigned read; // o retorno original da
        unsigned sz;

    int error; // uma vel para informar algum erro para thread main
} asyncState_t;
// Begin fica com os argumentos
asyncState_t * BeginLoadData(FILE *file, float *array, unsigned sz);

// End com o retorno
unsigned EndLoadData(asyncState_t *state);


void * AsyncLoadData(void *ctx);

int main(int argc, char *argv[])
{
    float array[5] = {0};
    FILE *datafile = fopen("datafile.txt", "r+");
    if (datafile == NULL) {
        perror("Erro ao abrir o arquivo");
        return 1;
    }
    asyncState_t *as = BeginLoadData(datafile, array, sizeof(array));

    // … a main pode fazer outras tarefas enquanto o arquivo é lido...
    printf("Foram lidos %d bytes do arquivo.\n", EndLoadData(as));
    int i;
    for (i = 0; i < sizeof(array)/sizeof(float); i++)
    {
        printf("%d: %f\n", i, array[i]);
    }
    fclose(datafile);
    system("pause");
    return 0;
}

// Begin cria o objeto compartilhado e cria a thread de trabalho
asyncState_t * BeginLoadData(FILE *file, float *array, unsigned sz)
{
    asyncState_t *state = (asyncState_t *)calloc(1, sizeof(asyncState_t));
    state->file = file;
    state->data = array;
    state->sz = sz;
    pthread_create(&state->thread, NULL, AsyncLoadData, (void *)state);
    return state;
}

// End aguarda a thread de trabalho terminar, se necessário faz o tratamento de
// erro se necessário e desaloca o(s) objeto(s) compartilhado(s)
unsigned EndLoadData(asyncState_t *state)
{
    pthread_join(state->thread, NULL);
    unsigned ret = state->read;
    if (state->error)
    {
        printf("Erro ao carregar dados!\n");
    }
    free(state);
    return ret;
}


void * AsyncLoadData(void *ctx)
{
    asyncState_t *state = (asyncState_t *)ctx;
    state->read = LoadData(state->file, state->data, state->sz);
    return NULL;
}


unsigned LoadData(FILE *file, float *data, unsigned sz)
{
    char line[32]; 
    int i = 0;
    unsigned read = 0;
    while ((fgets(line, sizeof(line), file) != NULL) && (read < sz))
    {
        data[i] = strtof(line, NULL);
        read += sizeof(float);
        i++;
    }
    return read;
}
