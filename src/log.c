#include "log.h"

#include "utils.h"
#include <stdio.h>

static void log(const char *level, const char *message);

void log_msg(const char *level, const char *message) {
    log(level, message);
}

void log_msg_int(const char *level, const char *message, int number) {
    char buffer[MAX_BUFFER];
    snprintf(buffer, sizeof(buffer), message, number);
    log(level, buffer);
}

static void log(const char *level, const char *message) {
    if (!LOG_ON) {
        return;
    }
    const time_t now = time(NULL);
    FILE *log_file = fopen(LOG_FILE, "a");
    if (log_file != NULL) {
        fprintf(log_file, "[%s] [%s] %s\n", strtok(ctime(&now), "\n"), level, message);
        fclose(log_file);
    } else {
        perror("Failed to open log file");
    }
}
