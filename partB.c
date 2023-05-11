#include "partB.h"

void generate_file(char *filename, long size_in_bytes, int quiet)
{
    FILE *fp;
    if (open_file_write(filename, &fp) < 0)
    {
        return;
    }

    const int chunk_size = 1024 * 1024; // 1MB
    char buffer[chunk_size];
    int bytes_written = 0;

    while (bytes_written < size_in_bytes)
    {
        int bytes_to_write = chunk_size;
        if (bytes_written + bytes_to_write > size_in_bytes)
        {
            bytes_to_write = size_in_bytes - bytes_written;
        }
        fwrite(buffer, bytes_to_write, 1, fp);
        bytes_written += bytes_to_write;
    }

    fclose(fp);
    if (!quiet)
        printf("Generated file '%s' of size %ld bytes\n", filename, size_in_bytes);
}

uint32_t generate_checksum(char *filename, int quiet)
{
    FILE *fp;
    if (open_file_read(filename, &fp) < 0)
    {
        return -1;
    }

    const int chunk_size = 1024 * 1024; // 1MB
    char buffer[chunk_size];
    uint32_t checksum = 0;

    while (!feof(fp))
    {
        size_t bytes_read = fread(buffer, 1, chunk_size, fp);
        for (size_t i = 0; i < bytes_read; i++)
        {
            checksum += (uint32_t)buffer[i];
        }
    }

    fclose(fp);
    if (!quiet)
        printf("Generated checksum for file '%s': 0x%08x\n", filename, checksum);
    return checksum;
}

int delete_file(char *filename, int quiet)
{
    int status = remove(filename);
    if (status != 0)
    {
        printf("Error deleting file '%s'\n", filename);
        return -1;
    }
    if (!quiet)
        printf("File '%s' deleted successfully\n", filename);
    return 0;
}

void print_time_diff(struct timeval *start, struct timeval *end)
{
    long seconds = end->tv_sec - start->tv_sec;
    long microseconds = end->tv_usec - start->tv_usec;
    if (microseconds < 0)
    {
        microseconds += 1000000;
        seconds--;
    }
    long milliseconds = seconds * 1000 + microseconds / 1000;
    printf("%ld\n", milliseconds);
};

int open_file_read(char *filename, FILE **fp)
{
    *fp = fopen(filename, "rb");
    if (!*fp)
    {
        printf("Error opening file '%s'\n", filename);
        return -1;
    }
    int filesize = 0;
    fseek(*fp, 0L, SEEK_END); // seek to end of file
    filesize = ftell(*fp);    // get current file pointer
    fseek(*fp, 0L, SEEK_SET); // seek back to beginning of file
    return filesize;
}

int open_file_write(char *filename, FILE **fp)
{
    *fp = fopen(filename, "wb");
    if (!*fp)
    {
        printf("Error opening file '%s'\n", filename);
        return -1;
    }
    int filesize = 0;
    fseek(*fp, 0L, SEEK_END); // seek to end of file
    filesize = ftell(*fp);    // get current file pointer
    fseek(*fp, 0L, SEEK_SET); // seek back to beginning of file
    return filesize;
}

void send_file(char *ip, char *port, char *filename, int domain, int type, int protocol, int quiet)
{
    if (!quiet)
        printf("Sending file '%s' to %s:%s\n", filename, ip, port);
    // Open File
    FILE *fp;
    int filesize = open_file_read(filename, &fp);
    if (filesize < 0)
    {
        printf("Error opening file\n");
        return;
    }

    // Create Socket
    int sockfd = socket(domain, type, protocol);
    if (sockfd < 0)
    {
        printf("ERROR opening socket\n");
        exit(1);
    }
    struct sockaddr_storage addr;
    socklen_t addr_len;

    if (domain == AF_INET6)
    { // ipv6
        int optval = 1;
        setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY, &optval, sizeof(optval));
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)&addr;
        memset((char *)addr6, 0, sizeof(*addr6));
        addr6->sin6_family = domain;
        addr6->sin6_port = htons(atoi(port));
        inet_pton(AF_INET6, ip, &addr6->sin6_addr);

        addr_len = sizeof(*addr6);
        addr = *(struct sockaddr_storage *)addr6;
    }
    else if (domain == AF_INET)
    { // ipv4
        struct sockaddr_in *addr4 = (struct sockaddr_in *)&addr;
        memset((char *)addr4, 0, sizeof(*addr4));
        addr4->sin_family = domain;
        addr4->sin_port = htons(atoi(port));
        inet_pton(AF_INET, ip, &addr4->sin_addr);

        addr_len = sizeof(*addr4);
        addr = *(struct sockaddr_storage *)addr4;
    }
    else if (domain == AF_UNIX)
    { // unix
        struct sockaddr_un *addr_un = (struct sockaddr_un *)&addr;
        memset((char *)addr_un, 0, sizeof(*addr_un));
        addr_un->sun_family = domain;
        strcpy(addr_un->sun_path, port); // port must be socket path

        addr_len = sizeof(*addr_un);
        addr = *(struct sockaddr_storage *)addr_un;
    }

    if (type == SOCK_STREAM || domain == AF_UNIX)
    {
        if (connect(sockfd, (struct sockaddr *)&addr, addr_len) < 0)
        {
            printf("ERROR connecting\n");
            exit(1);
        }
        if (!quiet)
            printf("Connected to %s:%s\n", ip, port);
    }

    // Send file
    char buffer[BUFFER_SIZE] = {0};
    int sent_bytes = 0;
    int bytes_read = 0;
    while (sent_bytes < filesize)
    {
        bytes_read = min(BUFFER_SIZE, filesize - sent_bytes); // Read at most BUFFER_SIZE bytes
        fread(buffer, 1, bytes_read, fp);
        int bytes_sent;
        if (type == SOCK_STREAM || domain == AF_UNIX)
        {
            bytes_sent = send(sockfd, buffer, bytes_read, 0);
        }
        else if (type == SOCK_DGRAM)
        {
            sleep(0.01); // delay to make sure packets are sent in order
            bytes_sent = sendto(sockfd, buffer, bytes_read, 0, (struct sockaddr *)&addr, sizeof(addr));
        }
        if (bytes_sent < 0)
        {
            printf("ERROR send() failed (FILE)\n");
            exit(1);
        }
        sent_bytes += bytes_sent;
        bzero(buffer, BUFFER_SIZE);
    }
    fclose(fp);
    if (!quiet)
        printf("File '%s' sent successfully\n", filename);
    close(sockfd);
}

void recive_file(char *port, int domain, int type, int protocol,int filesize,int quiet)
{
    if(!quiet)
        printf("Reciving file on port %s\n", port);
    // Create Socket
    int sockfd = socket(domain, type, protocol);
    if (sockfd < 0)
    {
        printf("ERROR opening socket\n");
        exit(1);
    }

    struct sockaddr_storage serveraddr, clientaddr;
    // Bind Socket

    socklen_t addr_size;

    if (domain == AF_INET6)
    { // ipv6
        int optval = 1;
        setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY, &optval, sizeof(optval));
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)&serveraddr;
        memset((char *)addr6, 0, sizeof(*addr6));
        addr6->sin6_family = domain;
        addr6->sin6_port = htons(atoi(port));
        addr6->sin6_addr = in6addr_any;

        addr_size = sizeof(*addr6);
        serveraddr = *(struct sockaddr_storage *)addr6;
    }
    else if (domain == AF_INET)
    { // ipv4
        struct sockaddr_in *addr4 = (struct sockaddr_in *)&serveraddr;
        memset((char *)addr4, 0, sizeof(*addr4));
        addr4->sin_family = domain;
        addr4->sin_port = htons(atoi(port));
        addr4->sin_addr.s_addr = INADDR_ANY;

        addr_size = sizeof(*addr4);
        serveraddr = *(struct sockaddr_storage *)addr4;
    }
    else if (domain == AF_UNIX)
    { // unix
        struct sockaddr_un *addr_un = (struct sockaddr_un *)&serveraddr;
        memset((char *)addr_un, 0, sizeof(*addr_un));
        addr_un->sun_family = domain;
        strncpy(addr_un->sun_path, port, sizeof(addr_un->sun_path) - 1);

        addr_size = sizeof(*addr_un);
        serveraddr = *(struct sockaddr_storage *)addr_un;
    }else{
        printf("ERROR: Invalid domain\n");
        exit(1);
    }

    if (bind(sockfd, (struct sockaddr *)&serveraddr, addr_size) < 0)
    {
        printf("ERROR on binding\n");
        exit(1);
    }


    int newsockfd;
    if (type == SOCK_STREAM)
    {

        listen(sockfd, 1);
        if(!quiet)
            printf("Listening on port %s\n", port);

        newsockfd = accept(sockfd, (struct sockaddr *)&serveraddr, &addr_size);
        if (newsockfd < 0)
        {
            printf("ERROR on accept\n");
            exit(1);
        }
        if(!quiet)
            printf("Accepted connection \n");
    }

    // Recive File
    char buffer[BUFFER_SIZE] = {0};
    FILE *fp = fopen("recived.txt", "wb");
    int size = 0;
    while (size < filesize)
    {

        
        int recived;
        if (type == SOCK_STREAM)
        {
            recived = recv(newsockfd, buffer, BUFFER_SIZE, 0);
        }
        else if (type == SOCK_DGRAM)
        {
            recived = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&clientaddr, &addr_size);
        }
        if (recived < 0)
        {
            printf("ERROR reciving file\n");
            exit(1);
        }
        if (recived == 0)
        {
            break;
        }
        size += recived;

        // Write to file
        fwrite(buffer, recived, 1, fp);
        bzero(buffer, BUFFER_SIZE);
    }
    fclose(fp);
    if (type == SOCK_STREAM)
        close(newsockfd);
    close(sockfd);
    if (domain == AF_UNIX)
    {
        unlink(port);
    }
}

void copy_file_mmap(char *filenameFrom, char *filenameTo)
{
    // Open file
    int fdFrom = open(filenameFrom, O_RDONLY, S_IRUSR | S_IWUSR);
    if (fdFrom < 0)
    {
        printf("ERROR opening file\n");
        exit(1);
    }
    struct stat statFrom; // Get file size
    if (fstat(fdFrom, &statFrom) < 0)
    {
        printf("ERROR fstat\n");
        exit(1);
    }

    // Create file
    int fdTo = open(filenameTo, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fdTo < 0)
    {
        printf("ERROR opening file\n");
        exit(1);
    }
    if (ftruncate(fdTo, statFrom.st_size) < 0) // Set size of file
    {
        printf("ERROR ftruncate\n");
        exit(1);
    }

    // Map files to memory
    char *mapFrom = mmap(NULL, statFrom.st_size, PROT_READ, MAP_SHARED, fdFrom, 0);
    if (mapFrom == MAP_FAILED)
    {
        printf("ERROR mmap\n");
        exit(1);
    }
    char *mapTo = mmap(NULL, statFrom.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fdTo, 0);
    if (mapTo == MAP_FAILED)
    {
        printf("ERROR mmap\n");
        exit(1);
    }

    // Copy file
    memcpy(mapTo, mapFrom, statFrom.st_size);
}

void copy_file_pipe(char *filenameFrom, char *filenameTo)
{
    // Open file
    int fdFrom = open(filenameFrom, O_RDONLY, S_IRUSR | S_IWUSR);
    if (fdFrom < 0)
    {
        printf("ERROR opening file\n");
        exit(1);
    }

    // Create file
    int fdTo = open(filenameTo, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fdTo < 0)
    {
        printf("ERROR opening file\n");
        exit(1);
    }

    int pipefd[2]; // Create pipe
    if (pipe(pipefd) < 0)
    {
        printf("ERROR pipe\n");
        exit(1);
    }
    pid_t pid = fork();

    if (pid < 0)
    {
        printf("ERROR fork\n");
        exit(1);
    }
    else if (pid == 0)
    {                     // Child
        close(pipefd[0]); // Close read end
        char buffer[BUFFER_SIZE];
        int bytes_read = 0;
        while ((bytes_read = read(fdFrom, buffer, BUFFER_SIZE)) > 0)
        { // Read from file to buffer
            if (write(pipefd[1], buffer, bytes_read) < 0)
            { // Write to pipe
                printf("ERROR write\n");
                exit(1);
            }
        }
        close(pipefd[1]);
        exit(0);
    }
    else
    {
        close(pipefd[1]);
        char buffer[BUFFER_SIZE];
        int bytes_read = 0;
        while ((bytes_read = read(pipefd[0], buffer, BUFFER_SIZE)) > 0)
        { // Read from pipe
            if (write(fdTo, buffer, bytes_read) < 0)
            { // Write to file
                printf("ERROR write\n");
                exit(1);
            }
        }
        close(pipefd[0]);
        pid_t wait(int *status);
    }

    close(fdFrom);
    close(fdTo);
}

int get_file_size(char *filename){
    struct stat statFrom; // Get file size
    if (stat(filename, &statFrom) < 0)
    {
        printf("ERROR stat\n");
        exit(1);
    }
    return statFrom.st_size;
}

int min(int a, int b)
{
    if (a < b)
    {
        return a;
    }
    return b;
}