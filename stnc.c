#include "stnc.h"

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
char *filename = NULL;

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("-----Usage-----\nPartA:\n   Client: ./stnc -[c] <ip> <port>\n   Server: ./stnc -[s] <port>\nPartB:\n   Client: ./stnc -[c] <ip> <port> -[p] <type> <param>\n");
        return 1;
    }
    if (!strcmp(argv[1], "-c"))
    {
        if (argc > 4 && !strcmp(argv[4], "-p"))
        {
            test = 1;
            if (argc > 5 && !strcmp(argv[5], "ipv4"))
            {
                ipv4 = 1;
                if (argc == 7 && !strcmp(argv[6], "tcp"))
                {
                    tcp = 1;
                }
                else if (argc == 7 && !strcmp(argv[6], "udp"))
                {
                    udp = 1;
                }
                else
                {
                    printf("Wrong param! (tcp, udp)\n");
                    return 1;
                }
            }
            else if (argc > 5 && !strcmp(argv[5], "ipv6"))
            {
                ipv6 = 1;
                if (argc == 7 && !strcmp(argv[6], "tcp"))
                {
                    tcp = 1;
                }
                else if (argc == 7 && !strcmp(argv[6], "udp"))
                {
                    udp = 1;
                }
                else
                {
                    printf("Wrong param! (tcp, udp)\n");
                    return 1;
                }
            }
            else if (argc > 5 && !strcmp(argv[5], "uds"))
            {
                uds = 1;
                if (argc == 7 && !strcmp(argv[6], "dgram"))
                {
                    dgram = 1;
                }
                else if (argc == 7 && !strcmp(argv[6], "stream"))
                {
                    stream = 1;
                }
                else
                {
                    printf("Wrong param! (dgram, stream)\n");
                    return 1;
                }
            }
            else if (argc > 5 && !strcmp(argv[5], "mmap"))
            {
                isMmap = 1;
                if (argc == 7 && argv[6] == NULL)
                {
                    filename = argv[6];
                }
            }
            else if (argc > 5 && !strcmp(argv[5], "pipe"))
            {
                isPipe = 1;
                if (argc == 7 && argv[6] != NULL)
                {
                    filename = argv[6];
                }
            }
            else
            {
                printf("Wrong type! (ipv4, ipv6, uds, mmap, pipe)\n");
                return 1;
            }
        }
        run_client(argv[2], argv[3]);
    }
    else if (!strcmp(argv[1], "-s"))
    {
        run_server(argv[2]);
    }

    else
    {
        printf("Usage:\n Client: stnc -[c] <ip> <port>\n Server: stnc -[s] <port>\n");
        return 1;
    }
}

void run_client(char *ip, char *port)
{
    printf("--------------------\n");
    printf("------ Client ------\n");
    printf("--------------------\n");

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
        printf("ERROR connecting\n");
        exit(1);
    }

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
            if (filename == NULL)
            {
                filename = "test.txt";
                generate_file(filename, 100 * 1024 * 1024);
                deleteFile = 1; // flag to delete file after transfer
            }
            // create new port for file transfer port+1
            char new_port[10];
            sprintf(new_port, "%d", atoi(port) + 1);

            // Send file
            int bytesSent = send(sockfd, "test", 4, 0); // send test command
            if (bytesSent < 0)
            {
                printf("ERROR send() failed\n");
                exit(1);
            }
            sleep(1);
            if (tcp && ipv4)
            {
                printf("TCP IPv4\n");
                bytesSent = send(sockfd, "ipv4 tcp", 8, 0); // send test command
            }
            else if (udp && ipv4)
            {
                printf("UDP IPv4\n");
                bytesSent = send(sockfd, "ipv4 udp", 8, 0); // send test command
            }
            else if (tcp && ipv6)
            {
                printf("TCP IPv6\n");
                bytesSent = send(sockfd, "ipv6 tcp", 8, 0); // send test command
            }
            else if (udp && ipv6)
            {
                printf("UDP IPv6\n");
                bytesSent = send(sockfd, "ipv6 udp", 8, 0); // send test command
            }
            else if (uds && dgram)
            {
                printf("UDS DGRAM\n");
                bytesSent = send(sockfd, "uds dgram", 9, 0); // send test command
            }
            else if (uds && stream)
            {
                printf("UDS STREAM\n");
                bytesSent = send(sockfd, "uds stream", 10, 0); // send test command
            }
            else if (isMmap)
            {
                printf("MMAP\n");
                bytesSent = send(sockfd, "mmap", 5, 0); // send test command
            }
            else if (isPipe)
            {
                printf("PIPE\n");
                bytesSent = send(sockfd, "pipe", 5, 0); // send test command
            }
            if (bytesSent < 0)
            {
                printf("ERROR send() failed\n");
                exit(1);
            }
            sleep(1); // wait for server to start listening
            struct timeval start, end;
            gettimeofday(&start, NULL);
            printf("Start at: %ld.%06ld\n", start.tv_sec, start.tv_usec);
            if (tcp && ipv4)
            {
                send_file(ip, new_port, filename, AF_INET, SOCK_STREAM, IPPROTO_TCP);
            }
            else if (udp && ipv4)
            {
                send_file(ip, new_port, filename, AF_INET, SOCK_DGRAM, 0);
            }
            else if (tcp && ipv6)
            {
                send_file(ip, new_port, filename, AF_INET6, SOCK_STREAM, IPPROTO_TCP);
            }
            else if (uds && dgram)
            {
                send_file_uds(new_port, filename, SOCK_DGRAM);
            }
            else if (uds && stream)
            {
                send_file_uds(new_port, filename, SOCK_STREAM);
            }
            else if (isMmap || isPipe)
            {
                bytesSent = send(sockfd, filename, strlen(filename), 0); // send filename
                if (bytesSent < 0)
                {
                    printf("ERROR send() failed\n");
                    exit(1);
                }
            }

            int recieved = recv(sockfd, messageBuffer, BUFFER_SIZE_MESSAGE - 1, 0); // recieve checksum
            if (recieved < 0)
            {
                printf("ERROR recv() failed\n");
                exit(1);
            }
            printf("Checksum recieved!\n");
            uint32_t my_checksum = generate_checksum(filename);
            uint32_t checksum = atoi(messageBuffer);
            if (my_checksum == checksum)
            {
                printf("Checksums match!\n");
            }
            else
            {
                printf("Checksums don't match, something went wrong!\n");
            }
            gettimeofday(&end, NULL);
            printf("End at: %ld.%06ld\n", end.tv_sec, end.tv_usec);
            print_time_diff(&start, &end);

            if (deleteFile)
                delete_file(filename);
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
    printf("--------------------\n");
    printf("------ Server ------\n");
    printf("--------------------\n");

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
                printf("Client disconnected\n");
                exit(1);
            }
            messageBuffer[bytesRecv] = '\0';
            printf("Client: %s \n", messageBuffer);
            if (!strcmp(messageBuffer, "test"))
            {
                printf("Test mode\n");
                bzero(messageBuffer, BUFFER_SIZE_MESSAGE);
                int bytesRecv = recv(clientSock, messageBuffer, 10, 0);
                if (bytesRecv < 0)
                {
                    printf("ERROR recv() failed\n");
                    exit(1);
                }
                messageBuffer[bytesRecv] = '\0';

                char *new_port[10];
                sprintf(new_port, "%d", atoi(port) + 1);
                if (!strcmp(messageBuffer, "ipv4 tcp"))
                {
                    printf("TCP IPv4\n");
                    recive_file(new_port, AF_INET, SOCK_STREAM, IPPROTO_TCP);
                }
                else if (!strcmp(messageBuffer, "ipv4 udp"))
                {
                    printf("UDP IPv4\n");
                    recive_file(new_port, AF_INET, SOCK_DGRAM, 0);
                }
                else if (!strcmp(messageBuffer, "ipv6 tcp"))
                {
                    printf("TCP IPv6\n");
                    recive_file(new_port, AF_INET6, SOCK_STREAM, IPPROTO_TCP);
                }
                else if (!strcmp(messageBuffer, "ipv6 udp"))
                {
                    printf("UDP IPv6\n");
                    recive_file(new_port, AF_INET6, SOCK_DGRAM, 0);
                }
                else if (!strcmp(messageBuffer, "uds dgram"))
                {
                    printf("UDS DGRAM\n");
                    recive_file_uds(new_port, SOCK_DGRAM);
                }
                else if (!strcmp(messageBuffer, "uds stream"))
                {
                    printf("UDS STREAM\n");
                    recive_file_uds(new_port, SOCK_STREAM);
                }
                else if (!strcmp(messageBuffer, "mmap"))
                {
                    printf("Mmap\n");
                    bytesRecv = recv(clientSock, messageBuffer, BUFFER_SIZE_MESSAGE - 1, 0); // recive file name
                    if (bytesRecv < 0)
                    {
                        printf("ERROR recv() failed\n");
                        exit(1);
                    }
                    copy_file_mmap(messageBuffer, "recived.txt");
                }
                else if (!strcmp(messageBuffer, "pipe"))
                {
                    printf("Pipe\n");
                    bytesRecv = recv(clientSock, messageBuffer, BUFFER_SIZE_MESSAGE - 1, 0); // recive file name
                    if (bytesRecv < 0)
                    {
                        printf("ERROR recv() failed\n");
                        exit(1);
                    }
                    copy_file_pipe(messageBuffer, "recived.txt");
                }
                u_int32_t checksum = generate_checksum("recived.txt");
                char checksum_str[10];
                sprintf(checksum_str, "%u", checksum);
                int bytesSent = send(clientSock, checksum_str, 10, 0);
                if (bytesSent < 0)
                {
                    printf("ERROR send() failed\n");
                    exit(1);
                }
                delete_file("recived.txt");
            }
            bzero(messageBuffer, BUFFER_SIZE_MESSAGE);
        }
    }
    close(sockfd);
    close(clientSock);
}