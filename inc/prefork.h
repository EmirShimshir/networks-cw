#ifndef PREFORK_H
#define PREFORK_H

#include "server.h"

#define MAX_EVENTS 64
#define MAX_PROCESSES 4

int create_prefork(server_t *server);
void monitor_prefork(server_t *server);

#endif