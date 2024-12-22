#ifndef LOG_H
#define LOG_H

#define LOG_ON 1
#define INFO "INFO"
#define ERROR "ERROR"
#define LOG_FILE "server.log"

void log_msg(const char *level, const char *message);
void log_msg_int(const char *level, const char *message, int number);

#endif