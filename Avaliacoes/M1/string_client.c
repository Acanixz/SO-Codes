#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define SOCK_PATH "/tmp/M1SO/String"

void sendOnce(const char *inputString);
void sendInfinitelyRandom(float interval);
void sendInfinitelySame(const char *inputString, float interval);
void generateRandomString(char *buffer, size_t length);
int connectToServer();

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
    int sockfd = connectToServer();
    if (sockfd < 0) {
        return;
    }

    // Enviar dados para o servidor
    if (write(sockfd, inputString, strlen(inputString) + 1) < 0)
    {
        perror("Falha em escrever no socket");
        close(sockfd);
        return;
    }

    printf("Dado enviado ao servidor.\n");

    // Ler dados do servidor
    char buffer[1024];
    if (read(sockfd, buffer, sizeof(buffer)) < 0)
    {
        perror("Falha em ler do socket");
        close(sockfd);
        return;
    }

    printf("Dado recebido: %s\n", buffer);

    // Fechar o socket
    close(sockfd);
}

void sendInfinitelyRandom(float interval) {
    char buffer[1024];

    while (1) {
        // Gerar string aleatória de 10 caracteres
        generateRandomString(buffer, 10);

        int sockfd = connectToServer();
        if (sockfd < 0) {
            return;
        }

        // Enviar dados para o servidor
        if (write(sockfd, buffer, strlen(buffer) + 1) < 0)
        {
            perror("Falha em escrever no socket");
            close(sockfd);
            return;
        }

        printf("Dado enviado ao servidor: %s\n", buffer);

        // Ler dados do servidor
        if (read(sockfd, buffer, sizeof(buffer)) < 0)
        {
            perror("Falha em ler do socket");
            close(sockfd);
            return;
        }

        printf("Dado recebido: %s\n", buffer);

        // Fechar o socket
        close(sockfd);

        // Dormir por um curto período para evitar sobrecarregar o servidor
        usleep((int)(interval * 1000000)); // Converter segundos para microsegundos
    }
}

void sendInfinitelySame(const char *inputString, float interval) {
    while (1) {
        int sockfd = connectToServer();
        if (sockfd < 0) {
            return;
        }

        // Enviar dados para o servidor
        if (write(sockfd, inputString, strlen(inputString) + 1) < 0)
        {
            perror("Falha em escrever no socket");
            close(sockfd);
            return;
        }

        printf("Dado enviado ao servidor: %s\n", inputString);

        // Ler dados do servidor
        char buffer[1024];
        if (read(sockfd, buffer, sizeof(buffer)) < 0)
        {
            perror("Falha em ler do socket");
            close(sockfd);
            return;
        }

        printf("Dado recebido: %s\n", buffer);

        // Fechar o socket
        close(sockfd);

        // Dormir por um curto período para evitar sobrecarregar o servidor
        usleep((int)(interval * 1000000)); // Converter segundos para microsegundos
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

int connectToServer() {
    int sockfd, len;
    struct sockaddr_un remote;

    // Criar socket
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Falha em criar o socket");
        return -1;
    }

    // Conectar ao servidor
    memset(&remote, 0, sizeof(remote));
    remote.sun_family = AF_UNIX;
    strncpy(remote.sun_path, SOCK_PATH, sizeof(remote.sun_path) - 1);
    len = strlen(remote.sun_path) + sizeof(remote.sun_family);
    if (connect(sockfd, (struct sockaddr *)&remote, len) < 0)
    {
        perror("Falha em conectar no servidor");
        close(sockfd);
        return -1;
    }

    return sockfd;
}
