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

/*  Escolhi 4 threads pois considerando em um servidor de 8 nucleos 
    lógicos onde ele é o principal processo, faz sentido cada pipe
    igualmente balancear a alocação, porém, se rodar em um
    ambiente com mais processos ativos (Comp. Pessoais por exemplo),
    pode considerar diminuir para evitar slowdown devido a 
    concorrencia com outros processos existentes
*/
#define THREAD_POOL_SIZE 4

int socketSetup(char *SOCK_PATH);
void *threadHandler(void *arg);

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
    // Prepara pasta do socket e thread pool
    prepareSocketFolder();
    pthread_t
        str_thread_pool[THREAD_POOL_SIZE],
        num_thread_pool[THREAD_POOL_SIZE]
    ;

    // Prepara o pipe de string e cria threads
    int str_sock = socketSetup(STRING_SOCK_PATH);
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        thread_arg_t *arg = malloc(sizeof(thread_arg_t));
        arg->sock_path = STRING_SOCK_PATH;
        arg->server_sock = str_sock;
        arg->id = i;
        pthread_create(&str_thread_pool[i], NULL, threadHandler, arg);
    }

    // Prepara o pipe de numero e cria threads
    int num_sock = socketSetup(NUMBER_SOCK_PATH);
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        thread_arg_t *arg = malloc(sizeof(thread_arg_t));
        arg->sock_path = NUMBER_SOCK_PATH;
        arg->server_sock = num_sock;
        arg->id = i;
        pthread_create(&num_thread_pool[i], NULL, threadHandler, arg);
    }

    // Segura o thread principal de terminar sem a pool acabar
    // por mais que nunca vá acabar sem intervenção manual...
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        pthread_join(str_thread_pool[i], NULL);
        pthread_join(num_thread_pool[i], NULL);
    }

    return 0;
}

int socketSetup(char *SOCK_PATH){
    int 
        server_sock, // Server socket
        len //  Tamanho do socket
    ;
    struct sockaddr_un local; // Endereço p/ o socket

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

    // Tentativa de colocar o socket no endereço
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
    int first = 0;
    int last = strlen(buffer) - 1;
    char temp;

    // Inverte caracteres da string
    while (first < last) {
        temp = buffer[first];
        buffer[first] = buffer[last];
        buffer[last] = temp;

        first++;
        last--;
    }
}

void numberProcess(char *buffer) {
    // Verifica se é um numero, se não for, sobrescreve o buffer por uma mensagem de erro e retorna
    for (int i = 0; i < strlen(buffer); i++) {
        if (buffer[i] < '0' || buffer[i] > '9') {
            snprintf(buffer, 1024, "ERRO: Nao eh um numero");
            return;
        }
    }

    // Converte string p/ numero e incrementa +10
    int number = atoi(buffer);
    number += 10;

    // Converte numero p/ string
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

        // Processamento do buffer conforme a qual socket pertence
        if (sock_path == STRING_SOCK_PATH){
            stringProcess(&buffer);
        } else {
            numberProcess(&buffer);
        }

        printf("%s [THREAD %d] Enviando p/ cliente: %s\n", sock_path, thread_id, buffer);

        // Retorna para o client
        if (write(client_sock, buffer, strlen(buffer) + 1) < 0)
        {
            perror("Falha em escrever no socket");
            close(client_sock);
            continue;
        }

        // Pedido encerrado, loop continua
        close(client_sock);
    }

    return NULL;
}