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
                close(epoll_fd);
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
