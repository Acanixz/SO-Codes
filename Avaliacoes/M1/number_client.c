#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define SOCK_PATH "/tmp/M1SO/Number"

void sendOnce(const char *inputNumber);
void sendInfinitelyRandom(float interval);
void sendInfinitelySame(const char *inputNumber, float interval);
void generateRandomNumber(char *buffer, size_t length);
int connectToServer();

int main()
{
    char inputNumber[1024];
    int mode;
    float interval;

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

    return 0;
}

void sendOnce(const char *inputNumber) {
    int sockfd = connectToServer();
    if (sockfd < 0) {
        return;
    }

    // Enviar dados para o servidor
    if (write(sockfd, inputNumber, strlen(inputNumber) + 1) < 0)
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
        // Gerar número aleatório de 10 dígitos
        generateRandomNumber(buffer, 3);

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

void sendInfinitelySame(const char *inputNumber, float interval) {
    while (1) {
        int sockfd = connectToServer();
        if (sockfd < 0) {
            return;
        }

        // Enviar dados para o servidor
        if (write(sockfd, inputNumber, strlen(inputNumber) + 1) < 0)
        {
            perror("Falha em escrever no socket");
            close(sockfd);
            return;
        }

        printf("Dado enviado ao servidor: %s\n", inputNumber);

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
