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