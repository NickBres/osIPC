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

void generate_file(char* filename, long size_in_bytes,int quite);
uint32_t generate_checksum(char* filename, int quite);
int delete_file(char* filename, int quite);
void print_time_diff(struct timeval* start, struct timeval* end);
void send_file(char *ip, char* port, char* filename, int domain, int type, int protocol, int quite);
int recive_file(char* port,int domain, int type, int protocol,int filesize,int quiet);
void copy_file_mmap(char* filenameFrom, char* filenameTo);
void copy_file_pipe(char* filenameFrom, char* filenameTo);
int min(int a, int b);
int file_size(char* filename);
void copy_file_to_shm_mmap(char* filenameFrom, char* sharedFilename,int quiet);
void copy_file_from_shm_mmap(char* filenameTo, char* sharedFilename,int fileSize,int quiet);
void recive_file_fifo(char *filenameTo, char* fifoName,int quiet);
void send_file_fifo(char *filenameFrom, char* fifoName,int quiet);

#endif