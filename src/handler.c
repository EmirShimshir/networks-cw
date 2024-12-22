#include "handler.h"
#include "logger.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>



static void serve_static_file(int fd, const char *file_path);
static void send_response(int fd, int status, const char *status_text, const char *content_type, const char *body, size_t body_length);

void handle_client_request(int client_fd) {
    char buffer[MAX_BUFFER];
    ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
    if (bytes_read <= 0) {
        close(client_fd);
        return;
    }
    buffer[bytes_read] = '\0';
    char method[16], path[256];
    sscanf(buffer, "%15s %255s", method, path);

    if (strcmp(method, "GET") != 0 && strcmp(method, "HEAD") != 0) {
        send_response(client_fd, 405, "Method Not Allowed", "text/html", "<h1>405 Method Not Allowed</h1>", 31);
        return;
    }
    if (strcmp(path, "/") == 0) {
        strncpy(path, "/index.html", sizeof(path));
    }

    char file_path[MAX_BUFFER];
    snprintf(file_path, sizeof(file_path), "%s%s", ROOT_DIR, path);
    log_msg(INFO, "Handling request for file");
    serve_static_file(client_fd, file_path);
}

static void serve_static_file(int fd, const char *file_path) {
    struct stat file_stat;
    if (stat(file_path, &file_stat) == -1 || S_ISDIR(file_stat.st_mode)) {
        send_response(fd, 404, "Not Found", "text/html", "<h1>404 Not Found</h1>", 22);
        return;
    }

    if (access(file_path, R_OK) == -1) {
        send_response(fd, 403, "Forbidden", "text/html", "<h1>403 Forbidden</h1>", 22);
        return;
    }

    int file_fd = open(file_path, O_RDONLY);
    if (file_fd == -1) {
        send_response(fd, 500, "Internal Server Error", "text/html", "<h1>500 Internal Error</h1>", 27);
        return;
    }

    char header[MAX_BUFFER];
    snprintf(header, sizeof(header), "HTTP/1.1 200 OK\r\nContent-Length: %ld\r\nContent-Type: %s\r\nConnection: close\r\n\r\n",
             file_stat.st_size, (strstr(file_path, ".html") ? "text/html" : "text/plain"));
    write(fd, header, strlen(header));

    char buffer[MAX_BUFFER];
    ssize_t bytes_read;
    while ((bytes_read = read(file_fd, buffer, sizeof(buffer))) > 0) {
        write(fd, buffer, bytes_read);
    }

    log_msg(INFO, "Sent response ok");
    close(file_fd);
    close(fd);
}

static void send_response(int fd, int status, const char *status_text, const char *content_type, const char *body, size_t body_length) {
    char header[MAX_BUFFER];
    int header_length = snprintf(header, sizeof(header),
                                 "HTTP/1.1 %d %s\r\n"
                                 "Content-Length: %zu\r\n"
                                 "Content-Type: %s\r\n"
                                 "Connection: close\r\n\r\n",
                                 status, status_text, body_length, content_type);
    write(fd, header, header_length);
    if (body && body_length > 0) {
        write(fd, body, body_length);
    }
    close(fd);

    char buffer[MAX_BUFFER];
    snprintf(buffer, sizeof(buffer), "Sent response: %d %s", status, status_text);
    log_msg(INFO, buffer);
}




