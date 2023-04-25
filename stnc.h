
#ifndef STNC_H
#define STNC_H

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <sys/time.h>
#include <poll.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024


void run_client(char *ip, char *port);
void run_server(char *port);

#endif