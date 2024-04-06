#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sqlite3.h>
#include <locale.h>

#define PORT 8080
#define BUFFER_SIZE 1024
typedef struct {
    char title[100];
    char artist[100];
    char language[50];
    char genre[50];
    char chorus[200];
    int release_year;
} Music;

int removeMusic(int id){
    sqlite3 *db;
    sqlite3_stmt *stmt;
    char sql[256];
    int rc;

    rc = sqlite3_open("MusicDatabase.db", &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return 0;
    } else {
        fprintf(stderr, "Opened database successfully\n");
    }

    snprintf(sql, sizeof(sql), "DELETE FROM music WHERE id = %d;", id);

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 0;
    } else {
        fprintf(stdout, "Music removed successfully\n");
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return 1;
}

int insertMusic(Music music) {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO music (title, artist, language, genre, chorus, release_year) VALUES (?, ?, ?, ?, ?, ?);";
    int rc;

    rc = sqlite3_open("MusicDatabase.db", &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return 0;
    } else {
        fprintf(stderr, "Opened database successfully\n");
    }

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    // Vincula os dados da estrutura Music aos placeholders SQL
    sqlite3_bind_text(stmt, 1, music.title, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, music.artist, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, music.language, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, music.genre, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, music.chorus, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 6, music.release_year);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 0;
    } else {
        fprintf(stdout, "Record created successfully\n");
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return 1;
}

void handle_client(int sock) {
    char client_message[BUFFER_SIZE];
    int read_size;

    // Limpa a mensagem buffer
    memset(client_message, 0, BUFFER_SIZE);

    // Recebe mensagem do cliente e responde
    while ((read_size = recv(sock, client_message, BUFFER_SIZE - 1, 0)) > 0) {
        // Coloca um terminador de string no final da mensagem recebida
        client_message[read_size] = '\0';

        char *command = strtok(client_message, " ");
        if (command != NULL) {
            char *response = "Comando não encontrado.";
            //determina o comando que foi enviado
            if (strcmp(command, "addmusic") == 0) { //adicionar musica 
                Music newMusic;
                strncpy(newMusic.title, strtok(NULL, "|"), sizeof(newMusic.title) - 1);
                strncpy(newMusic.artist, strtok(NULL, "|"), sizeof(newMusic.artist) - 1);
                strncpy(newMusic.language, strtok(NULL, "|"), sizeof(newMusic.language) - 1);
                strncpy(newMusic.genre, strtok(NULL, "|"), sizeof(newMusic.genre) - 1);
                strncpy(newMusic.chorus, strtok(NULL, "|"), sizeof(newMusic.chorus) - 1);
                char *year = strtok(NULL, "|");
                newMusic.release_year = year ? atoi(year) : 0;

                // Insere a música
                int inserted = insertMusic(newMusic);
                response = inserted ? "Música adicionada com sucesso!" : "Erro ao adicionar música.";
            } else if (strcmp(command, "removemusic") == 0) { //remover musica
                char *idStr = strtok(NULL, " ");
                int id = atoi(idStr);

                int removed = removeMusic(id);
                response = removed ? "Música removida com sucesso!" : "Erro ao remover música.";
            } else if (strcmp(command, "listall") == 0) { //listar todas as musicas
        
            } else if (strcmp(command, "list") == 0) { //listar todas as musicas dependendo da flag
        
            } else if (strcmp(command, "listlanguageyear") == 0) { //listar todas as musicas de um idioma lançadas em um ano
        
            }

            //envia a mensagem de resposta ao cliente
            send(sock, response, strlen(response), 0);
        }
        
        // Limpa a mensagem buffer novamente
        memset(client_message, 0, BUFFER_SIZE);
    }

    if(read_size == 0) {
        puts("Cliente desconectado.");
        fflush(stdout);
    } else if(read_size == -1) {
        perror("recv falhou");
    }

    close(sock);
}

int main() {
    int server_fd, client_sock, c;
    struct sockaddr_in server, client;

    // Criação do socket do servidor
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Não foi possível criar o socket");
    }
    puts("Socket criado");

    // Preparação da estrutura sockaddr_in
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    // Bind
    if(bind(server_fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("bind falhou. Error");
        return 1;
    }
    puts("Bind feito");

    // Listen
    listen(server_fd, 3);

    // Aceitar conexões entrantes
    puts("Aguardando conexões...");
    c = sizeof(struct sockaddr_in);
    while ((client_sock = accept(server_fd, (struct sockaddr *)&client, (socklen_t*)&c))) {
        puts("Connection aceita");

        pid_t pid = fork();

        if (pid < 0) {
            perror("fork falhou");
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
        perror("accept falhou");
        return 1;
    }

    return 0;
}
