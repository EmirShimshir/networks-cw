static void logger(const char *level, const char *message) {
    if (!LOGGER_ON) {
        return;
    }
    const time_t now = time(NULL);
    FILE *log_file = fopen(LOGGER_FILE, "a");
    if (log_file != NULL) {
        fprintf(log_file, "[%s] [%s] %s\n", strtok(ctime(&now), "\n"), level, message);
        fclose(log_file);
    } else {
        perror("Failed to open logger file");
    }
}