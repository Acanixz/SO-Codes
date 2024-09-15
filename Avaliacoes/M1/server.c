//Servidor pipe (testado usando WSL)

// 
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <errno.h>

#define STRING_SOCK_PATH "/tmp/M1SO/String"
#define NUMBER_SOCK_PATH "/tmp/M1SO/Number"
#define THREAD_POOL_SIZE 4

int socketSetup(char *SOCK_PATH);
void *threadHandler(void *arg);
void strThreadHandler();

typedef struct {
    char *sock_path;
    int server_sock;
    int id;
} thread_arg_t;

void prepareSocketFolder(){
    struct stat st = {0};

    // Procura por pasta
    if (stat("/tmp/M1SO", &st) == -1) {
        // Cria diretório
        if (mkdir("/tmp/M1SO", 0700) == 0) {
            printf("Pasta temporaria criada com sucesso\n");
        } else {
            perror("Falha ao criar o diretorio: ");
            return 1;
        }
    } else {
        printf("Pasta temporaria OK\n");
    }

    return 0;
}

int main()
{
    prepareSocketFolder();
    pthread_t
        str_thread_pool[THREAD_POOL_SIZE],
        num_thread_pool[THREAD_POOL_SIZE]
    ;

    // Socket e Thread pool para String
    int str_sock = socketSetup(STRING_SOCK_PATH);
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        thread_arg_t *arg = malloc(sizeof(thread_arg_t));
        arg->sock_path = STRING_SOCK_PATH;
        arg->server_sock = str_sock;
        arg->id = i;
        pthread_create(&str_thread_pool[i], NULL, threadHandler, arg);
    }

    // Socket e Thread pool para Número
    int num_sock = socketSetup(NUMBER_SOCK_PATH);
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        thread_arg_t *arg = malloc(sizeof(thread_arg_t));
        arg->sock_path = NUMBER_SOCK_PATH;
        arg->server_sock = num_sock;
        arg->id = i;
        pthread_create(&num_thread_pool[i], NULL, threadHandler, arg);
    }

    // Wait for threads to finish (optional, depending on your design)
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        pthread_join(str_thread_pool[i], NULL);
        pthread_join(num_thread_pool[i], NULL);
    }

    return 0;
}

int socketSetup(char *SOCK_PATH){
    // server_sock = socket que declara pipe disponivel (usa local)
    int server_sock, len;
    struct sockaddr_un local;
    char buffer[1024];

    // Criação do pipe
    server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_sock < 0)
    {
        perror("Falha em criar o pipe");
        return 1;
    }

    memset(&local, 0, sizeof(local)); // server addr. = 0
    local.sun_family = AF_UNIX; // endereçamento UNIX
    strncpy(local.sun_path, SOCK_PATH, sizeof(local.sun_path) - 1); // Define server addr.
    unlink(local.sun_path); // Apaga socket pre-existente
    len = strlen(local.sun_path) + sizeof(local.sun_family); // len = server addr. + familia

    // Tentativa de colocar o socket no server addr.
    if (bind(server_sock, (struct sockaddr *)&local, len) < 0)
    {
        perror("Falha em capturar o socket");
        close(server_sock);
        return 1;
    }

    // Aceitar no server_sock até X conexões simultaneas
    if (listen(server_sock, 5) < 0)
    {
        perror("Falha em escutar o socket");
        close(server_sock);
        return 1;
    }

    // Conectado!
    printf("Servidor ouvindo em %s...\n", SOCK_PATH);

    return server_sock;
}

void stringProcess(char *buffer){
    // Initialize first and last pointers
    int first = 0;
    int last = strlen(buffer) - 1;
    char temp;

    // Swap characters till first and last meet
    while (first < last) {
      
        // Swap characters
        temp = buffer[first];
        buffer[first] = buffer[last];
        buffer[last] = temp;

        // Move pointers towards each other
        first++;
        last--;
    }
}

void numberProcess(char *buffer) {
    int number = atoi(buffer); // Convert the string to an integer
    number += 10; // Increment the number by 10

    // Convert the number back to a string
    snprintf(buffer, 1024, "%d", number);
}

void *threadHandler(void *arg){
    // Define o server address no escopo local e apaga o arg alocado
    thread_arg_t *thread_arg = (thread_arg_t *)arg;
    char *sock_path = thread_arg->sock_path;
    int server_sock = thread_arg->server_sock;
    int thread_id = thread_arg->id;
    free(thread_arg);

    // 
    struct sockaddr_un remote;
    socklen_t len = sizeof(remote);
    char buffer[1024];

    // Thread continuará aceitando conexões
    while (1) {
        printf("%s [THREAD %d]: Aguardando conexao\n", sock_path, thread_id);
        int client_sock = accept(server_sock, (struct sockaddr *)&remote, &len);
        if (client_sock < 0) {
            perror("Falha em aceitar conexão");
            continue;
        }

        printf("%s [THREAD %d]: Cliente conectado\n", sock_path ,thread_id);

        // Lendo buffer
        if (read(client_sock, buffer, sizeof(buffer)) < 0)
        {
            perror("Falha em ler do socket");
            close(client_sock);
            continue;
        }

        printf("%s [THREAD %d] Dado recebido: %s\n", sock_path, thread_id, buffer);

        if (sock_path == STRING_SOCK_PATH){
            stringProcess(&buffer);
        } else {
            numberProcess(&buffer);
        }

        printf("%s [THREAD %d] Enviando p/ cliente: %s\n", sock_path, thread_id, buffer);

        // Retorna a string para o client
        if (write(client_sock, buffer, strlen(buffer) + 1) < 0)
        {
            perror("Falha em escrever no socket");
            close(client_sock);
            continue;
        }

        // Fecha conexão atual, loop continua
        close(client_sock);
    }

    return NULL;
}