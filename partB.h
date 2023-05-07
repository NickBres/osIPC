#ifndef PARTB_H
#define PARTB_H

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
#include <sys/un.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>






#define BUFFER_SIZE 32768 // 32KB udp cant send more 

void generate_file(char* filename, long size_in_bytes);
uint32_t generate_checksum(char* filename);
int delete_file(char* filename);
void print_time_diff(struct timeval* start, struct timeval* end);
void send_file(char *ip, char* port, char* filename, int domain, int type, int protocol);
void recive_file(char* port,int domain, int type, int protocol);
void send_file_uds(char* filename,char* sockpath,int type);
void recive_file_uds(char* sockpath,int type);
void copy_file_mmap(char* filenameFrom, char* filenameTo);
void copy_file_pipe(char* filenameFrom, char* filenameTo);
int min(int a, int b);

#endif