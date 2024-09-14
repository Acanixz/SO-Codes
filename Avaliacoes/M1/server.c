//Servidor pipe (testado usando WSL)

// 
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <pthread.h>

#define STRING_SOCK_PATH "/tmp/M1SO/String"
#define NUMBER_SOCK_PATH "/tmp/M1SO/Number"
#define THREAD_POOL_SIZE 4

int socketSetup(char *SOCK_PATH);
void *strThreadHandler(void *arg);

typedef struct {
    int server_sock;
    int id;
} thread_arg_t;

int main()
{
    // TODO: CÓDIGO PODE FALHAR CASO OS CAMINHOS NAO EXISTAM, CERTIFIQUE QUE A PASTA EXISTE ANTES
    pthread_t 
        str_thread_pool[THREAD_POOL_SIZE],
        num_thread_pool[THREAD_POOL_SIZE]
    ;

    int str_sock = socketSetup(STRING_SOCK_PATH);
    
    // Create thread pool
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        thread_arg_t *arg = malloc(sizeof(thread_arg_t));
        arg->server_sock = str_sock;
        arg->id = i;
        pthread_create(&str_thread_pool[i], NULL, strThreadHandler, arg);
    }

    // TODO: IMPLEMENTAR NUMBER SOCKET
    // int num_sock = socketSetup(NUMBER_SOCK_PATH);

    // Wait for threads to finish (optional, depending on your design)
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        pthread_join(str_thread_pool[i], NULL);
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

void *strThreadHandler(void *arg){
    // Define o server address no escopo local e apaga o arg alocado
    thread_arg_t *thread_arg = (thread_arg_t *)arg;
    int server_sock = thread_arg->server_sock;
    int thread_id = thread_arg->id;
    free(thread_arg);

    // 
    struct sockaddr_un remote;
    socklen_t len = sizeof(remote);
    char buffer[1024];

    while (1) {
        int client_sock = accept(server_sock, (struct sockaddr *)&remote, &len);
        if (client_sock < 0) {
            perror("Falha em aceitar conexão");
            continue;
        }

        printf("Client conectado!\n");

        // Handle the client request
        int n = read(client_sock, buffer, sizeof(buffer));
        if (n > 0) {
            //buffer[n] = '\0';
            printf("Thread %d recebeu: %s\n", thread_id, buffer); // Print the thread identifier
            
            if (buffer == "exit"){
                printf("Fechando conexão!");
            } else {
                printf("Comando normal :)");
            }
        }

        close(client_sock);
    }

    return NULL;
}

void *numThreadHandler(void *arg){

}