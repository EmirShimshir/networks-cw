#ifndef SERVER_H
#define SERVER_H

#include <sys/types.h>

#define PORT 80
#define MAX_PROCESSES 4

typedef struct {
    int server_fd;
    pid_t child_pids[MAX_PROCESSES];
    void (*handler)(int fd);
} server_t;

server_t *new_server(void (*handler)(int fd));
void clear_server(server_t *server);
int start_server(server_t *server);

#endif
