#include "server.h"
#include "utils.h"
#include "logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>


server_t *new_server(void (*handler)(int fd)) {
    server_t *server = (server_t *)malloc(sizeof(server_t));
    if (server == NULL) {
        perror("Failed to allocate memory for server");
        log_msg(ERROR, "Failed to allocate memory for server");
        return NULL;
    }

    server->server_fd = -1;
    for (int i = 0; i < MAX_PROCESSES; ++i) {
        server->child_pids[i] = -1;
    }
    server->handler = handler;

    return server;
}

void clear_server(server_t *server) {
    if (server == NULL) {
        return;
    }
    if (server->server_fd > 0) {
        close(server->server_fd);
        log_msg(INFO, "Server socket closed");
    }
    free(server);
}

int start_server(server_t *server) {
    server->server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server->server_fd == -1) {
        perror("Socket creation failed");
        log_msg(ERROR, "Socket creation failed");
        return -1;
    }

    int opt = 1;
    setsockopt(server->server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server->server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        log_msg(ERROR, "Bind failed");
        return -1;
    }

    if (listen(server->server_fd, SOMAXCONN) == -1) {
        perror("Listen failed");
        log_msg(ERROR, "Listen failed");
        return -1;
    }

    if (set_nonblocking(server->server_fd) == -1) {
        perror("Failed to set server socket to non-blocking");
        log_msg(ERROR, "Failed to set server socket to non-blocking");
        return -1;
    }
    return 0;
}

