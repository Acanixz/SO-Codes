#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string.h>
#include <time.h>

#pragma comment(lib, "Ws2_32.lib")

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT 8080

void sendOnce(const char *inputString);
void sendInfinitelyRandom(float interval);
void sendInfinitelySame(const char *inputString, float interval);
void generateRandomString(char *buffer, size_t length);
SOCKET connectToServer();

int main()
{
    char inputString[1024];
    int mode;
    float interval;

    // Solicitar a string do usuário
    printf("O servidor irá receber a string e inverter\n");
    printf("Entre com o dado a ser enviado: ");
    fgets(inputString, sizeof(inputString), stdin);
    inputString[strcspn(inputString, "\n")] = '\0'; // Remove o caractere de nova linha

    // Solicitar o modo
    printf("Escolha o modo:\n1. Enviar uma vez\n2. Enviar infinitamente mensagens aleatórias\n3. Enviar infinitamente a mesma mensagem\n");
    scanf("%d", &mode);
    getchar(); // Consumir o caractere de nova linha deixado pelo scanf

    if (mode == 1) {
        sendOnce(inputString);
    } else if (mode == 2 || mode == 3) {
        // Solicitar o intervalo de tempo
        printf("Entre com o intervalo de tempo (em segundos): ");
        scanf("%f", &interval);
        getchar(); // Consumir o caractere de nova linha deixado pelo scanf

        if (mode == 2) {
            sendInfinitelyRandom(interval);
        } else if (mode == 3) {
            sendInfinitelySame(inputString, interval);
        }
    } else {
        printf("Modo inválido.\n");
    }

    return 0;
}

void sendOnce(const char *inputString) {
    SOCKET sockfd = connectToServer();
    if (sockfd == INVALID_SOCKET) {
        return;
    }

    // Enviar dados para o servidor
    if (send(sockfd, inputString, strlen(inputString) + 1, 0) == SOCKET_ERROR)
    {
        perror("Falha em enviar dados");
        closesocket(sockfd);
        return;
    }

    printf("Dado enviado ao servidor.\n");

    // Ler dados do servidor
    char buffer[1024];
    if (recv(sockfd, buffer, sizeof(buffer), 0) == SOCKET_ERROR)
    {
        perror("Falha em receber dados");
        closesocket(sockfd);
        return;
    }

    printf("Dado recebido: %s\n", buffer);

    // Fechar o socket
    closesocket(sockfd);
}

void sendInfinitelyRandom(float interval) {
    char buffer[1024];

    while (1) {
        // Gerar string aleatória de 10 caracteres
        generateRandomString(buffer, 10);

        SOCKET sockfd = connectToServer();
        if (sockfd == INVALID_SOCKET) {
            return;
        }

        // Enviar dados para o servidor
        if (send(sockfd, buffer, strlen(buffer) + 1, 0) == SOCKET_ERROR)
        {
            perror("Falha em enviar dados");
            closesocket(sockfd);
            return;
        }

        printf("Dado enviado ao servidor: %s\n", buffer);

        // Ler dados do servidor
        if (recv(sockfd, buffer, sizeof(buffer), 0) == SOCKET_ERROR)
        {
            perror("Falha em receber dados");
            closesocket(sockfd);
            return;
        }

        printf("Dado recebido: %s\n", buffer);

        // Fechar o socket
        closesocket(sockfd);

        // Dormir por um curto período para evitar sobrecarregar o servidor
        Sleep((int)(interval * 1000)); // Converter segundos para milissegundos
    }
}

void sendInfinitelySame(const char *inputString, float interval) {
    while (1) {
        SOCKET sockfd = connectToServer();
        if (sockfd == INVALID_SOCKET) {
            return;
        }

        // Enviar dados para o servidor
        if (send(sockfd, inputString, strlen(inputString) + 1, 0) == SOCKET_ERROR)
        {
            perror("Falha em enviar dados");
            closesocket(sockfd);
            return;
        }

        printf("Dado enviado ao servidor: %s\n", inputString);

        // Ler dados do servidor
        char buffer[1024];
        if (recv(sockfd, buffer, sizeof(buffer), 0) == SOCKET_ERROR)
        {
            perror("Falha em receber dados");
            closesocket(sockfd);
            return;
        }

        printf("Dado recebido: %s\n", buffer);

        // Fechar o socket
        closesocket(sockfd);

        // Dormir por um curto período para evitar sobrecarregar o servidor
        Sleep((int)(interval * 1000)); // Converter segundos para milissegundos
    }
}

void generateRandomString(char *buffer, size_t length) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    if (length) {
        for (size_t i = 0; i < length; i++) {
            int key = rand() % (int)(sizeof(charset) - 1);
            buffer[i] = charset[key];
        }
        buffer[length] = '\0';
    }
}

SOCKET connectToServer() {
    WSADATA wsaData;
    SOCKET sockfd = INVALID_SOCKET;
    struct sockaddr_in serverAddr;

    // Inicializar Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        perror("Falha em inicializar o Winsock");
        return INVALID_SOCKET;
    }

    // Criar socket
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == INVALID_SOCKET) {
        perror("Falha em criar o socket");
        WSACleanup();
        return INVALID_SOCKET;
    }

    // Configurar endereço do servidor
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_ADDRESS, &serverAddr.sin_addr) <= 0) {
        perror("Endereço inválido");
        closesocket(sockfd);
        WSACleanup();
        return INVALID_SOCKET;
    }

    // Conectar ao servidor
    if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        perror("Falha em conectar ao servidor");
        closesocket(sockfd);
        WSACleanup();
        return INVALID_SOCKET;
    }

    return sockfd;
}