#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <windows.h>
#include <process.h>

#define STRING_SOCK_PATH "127.0.0.1"
#define NUMBER_SOCK_PATH "127.0.0.2"
#define PORT_STRING 8080
#define PORT_NUMBER 8081

#define THREAD_POOL_SIZE 4

int socketSetup(char *SOCK_PATH, int port);
unsigned __stdcall threadHandler(void *arg);

typedef struct {
    char *sock_path;
    SOCKET server_sock;
    int id;
} thread_arg_t;

int main()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        perror("WSAStartup failed");
        return 1;
    }

    HANDLE str_thread_pool[THREAD_POOL_SIZE];
    HANDLE num_thread_pool[THREAD_POOL_SIZE];

    SOCKET str_sock = socketSetup(STRING_SOCK_PATH, PORT_STRING);
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        thread_arg_t *arg = (thread_arg_t *)malloc(sizeof(thread_arg_t));
        arg->sock_path = STRING_SOCK_PATH;
        arg->server_sock = str_sock;
        arg->id = i;
        str_thread_pool[i] = (HANDLE)_beginthreadex(NULL, 0, threadHandler, arg, 0, NULL);
    }

    SOCKET num_sock = socketSetup(NUMBER_SOCK_PATH, PORT_NUMBER);
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        thread_arg_t *arg = (thread_arg_t *)malloc(sizeof(thread_arg_t));
        arg->sock_path = NUMBER_SOCK_PATH;
        arg->server_sock = num_sock;
        arg->id = i;
        num_thread_pool[i] = (HANDLE)_beginthreadex(NULL, 0, threadHandler, arg, 0, NULL);
    }

    WaitForMultipleObjects(THREAD_POOL_SIZE, str_thread_pool, TRUE, INFINITE);
    WaitForMultipleObjects(THREAD_POOL_SIZE, num_thread_pool, TRUE, INFINITE);

    WSACleanup();
    return 0;
}

int socketSetup(char *SOCK_PATH, int port){
    SOCKET server_sock;
    struct sockaddr_in server;

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == INVALID_SOCKET) {
        perror("Socket creation failed");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(SOCK_PATH);
    server.sin_port = htons(port);

    if (bind(server_sock, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
        perror("Bind failed");
        closesocket(server_sock);
        return 1;
    }

    if (listen(server_sock, 5) == SOCKET_ERROR) {
        perror("Listen failed");
        closesocket(server_sock);
        return 1;
    }

    printf("Server listening on %s:%d...\n", SOCK_PATH, port);
    return server_sock;
}

void stringProcess(char *buffer){
    int first = 0;
    int last = strlen(buffer) - 1;
    char temp;

    while (first < last) {
        temp = buffer[first];
        buffer[first] = buffer[last];
        buffer[last] = temp;

        first++;
        last--;
    }
}

void numberProcess(char *buffer) {
    for (int i = 0; i < strlen(buffer); i++) {
        if (buffer[i] < '0' || buffer[i] > '9') {
            snprintf(buffer, 1024, "ERROR: Not a number");
            return;
        }
    }

    int number = atoi(buffer);
    number += 10;
    snprintf(buffer, 1024, "%d", number);
}

unsigned __stdcall threadHandler(void *arg){
    thread_arg_t *thread_arg = (thread_arg_t *)arg;
    char *sock_path = thread_arg->sock_path;
    SOCKET server_sock = thread_arg->server_sock;
    int thread_id = thread_arg->id;
    free(thread_arg);

    struct sockaddr_in client;
    int client_len = sizeof(client);
    char buffer[1024];

    while (1) {
        printf("%s [THREAD %d]: Waiting for connection\n", sock_path, thread_id);
        SOCKET client_sock = accept(server_sock, (struct sockaddr *)&client, &client_len);
        if (client_sock == INVALID_SOCKET) {
            perror("Accept failed");
            continue;
        }

        printf("%s [THREAD %d]: Client connected\n", sock_path, thread_id);

        if (recv(client_sock, buffer, sizeof(buffer), 0) == SOCKET_ERROR) {
            perror("Receive failed");
            closesocket(client_sock);
            continue;
        }

        printf("%s [THREAD %d] Data received: %s\n", sock_path, thread_id, buffer);

        if (strcmp(sock_path, STRING_SOCK_PATH) == 0){
            stringProcess(buffer);
        } else {
            numberProcess(buffer);
        }

        printf("%s [THREAD %d] Sending to client: %s\n", sock_path, thread_id, buffer);

        if (send(client_sock, buffer, strlen(buffer) + 1, 0) == SOCKET_ERROR) {
            perror("Send failed");
            closesocket(client_sock);
            continue;
        }

        closesocket(client_sock);
    }

    return 0;
}