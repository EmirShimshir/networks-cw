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