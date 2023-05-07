#include "partB.h"

void generate_file(char *filename, long size_in_bytes, int quite)
{
    FILE *fp = fopen(filename, "wb");
    if (!fp)
    {
        printf("Error opening file '%s'\n", filename);
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
    if (!quite)
        printf("Generated file '%s' of size %ld bytes\n", filename, size_in_bytes);
}

uint32_t generate_checksum(char *filename, int quite)
{
    FILE *fp = fopen(filename, "rb");
    if (!fp)
    {
        printf("Error opening file '%s'\n", filename);
        return 0;
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
    if (!quite)
        printf("Generated checksum for file '%s': 0x%08x\n", filename, checksum);
    return checksum;
}

int delete_file(char *filename, int quite)
{
    int status = remove(filename);
    if (status != 0)
    {
        printf("Error deleting file '%s'\n", filename);
        return -1;
    }
    if (!quite)
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
    printf("Time elapsed: %ld milliseconds\n", milliseconds);
};

void send_file(char *ip, char *port, char *filename, int domain, int type, int protocol,int quite)
{
    if (!quite)
        printf("Sending file '%s' to %s:%s\n", filename, ip, port);
    // Open File
    int filesize = 0;
    FILE *fp = fopen(filename, "rb");
    if (!fp)
    {
        printf("Error opening file '%s'\n", filename);
        return;
    }
    fseek(fp, 0L, SEEK_END); // seek to end of file
    filesize = ftell(fp);    // get current file pointer
    fseek(fp, 0L, SEEK_SET); // seek back to beginning of file

    // Create Socket
    int sockfd = socket(domain, type, type == SOCK_DGRAM ? 0 : protocol);
    if (sockfd < 0)
    {
        printf("ERROR opening socket\n");
        exit(1);
    }
    struct sockaddr_storage addr;
    
    if(domain == AF_INET6){ // ipv6
        int optval = 1;
        setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY, &optval, sizeof(optval));
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)&addr;
        memset((char *)addr6, 0, sizeof(*addr6));
        addr6->sin6_family = domain;
        addr6->sin6_port = htons(atoi(port));
        inet_pton(AF_INET6, ip, &addr6->sin6_addr);

        addr = *(struct sockaddr_storage *)addr6;
    }else if(domain == AF_INET){ // ipv4
        struct sockaddr_in *addr4 = (struct sockaddr_in *)&addr;
        memset((char *)addr4, 0, sizeof(*addr4));
        addr4->sin_family = domain;
        addr4->sin_port = htons(atoi(port));
        inet_pton(AF_INET, ip, &addr4->sin_addr);

        addr = *(struct sockaddr_storage *)addr4;
    }
    if (type == SOCK_STREAM)
    {
        if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
            printf("ERROR connecting\n");
            exit(1);
        }
        if(!quite)
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
        if (type == SOCK_STREAM)
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
    if (!quite)
        printf("File '%s' sent successfully\n", filename);
    close(sockfd);
}

void recive_file(char *port, int domain, int type, int protocol)
{
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

    if(domain == AF_INET6){ // ipv6
        int optval = 1;
        setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY, &optval, sizeof(optval));
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)&serveraddr;
        memset((char *)addr6, 0, sizeof(*addr6));
        addr6->sin6_family = domain;
        addr6->sin6_port = htons(atoi(port));
        addr6->sin6_addr = in6addr_any;

        addr_size = sizeof(*addr6);
        serveraddr = *(struct sockaddr_storage *)addr6;
    }else if(domain == AF_INET){ // ipv4
        struct sockaddr_in *addr4 = (struct sockaddr_in *)&serveraddr;
        memset((char *)addr4, 0, sizeof(*addr4));
        addr4->sin_family = domain;
        addr4->sin_port = htons(atoi(port));
        addr4->sin_addr.s_addr = INADDR_ANY;

        addr_size = sizeof(*addr4);
        serveraddr = *(struct sockaddr_storage *)addr4;
    }
    if (bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
    {
        printf("ERROR on binding\n");
        exit(1);
    }

    struct pollfd fds[2];
        fds[0].fd = sockfd;
        fds[0].events = POLLIN;

    
    int newsockfd;
    if (type == SOCK_STREAM)
    {
        
        listen(sockfd, 1);
        printf("Listening on port %s\n", port);

        newsockfd = accept(sockfd, (struct sockaddr *)&serveraddr, &addr_size);
        if (newsockfd < 0)
        {
            printf("ERROR on accept\n");
            exit(1);
        }
        fds[1].fd = newsockfd;
        fds[1].events = POLLIN;
        printf("Accepted connection \n");
    }

    // Recive File
    char buffer[BUFFER_SIZE] = {0};
    FILE *fp = fopen("recived.txt", "wb");
    while (1)
    {
        
        int poll_status = poll(fds, 2, 2000); // 2 seconds timeout
        if (poll_status == 0)
        {
            printf("Timeout\n");
            break;
        }
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

        // Write to file
        fwrite(buffer, recived, 1, fp);
        bzero(buffer, BUFFER_SIZE);
    }
    fclose(fp);
    if (type == SOCK_STREAM)
        close(newsockfd);
    close(sockfd);
}

void recive_file_uds(char *sock_path, int type)
{
    printf("Reciving file on Unix domain socket '%s'\n", sock_path);

    // Create Socket
    int sockfd = socket(AF_UNIX, type, 0);
    if (sockfd < 0)
    {
        printf("ERROR opening socket\n");
        exit(1);
    }

    struct sockaddr_un serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sun_family = AF_UNIX;
    strncpy(serveraddr.sun_path, sock_path, sizeof(serveraddr.sun_path) - 1);

    // Bind Socket
    if (bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
    {
        printf("ERROR on binding\n");
        exit(1);
    }

    struct pollfd fds[2];
    fds[0].fd = sockfd;
    fds[0].events = POLLIN;

    int newsockfd;
    if (type == SOCK_STREAM)
    {
        listen(sockfd, 1);
        printf("Listening on Unix domain socket '%s'\n", sock_path);

        newsockfd = accept(sockfd, NULL, NULL);
        if (newsockfd < 0)
        {
            printf("ERROR on accept\n");
            exit(1);
        }
        fds[1].fd = newsockfd;
        fds[1].events = POLLIN;
        printf("Accepted connection \n");
    }

    // Recive File
    char buffer[BUFFER_SIZE] = {0};
    FILE *fp = fopen("recived.txt", "wb");
    while (1)
    {
        int poll_status = poll(fds, 2, 2000); // 2 seconds timeout
        if (poll_status == 0)
        {
            printf("Timeout\n");
            break;
        }

        int recived;
        if (type == SOCK_STREAM)
        {
            recived = recv(newsockfd, buffer, BUFFER_SIZE, 0);
        }
        else if (type == SOCK_DGRAM)
        {
            recived = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, NULL, NULL);
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

        // Write to file
        fwrite(buffer, recived, 1, fp);
        bzero(buffer, BUFFER_SIZE);
    }

    fclose(fp);
    if (type == SOCK_STREAM)
        close(newsockfd);
    close(sockfd);
    unlink(sock_path);

    printf("File received successfully\n");
}


void send_file_uds(char *sock_path, char *filename, int type,int quiet)
{
    if(!quiet)
        printf("Sending file '%s' to Unix domain socket '%s'\n", filename, sock_path);

    // Open File
    int filesize = 0;
    FILE *fp = fopen(filename, "rb");
    if (!fp)
    {
        printf("Error opening file '%s'\n", filename);
        return;
    }
    fseek(fp, 0L, SEEK_END); // seek to end of file
    filesize = ftell(fp);    // get current file pointer
    fseek(fp, 0L, SEEK_SET); // seek back to beginning of file

    // Create Socket
    int sockfd = socket(AF_UNIX, type, 0);
    if (sockfd < 0)
    {
        printf("ERROR opening socket\n");
        exit(1);
    }

    struct sockaddr_un serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sun_family = AF_UNIX;
    strncpy(serveraddr.sun_path, sock_path, sizeof(serveraddr.sun_path) - 1);

    // Connect Socket
    if (connect(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
    {
        printf("ERROR connecting\n");
        exit(1);
    }
    if(!quiet)
        printf("Connected to Unix domain socket '%s'\n", sock_path);

    // Send File
    char buffer[BUFFER_SIZE] = {0};
    int sent_bytes = 0;
    int bytes_read = 0;
    while (sent_bytes < filesize)
    {
        bytes_read = min(BUFFER_SIZE, filesize - sent_bytes); // Read at most BUFFER_SIZE bytes
        fread(buffer, 1, bytes_read, fp);
        int bytes_sent = send(sockfd, buffer, bytes_read, 0);
        if (bytes_sent < 0)
        {
            printf("ERROR send() failed (FILE)\n");
            exit(1);
        }
        sent_bytes += bytes_sent;
        bzero(buffer, BUFFER_SIZE);
    }

    fclose(fp);
    close(sockfd);

    if(!quiet)
        printf("File '%s' sent successfully\n", filename);
}

void copy_file_mmap(char* filenameFrom, char* filenameTo){
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

void copy_file_pipe(char *filenameFrom, char *filenameTo) {
    // Open file
    int fdFrom = open(filenameFrom, O_RDONLY, S_IRUSR | S_IWUSR);
    if (fdFrom < 0) {
        printf("ERROR opening file\n");
        exit(1);
    }

    // Create file
    int fdTo = open(filenameTo, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fdTo < 0) {
        printf("ERROR opening file\n");
        exit(1);
    }
    

    int pipefd[2]; // Create pipe
    if(pipe(pipefd) < 0) {
        printf("ERROR pipe\n");
        exit(1);
    }
    pid_t pid = fork();

    if(pid < 0){
        printf("ERROR fork\n");
        exit(1);
    }else if(pid == 0){ // Child
        close(pipefd[0]); // Close read end
        char buffer[BUFFER_SIZE]; 
        int bytes_read = 0;
        while((bytes_read = read(fdFrom, buffer, BUFFER_SIZE)) > 0){ // Read from file to buffer
            if(write(pipefd[1], buffer, bytes_read) < 0){ // Write to pipe
                printf("ERROR write\n");
                exit(1);
            }
        }
        close(pipefd[1]);
        exit(0);
    }else{
        close(pipefd[1]);
        char buffer[BUFFER_SIZE];
        int bytes_read = 0;
        while((bytes_read = read(pipefd[0], buffer, BUFFER_SIZE)) > 0){  // Read from pipe
            if(write(fdTo, buffer, bytes_read) < 0){ // Write to file
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




int min(int a, int b)
{
    if (a < b)
    {
        return a;
    }
    return b;
}