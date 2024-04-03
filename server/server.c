#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sqlite3.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int insertMusic(){
    sqlite3 *db;
    char *errMsg = 0;
    int rc;

    rc = sqlite3_open("MusicDatabase.db", &db);

    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return(0);
    } else {
        fprintf(stderr, "Opened database successfully\n");
    }

    char *sql = "INSERT INTO music (title, artist, language, genre, chorus, release_year) VALUES ('Song Title', 'Artist', 'English', 'Pop', 'Chorus of the song', 2021);";
    rc = sqlite3_exec(db, sql, NULL, 0, &errMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    } else {
        fprintf(stdout, "Record created successfully\n");
    }
}

void handle_client(int sock) {
    char client_message[BUFFER_SIZE];
    int read_size;

    // Limpa a mensagem buffer
    memset(client_message, 0, BUFFER_SIZE);

    // Recebe mensagem do cliente e responde
    while ((read_size = recv(sock, client_message, BUFFER_SIZE - 1, 0)) > 0) {
        // Envia a mensagem de volta ao cliente
        send(sock, client_message, strlen(client_message), 0);
        insertMusic();
        
        // Limpa a mensagem buffer novamente
        memset(client_message, 0, BUFFER_SIZE);
    }

    if(read_size == 0) {
        puts("Cliente desconectado.");
        fflush(stdout);
    } else if(read_size == -1) {
        perror("recv failed");
    }

    close(sock);
}

int main() {
    int server_fd, client_sock, c;
    struct sockaddr_in server, client;

    // Criação do socket do servidor
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Could not create socket");
    }
    puts("Socket created");

    // Preparação da estrutura sockaddr_in
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    // Bind
    if(bind(server_fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        // Print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("Bind done");

    // Listen
    listen(server_fd, 3);

    // Aceitar conexões entrantes
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    while ((client_sock = accept(server_fd, (struct sockaddr *)&client, (socklen_t*)&c))) {
        puts("Connection accepted");

        pid_t pid = fork();

        if (pid < 0) {
            perror("fork failed");
            return 1;
        }

        if (pid == 0) {
            // Este é o processo filho
            close(server_fd);
            handle_client(client_sock);
            exit(0);
        } else {
            // Este é o processo pai
            close(client_sock);
        }
    }

    if (client_sock < 0) {
        perror("accept failed");
        return 1;
    }

    return 0;
}
