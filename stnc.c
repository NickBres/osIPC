#include "stnc.h"

int client = 0;
int server = 0;
int test = 0;
int ipv4 = 0;
int ipv6 = 0;
int tcp = 0;
int udp = 0;
int uds = 0;
int dgram = 0;
int stream = 0;
int isMmap = 0;
int isPipe = 0;
int deleteFile = 0;
int quiet = 0;
char *filename = NULL;
char *ip = NULL;
char *port = NULL;
char *type = NULL;
char *param = NULL;

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("-----Usage-----\nPartA:\n   Client: ./stnc -[c] <ip> <port>\n   Server: ./stnc -[s] <port>\nPartB:\n   Client: ./stnc -[c] <ip> <port> -[p] <type> <param> \n");
        return 1;
    }
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-c") == 0)
        {
            client = 1;
            ip = argv[i + 1];
            port = argv[i + 2];
        }
        else if (strcmp(argv[i], "-s") == 0)
        {
            server = 1;
            port = argv[i + 1];
        }
        else if (strcmp(argv[i], "-p") == 0)
        {
            test = 1;
            type = argv[i + 1];
            param = argv[i + 2];
        }
        else if (strcmp(argv[i], "-q") == 0)
        {
            quiet = 1;
        }
    }

    if (test && client)
    {
        if (!type || !param)
        {
            printf("No type or param after -p\n");
            return 1;
        }
        if (strcmp(type, "ipv4") == 0)
        {
            ipv4 = 1;
        }
        else if (strcmp(type, "ipv6") == 0)
        {
            ipv6 = 1;
        }
        else if (strcmp(type, "uds") == 0)
        {
            uds = 1;
        }
        else if (strcmp(type, "mmap") == 0)
        {
            isMmap = 1;
        }
        else if (strcmp(type, "pipe") == 0)
        {
            isPipe = 1;
        }
        else
        {
            printf("wrong type after -p (ipv4, ipv6, uds, mmap, pipe)\n)");
        }

        if (strcmp(param, "tcp") == 0)
        {
            tcp = 1;
        }
        else if (strcmp(param, "udp") == 0)
        {
            udp = 1;
        }
        else if (strcmp(param, "dgram") == 0)
        {
            dgram = 1;
        }
        else if (strcmp(param, "stream") == 0)
        {
            stream = 1;
        }
        else
        {
            filename = param;
        }
    }

    if (client)
    {
        run_client(ip, port);
    }
    else if (server)
    {
        run_server(port);
    }
    else
    {
        printf("-----Usage-----\nPartA:\n   Client: ./stnc -[c] <ip> <port>\n   Server: ./stnc -[s] <port>\nPartB:\n   Client: ./stnc -[c] <ip> <port> -[p] <type> <param> \n");
    }
}

void run_client(char *ip, char *port)
{
    if (!quiet)
    {
        printf("--------------------\n");
        printf("------ Client ------\n");
        printf("--------------------\n");
    }

    // Create socket
    int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0)
    {
        printf("ERROR opening socket\n");
        exit(1);
    }

    // Get server
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(port));
    int rval = inet_pton(AF_INET, ip, &serv_addr.sin_addr); // Convert IPv4 from text to binary form
    if (rval <= 0)
    {
        printf("ERROR inet_pton() failed\n");
        exit(1);
    }
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("ERROR connecting to %s:%s\n", ip, port);
        exit(1);
    }

    if (!quiet)
        printf("Connected to %s:%s\n", ip, port);

    // Create pollfd to monitor stdin and socket
    struct pollfd fds[2] = {
        {.fd = STDIN_FILENO, .events = POLLIN},
        {.fd = sockfd, .events = POLLIN}};

    char messageBuffer[BUFFER_SIZE_MESSAGE];
    int timeout = -1; // Infinite timeout

    while (1)
    {
        if (test)
        { // test mode
          // generate file if not exist
            generate_file("test.txt", 100 * 1024 * 1024, quiet);
            // create new port for file transfer port+1
            char new_port[10];
            sprintf(new_port, "%d", atoi(port) + 1);

            // send filesize
            int filesize = get_file_size("test.txt");
            char filesize_str[20];
            sprintf(filesize_str, "%d", filesize);
            int bytesSent = send(sockfd, filesize_str, strlen(filesize_str), 0);
            if (bytesSent < 0)
            {
                printf("ERROR send() failed\n");
                exit(1);
            }
            sleep(0.1);

            // send checksum
            uint32_t checksum = generate_checksum("test.txt", quiet);
            char checksum_str[20];
            sprintf(checksum_str, "%d", checksum);
            bytesSent = send(sockfd, checksum_str, strlen(checksum_str), 0);
            if (bytesSent < 0)
            {
                printf("ERROR send() failed\n");
                exit(1);
            }
            sleep(0.1);

            struct timeval start;
            gettimeofday(&start, NULL);

            // send start time
            char start_time_str[20];
            sprintf(start_time_str, "%ld.%06ld", start.tv_sec, start.tv_usec);
            bytesSent = send(sockfd, start_time_str, strlen(start_time_str), 0);
            if (bytesSent < 0)
            {
                printf("ERROR send() failed\n");
                exit(1);
            }
            sleep(0.1);

            if (tcp && ipv4)
            {
                if (!quiet)
                    printf("TCP IPv4\n");
                bytesSent = send(sockfd, "ipv4 tcp", 8, 0); // send test command
            }
            else if (udp && ipv4)
            {
                if (!quiet)
                    printf("UDP IPv4\n");
                bytesSent = send(sockfd, "ipv4 udp", 8, 0); // send test command
            }
            else if (tcp && ipv6)
            {
                if (!quiet)
                    printf("TCP IPv6\n");
                bytesSent = send(sockfd, "ipv6 tcp", 8, 0); // send test command
            }
            else if (udp && ipv6)
            {
                if (!quiet)
                    printf("UDP IPv6\n");
                bytesSent = send(sockfd, "ipv6 udp", 8, 0); // send test command
            }
            else if (uds && dgram)
            {
                if (!quiet)
                    printf("UDS DGRAM\n");
                bytesSent = send(sockfd, "uds dgram", 9, 0); // send test command
            }
            else if (uds && stream)
            {
                if (!quiet)
                    printf("UDS STREAM\n");
                bytesSent = send(sockfd, "uds stream", 10, 0); // send test command
            }
            else if (isMmap)
            {
                if (!quiet)
                    printf("MMAP\n");
                bytesSent = send(sockfd, "mmap", 5, 0); // send test command
            }
            else if (isPipe)
            {
                if (!quiet)
                    printf("PIPE\n");
                bytesSent = send(sockfd, "pipe", 5, 0); // send test command
            }
            if (bytesSent < 0)
            {
                printf("ERROR send() failed\n");
                exit(1);
            }

            sleep(0.1); // make sure server is ready

            if (tcp && ipv4)
            {
                send_file(ip, new_port, "test.txt", AF_INET, SOCK_STREAM, IPPROTO_TCP, quiet);
            }
            else if (udp && ipv4)
            {
                send_file(ip, new_port, "test.txt", AF_INET, SOCK_DGRAM, 0, quiet);
            }
            else if (tcp && ipv6)
            {
                send_file(ip, new_port, "test.txt", AF_INET6, SOCK_STREAM, IPPROTO_TCP, quiet);
            }
            else if (udp && ipv6)
            {
                send_file(ip, new_port, "test.txt", AF_INET6, SOCK_DGRAM, 0, quiet);
            }
            else if (uds && dgram)
            {
                sleep(0.1);
                send_file(0, new_port, "test.txt", AF_UNIX, SOCK_DGRAM, 0, quiet);
            }
            else if (uds && stream)
            {
                sleep(0.1);
                send_file(0, new_port, "test.txt", AF_UNIX, SOCK_STREAM, 0, quiet);
            }
            else if (isMmap)
            {
                
                copy_file_to_shm_mmap("test.txt",filename,quiet); // copy file to shared memory
                bytesSent = send(sockfd, filename, strlen(filename), 0); // send filepath to shared memory
                if (bytesSent < 0)
                {
                    printf("ERROR send() failed\n");
                    exit(1);
                }
            }else if(isPipe){
                bytesSent = send(sockfd, filename, strlen(filename), 0); // send fifo name to named pipe
                if (bytesSent < 0)
                {
                    printf("ERROR send() failed\n");
                    exit(1);
                }
                send_file_fifo("test.txt",filename,quiet); // copy file to named pipe
                
            }
            delete_file("test.txt", quiet);
            exit(0);
        }
        // Poll stdin and socket
        int pollResult = poll(fds, 2, timeout);
        if (pollResult < 0)
        {
            printf("ERROR poll() failed\n");
            exit(1);
        }

        if (fds[0].revents & POLLIN) // check if user input
        {
            // Read user input
            int bytesRead = read(STDIN_FILENO, messageBuffer, BUFFER_SIZE_MESSAGE);
            if (bytesRead < 0)
            {
                printf("ERROR read() failed\n");
                exit(1);
            }
            messageBuffer[bytesRead] = '\0';

            // Send user input to server
            int bytesSent = send(sockfd, messageBuffer, bytesRead, 0);
            if (bytesSent < 0)
            {
                printf("ERROR send() failed\n");
                exit(1);
            }
            bzero(messageBuffer, BUFFER_SIZE_MESSAGE);
        }
        if (fds[1].revents & POLLIN) // check if server sent message
        {
            // Read message from server
            int bytesRecv = recv(sockfd, messageBuffer, BUFFER_SIZE_MESSAGE - 1, 0);
            if (bytesRecv < 0)
            {
                printf("ERROR recv() failed\n");
                exit(1);
            }
            if (bytesRecv == 0)
            {
                printf("Server disconnected\n");
                exit(1);
            }
            messageBuffer[bytesRecv] = '\0';
            printf("Server: %s", messageBuffer);
            bzero(messageBuffer, BUFFER_SIZE_MESSAGE);
        }
    }
    close(sockfd);
}

void run_server(char *port)
{
    if (!quiet)
    {
        printf("--------------------\n");
        printf("------ Server ------\n");
        printf("--------------------\n");
    }

    // Create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0)
    {
        printf("ERROR opening socket\n");
        exit(1);
    }

    // Bind socket to port
    struct sockaddr_in serv_addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(atoi(port))};
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("ERROR on binding\n");
        exit(1);
    }
    while (1)
    {

        // Listen for connections
        if (listen(sockfd, 1) < 0)
        {
            printf("ERROR on listen\n");
            exit(1);
        }

        // Accept connection
        struct sockaddr_in cli_addr;
        socklen_t clilen = sizeof(cli_addr);
        int clientSock = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (clientSock < 0)
        {
            printf("ERROR on accept\n");
            exit(1);
        }

        if (!quiet)
            printf("Client connected\n");

        // Create poll
        struct pollfd fds[2] = {
            {.fd = STDIN_FILENO, .events = POLLIN},
            {.fd = clientSock, .events = POLLIN}};

        char messageBuffer[BUFFER_SIZE_MESSAGE];

        while (1)
        {
            int pllResult = poll(fds, 2, -1);
            if (pllResult < 0)
            {
                printf("ERROR poll() failed\n");
                exit(1);
            }
            if (fds[0].revents & POLLIN) // check if user input
            {
                // Read user input
                int bytesRead = read(STDIN_FILENO, messageBuffer, BUFFER_SIZE_MESSAGE);
                if (bytesRead < 0)
                {
                    printf("ERROR read() failed\n");
                    exit(1);
                }
                messageBuffer[bytesRead] = '\0';
                if (send(clientSock, messageBuffer, bytesRead, 0) < 0)
                {
                    printf("ERROR send() failed\n");
                    exit(1);
                }
                bzero(messageBuffer, BUFFER_SIZE_MESSAGE);
            }
            if (fds[1].revents & POLLIN)
            {
                // Read message from client
                int bytesRecv = recv(clientSock, messageBuffer, BUFFER_SIZE_MESSAGE - 1, 0);
                if (bytesRecv < 0)
                {
                    printf("ERROR recv() failed\n");
                    exit(1);
                }
                if (bytesRecv == 0)
                {
                    if (!quiet)
                        printf("Client disconnected\n");
                    break;
                }
                messageBuffer[bytesRecv] = '\0';
                if (!quiet)
                    printf("Client: %s", messageBuffer);
                if (test)
                {
                    int fileSize = 0;
                    int recievedSize = 0;
                    uint32_t checksum = 0;
                    struct timeval start, end;
                    if (!quiet)
                        printf("\n---------PERFORMANCE MODE---------\n");

                    // recieve file size
                    fileSize = atoi(messageBuffer);
                    if (!quiet)
                        printf("File size will be: %d\n", fileSize);
                    bzero(messageBuffer, BUFFER_SIZE_MESSAGE);

                    // recieve checksum
                    bytesRecv = recv(clientSock, messageBuffer, 20, 0);
                    if (bytesRecv <= 0)
                    {
                        printf("ERROR recv() failed\n");
                        exit(1);
                    }
                    messageBuffer[bytesRecv] = '\0';
                    sscanf(messageBuffer, "%u", &checksum);

                    if (!quiet)
                        printf("Checksum will be: 0x%08x\n", checksum);
                    bzero(messageBuffer, BUFFER_SIZE_MESSAGE);

                    // recieve timestart
                    bytesRecv = recv(clientSock, messageBuffer, 20, 0);
                    if (bytesRecv <= 0)
                    {
                        printf("ERROR recv() failed\n");
                        exit(1);
                    }
                    messageBuffer[bytesRecv] = '\0';
                    sscanf(messageBuffer, "%ld.%06ld", &start.tv_sec, &start.tv_usec);
                    if (!quiet)
                        printf("Start time: %ld.%06ld\n", start.tv_sec, start.tv_usec);
                    bzero(messageBuffer, BUFFER_SIZE_MESSAGE);

                    // recieve test type
                    bytesRecv = recv(clientSock, messageBuffer, 20, 0);
                    if (bytesRecv <= 0)
                    {
                        printf("ERROR recv() failed\n");
                        exit(1);
                    }
                    messageBuffer[bytesRecv] = '\0';
                    if (!quiet)
                        printf("Test: %s\n", messageBuffer);

                    char *new_port[10];
                    sprintf(new_port, "%d", atoi(port) + 1);

                    if (!strcmp(messageBuffer, "ipv4 tcp"))
                    {
                        recievedSize = recive_file(new_port, AF_INET, SOCK_STREAM, IPPROTO_TCP, fileSize, quiet);
                    }
                    else if (!strcmp(messageBuffer, "ipv4 udp"))
                    {
                        recievedSize = recive_file(new_port, AF_INET, SOCK_DGRAM, 0, fileSize, quiet);
                    }
                    else if (!strcmp(messageBuffer, "ipv6 tcp"))
                    {
                        recievedSize = recive_file(new_port, AF_INET6, SOCK_STREAM, IPPROTO_TCP, fileSize, quiet);
                    }
                    else if (!strcmp(messageBuffer, "ipv6 udp"))
                    {
                        recievedSize = recive_file(new_port, AF_INET6, SOCK_DGRAM, 0, fileSize, quiet);
                    }
                    else if (!strcmp(messageBuffer, "uds dgram"))
                    {
                        recievedSize = recive_file(new_port, AF_UNIX, SOCK_DGRAM, 0, fileSize, quiet);
                    }
                    else if (!strcmp(messageBuffer, "uds stream"))
                    {
                        recievedSize = recive_file(new_port, AF_UNIX, SOCK_STREAM, 0, fileSize, quiet);
                    }
                    else if (!strcmp(messageBuffer, "mmap"))
                    {
                        bytesRecv = recv(clientSock, messageBuffer, BUFFER_SIZE_MESSAGE - 1, 0); // recive file name
                        if (bytesRecv < 0)
                        {
                            printf("ERROR recv() failed\n");
                            exit(1);
                        }
                        if (!quiet)
                            printf("Shared file name: %s\n", messageBuffer);
                        sleep(0.1); // wait for file to be created
                        copy_file_from_shm_mmap("recived.txt",messageBuffer,fileSize,quiet); // copy file from shared memory
                        recievedSize = file_size("recived.txt");
                    }
                    else if (!strcmp(messageBuffer, "pipe"))
                    {
                        bytesRecv = recv(clientSock, messageBuffer, BUFFER_SIZE_MESSAGE - 1, 0); // recive file name
                        if (bytesRecv < 0)
                        {
                            printf("ERROR recv() failed\n");
                            exit(1);
                        }
                        if (!quiet)
                            printf("Shared fifo name: %s\n", messageBuffer);
                        sleep(0.1); // wait for file to be created
                        recive_file_fifo("recived.txt",messageBuffer,quiet); // copy file from fifo
                        recievedSize = file_size("recived.txt");
                    }

                    gettimeofday(&end, NULL);

                    u_int32_t recieved_file_checksum = generate_checksum("recived.txt", quiet);
                    if (recieved_file_checksum == checksum && !quiet)
                    {
                        printf("Checksums are equal\n");
                    }
                    else if (!quiet)
                    {
                        printf("Checksums are not equal\n");
                        if (recievedSize != fileSize)
                        {
                            printf("File sizes are not equal packets were lost\n");
                        }
                    }

                    if (!quiet)
                    {
                        printf("End time: %ld.%06ld\n", end.tv_sec, end.tv_usec);
                        printf("Time took: ");
                    }

                    print_time_diff(&start, &end);

                    delete_file("recived.txt", quiet);
                }
                bzero(messageBuffer, BUFFER_SIZE_MESSAGE);
            }
        }
        close(clientSock);
    }
    close(sockfd);
}