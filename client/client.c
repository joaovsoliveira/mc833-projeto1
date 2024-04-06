#include <winsock2.h>
#include <ws2tcpip.h> // Para inet_pton
#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib, "ws2_32.lib") // Link com a biblioteca Winsock (se estiver usando MSVC para compilar)

#define PORT "8080"

int main() {
    WSADATA wsaData;
    SOCKET sock = INVALID_SOCKET;
    struct sockaddr_in server;
    char message[1000], server_reply[2000];

    // Inicializa o Winsock
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        printf("Failed. Error Code : %d", WSAGetLastError());
        return 1;
    }

    // Criação do socket
    if((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket : %d", WSAGetLastError());
    }
    puts("Socket created");

    server.sin_addr.s_addr = inet_addr("172.30.208.101");  //trocar o endereço IP para o do servidor
    server.sin_family = AF_INET;
    server.sin_port = htons(8080);

    // Conexão ao servidor remoto
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect failed. Error");
        return 1;
    }

    puts("Connected\n");

    // Loop de comunicação
    do {
        printf("Enter message: ");
        fgets(message, sizeof(message), stdin); // Usando fgets para leitura segura de strings.

        // Remove newline character, se existir
        message[strcspn(message, "\n")] = 0;

        // Verifica se o usuário digitou "quit"
        if (strcmp(message, "quit") == 0) {
            break;
        }

        // Envia a mensagem
        if (send(sock, message, strlen(message), 0) < 0) {
            puts("Send failed");
            break;
        }

        // Limpa o buffer de resposta
        memset(server_reply, 0, sizeof(server_reply));

        // Recebe a resposta do servidor
        int recv_size = recv(sock, server_reply, sizeof(server_reply) - 1, 0);
        if (recv_size < 0) {
            puts("recv failed");
            break;
        } else if (recv_size == 0) {
            puts("Server closed connection");
            break;
        } else {
            // Coloca um terminador de string no final da mensagem recebida antes de imprimir
            server_reply[recv_size] = '\0';
            puts("Server reply:");
            puts(server_reply);
        }
    } while(1);

    closesocket(sock);
    WSACleanup(); // Limpa o Winsock

    return 0;
}