#include "stnc.h"

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage:\n Client: stnc -[c] <ip> <port>\n Server: stnc -[s] <port>\n");
        return 1;
    }
    if (!strcmp(argv[1], "-c"))
    {
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

    char messageBuffer[BUFFER_SIZE];
    int timeout = -1; // Infinite timeout

    while (1)
    {
        int pollResult = poll(fds, 2, timeout);
        if (pollResult < 0)
        {
            printf("ERROR poll() failed\n");
            exit(1);
        }

        if (fds[0].revents & POLLIN) // check if user input
        {
            // Read user input
            int bytesRead = read(STDIN_FILENO, messageBuffer, BUFFER_SIZE);
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
            bzero(messageBuffer, BUFFER_SIZE);
        }
        if (fds[1].revents & POLLIN) // check if server sent message
        {
            // Read message from server
            int bytesRecv = recv(sockfd, messageBuffer, BUFFER_SIZE - 1, 0);
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
            bzero(messageBuffer, BUFFER_SIZE);
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

    char messageBuffer[BUFFER_SIZE];

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
            int bytesRead = read(STDIN_FILENO, messageBuffer, BUFFER_SIZE);
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
            bzero(messageBuffer, BUFFER_SIZE);
        }
        if (fds[1].revents & POLLIN)
        {
            // Read message from client
            int bytesRecv = recv(clientSock, messageBuffer, BUFFER_SIZE - 1, 0);
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
            printf("Client: %s", messageBuffer);
            bzero(messageBuffer, BUFFER_SIZE);
        }
    }
    close(sockfd);
    close(clientSock);
}