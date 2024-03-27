#include <winsock2.h>
#include <ws2tcpip.h> // Para inet_pton
#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib, "ws2_32.lib") // Link com a biblioteca Winsock

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

    server.sin_addr.s_addr = inet_addr("172.29.116.227");
    server.sin_family = AF_INET;
    server.sin_port = htons(8080);

    // Conexão ao servidor remoto
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect failed. Error");
        return 1;
    }

    puts("Connected\n");

    // Comunicação
    printf("Enter message: ");
    scanf("%s", message);

    if (send(sock, message, strlen(message), 0) < 0) {
        puts("Send failed");
        return 1;
    }

    if (recv(sock, server_reply, 2000, 0) < 0) {
        puts("recv failed");
    }

    puts("Server reply:");
    puts(server_reply);

    closesocket(sock);
    WSACleanup(); // Limpa o Winsock

    return 0;
}