#include "prefork.h"
#include "utils.h"
#include "logger.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

volatile sig_atomic_t stop_requested = 0;

static void prefork_process(server_t *server);
static void handle_signal(int signal);

int create_prefork(server_t *server) {
    for (int i = 0; i < MAX_PROCESSES; ++i) {
        pid_t pid = fork();
        if (pid == 0) {
            prefork_process(server);
            exit(0);
        } else if (pid > 0) {
            server->child_pids[i] = pid;
        } else {
            perror("Fork failed");
            log_msg(ERROR, "Fork failed");
            return -1;
        }
    }
    return 0;
}

int monitor_prefork(server_t *server) {
    signal(SIGINT, handle_signal);

    while (!stop_requested) {
        int status;
        pid_t pid = waitpid(-1, &status, WNOHANG);
        if (pid == 0) {
            continue;
        }
        if (pid == -1 && errno == EINTR) {
            continue;
        }
        if (pid == -1) {
            perror("waitpid");
            return -1;
        }

        for (int i = 0; i < MAX_PROCESSES; ++i) {
            if (server->child_pids[i] == pid) {
                log_msg_int(INFO, "Child process pid: %d exited, restarting...", pid);
                pid_t new_pid = fork();
                if (new_pid == 0) {
                    prefork_process(server);
                    exit(0);
                } else if (new_pid > 0) {
                    server->child_pids[i] = new_pid;
                    log_msg_int(INFO, "Child process restarted with pid: %d", new_pid);
                } else {
                    log_msg(ERROR, "Failed to restart child process");
                }
            }
            break;
        }
    }
    log_msg(INFO, "Monitoring process terminated.");
    return 0;
}




static void prefork_process(server_t *server) {
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("Failed to create epoll instance");
        log_msg(ERROR, "Failed to create epoll instance");
        exit(1);
    }

    struct epoll_event event = {0};
    event.events = EPOLLIN;
    event.data.fd = server->server_fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server->server_fd, &event) == -1) {
        perror("Failed to add server_fd to epoll");
        log_msg(ERROR, "Failed to add server_fd to epoll");
        exit(1);
    }

    struct epoll_event events[MAX_EVENTS];

    while (1) {
        int event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        for (int i = 0; i < event_count; ++i) {
            if (events[i].data.fd == server->server_fd) {
                struct sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);
                int client_fd = accept(server->server_fd, (struct sockaddr *)&client_addr, &client_len);
                if (client_fd == -1 && errno == EAGAIN) {
                    continue;
                }
                if (client_fd == -1) {
                    perror("Failed accept");
                    log_msg(ERROR, "Failed accept");
                    exit(1);
                }

                set_nonblocking(client_fd);
                log_msg(INFO, "New client connection accepted");

                struct epoll_event client_event = {0};
                client_event.events = EPOLLIN | EPOLLET;
                client_event.data.fd = client_fd;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_event) == -1) {
                    perror("Failed to add client_fd to epoll");
                    log_msg(ERROR, "Failed to add client_fd to epoll");
                    close(client_fd);
                }
            } else {
                int client_fd = events[i].data.fd;
                server->handler(client_fd);
            }
        }
    }

    close(epoll_fd);
}



static void handle_signal(int signal) {
    if (signal == SIGINT) {
        stop_requested = 1;
    }
}
