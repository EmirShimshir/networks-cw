#include "server.h"
#include "prefork.h"
#include "handler.h"
#include <stddef.h>

int main() {
    server_t *server = new_server(handle_client_request);
    if (server == NULL) {
        return -1;
    }

    int err = 0;
    err = start_server(server);
    if (err != 0) {
        clear_server(server);
        return err;
    }
    err = create_prefork(server);
    if (err != 0) {
        clear_server(server);
        return err;
    }

    err = monitor_prefork(server);
    if (err != 0) {
        clear_server(server);
        return err;
    }

    clear_server(server);
    return 0;
}