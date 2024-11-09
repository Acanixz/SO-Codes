#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <string.h>
#include <time.h>

#pragma comment(lib, "Ws2_32.lib")

#define SERVER_ADDRESS "127.0.0.2"
#define SERVER_PORT 8081

void sendOnce(const char *inputNumber);
void sendInfinitelyRandom(float interval);
void sendInfinitelySame(const char *inputNumber, float interval);
void generateRandomNumber(char *buffer, size_t length);
SOCKET connectToServer();

int main()
{
    char inputNumber[1024];
    int mode;
    float interval;

    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed.\n");
        return 1;
    }

    // Solicitar o número do usuário
    printf("O servidor irá receber um número\n");
    printf("Entre com o número a ser enviado: ");
    fgets(inputNumber, sizeof(inputNumber), stdin);
    inputNumber[strcspn(inputNumber, "\n")] = '\0'; // Remove o caractere de nova linha

    // Solicitar o modo
    printf("Escolha o modo:\n1. Enviar uma vez\n2. Enviar infinitamente números aleatórios\n3. Enviar infinitamente o mesmo número\n");
    scanf("%d", &mode);
    getchar(); // Consumir o caractere de nova linha deixado pelo scanf

    if (mode == 1) {
        sendOnce(inputNumber);
    } else if (mode == 2 || mode == 3) {
        // Solicitar o intervalo de tempo
        printf("Entre com o intervalo de tempo (em segundos): ");
        scanf("%f", &interval);
        getchar(); // Consumir o caractere de nova linha deixado pelo scanf

        if (mode == 2) {
            sendInfinitelyRandom(interval);
        } else if (mode == 3) {
            sendInfinitelySame(inputNumber, interval);
        }
    } else {
        printf("Modo inválido.\n");
    }

    // Cleanup Winsock
    WSACleanup();

    return 0;
}

void sendOnce(const char *inputNumber) {
    SOCKET sockfd = connectToServer();
    if (sockfd == INVALID_SOCKET) {
        return;
    }

    // Enviar dados para o servidor
    if (send(sockfd, inputNumber, (int)strlen(inputNumber) + 1, 0) == SOCKET_ERROR)
    {
        printf("Falha em enviar dados: %d\n", WSAGetLastError());
        closesocket(sockfd);
        return;
    }

    printf("Dado enviado ao servidor.\n");

    // Ler dados do servidor
    char buffer[1024];
    if (recv(sockfd, buffer, sizeof(buffer), 0) == SOCKET_ERROR)
    {
        printf("Falha em receber dados: %d\n", WSAGetLastError());
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
        // Gerar número aleatório de 3 digitos
        generateRandomNumber(buffer, 3);

        SOCKET sockfd = connectToServer();
        if (sockfd == INVALID_SOCKET) {
            return;
        }

        // Enviar dados para o servidor
        if (send(sockfd, buffer, (int)strlen(buffer) + 1, 0) == SOCKET_ERROR)
        {
            printf("Falha em enviar dados: %d\n", WSAGetLastError());
            closesocket(sockfd);
            return;
        }

        printf("Dado enviado ao servidor: %s\n", buffer);

        // Ler dados do servidor
        if (recv(sockfd, buffer, sizeof(buffer), 0) == SOCKET_ERROR)
        {
            printf("Falha em receber dados: %d\n", WSAGetLastError());
            closesocket(sockfd);
            return;
        }

        printf("Dado recebido: %s\n", buffer);

        // Fechar o socket
        closesocket(sockfd);

        // Dormir por um curto período para evitar sobrecarregar o servidor
        Sleep((DWORD)(interval * 1000)); // Converter segundos para milissegundos
    }
}

void sendInfinitelySame(const char *inputNumber, float interval) {
    while (1) {
        SOCKET sockfd = connectToServer();
        if (sockfd == INVALID_SOCKET) {
            return;
        }

        // Enviar dados para o servidor
        if (send(sockfd, inputNumber, (int)strlen(inputNumber) + 1, 0) == SOCKET_ERROR)
        {
            printf("Falha em enviar dados: %d\n", WSAGetLastError());
            closesocket(sockfd);
            return;
        }

        printf("Dado enviado ao servidor: %s\n", inputNumber);

        // Ler dados do servidor
        char buffer[1024];
        if (recv(sockfd, buffer, sizeof(buffer), 0) == SOCKET_ERROR)
        {
            printf("Falha em receber dados: %d\n", WSAGetLastError());
            closesocket(sockfd);
            return;
        }

        printf("Dado recebido: %s\n", buffer);

        // Fechar o socket
        closesocket(sockfd);

        // Dormir por um curto período para evitar sobrecarregar o servidor
        Sleep((DWORD)(interval * 1000)); // Converter segundos para milissegundos
    }
}

void generateRandomNumber(char *buffer, size_t length) {
    const char charset[] = "0123456789";
    if (length) {
        for (size_t i = 0; i < length; i++) {
            int key = rand() % (int)(sizeof(charset) - 1);
            buffer[i] = charset[key];
        }
        buffer[length] = '\0';
    }
}

SOCKET connectToServer() {
    SOCKET sockfd;
    struct sockaddr_in serverAddr;

    // Criar socket
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == INVALID_SOCKET)
    {
        printf("Falha em criar o socket: %d\n", WSAGetLastError());
        return INVALID_SOCKET;
    }

    // Configurar endereço do servidor
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_ADDRESS, &serverAddr.sin_addr);

    // Conectar ao servidor
    if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        printf("Falha em conectar no servidor: %d\n", WSAGetLastError());
        closesocket(sockfd);
        return INVALID_SOCKET;
    }

    return sockfd;
}