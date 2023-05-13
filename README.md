# osIPC

This repository contains the code for the stnc project, which is a simple network transfer tool developed in C. The tool allows users to transfer files between a client and a server using different protocols and techniques. It also shows how to control multiple file descriptors without threads using the poll function.


All tests have been performed on a MacBook Pro with an ARM processor using the UTM virtual machine to run GCC (Ubuntu 11.3.0-1ubuntu1~22.04) 11.3.0.

## How to use

```sh
  git clone https://github.com/NickBres/osIPC.git
  cd osIPC/
  make
   ```

> It is not recommended to change the order of input.

First run the server.

```sh
   ./stnc -s <port>
   ```

Once the server is running, you can run the client. Specify the IP address and port of the server.

For the simple chat tool between server and client:
```sh
   ./stnc -c <ip> <port>
   ```

For file transfer using different methods:


| Type  | Param  |
|-------|--------|
| ipv4  | tcp    |
| ipv4  | udp    |
| ipv6  | tcp    |
| ipv6  | udp    |
| uds   | dgram  |
| uds   | stream |
| mmap   | filename  |
| pipe   | filename |


Only the file transfer will use the specified method. All other communications between the server and client will occur via chat using TCP.

All methods will generate a 100MB file for transmission.

All generated and received files will be deleted after the test.

> When sending via UDP protocol, it may happen that packets will be lost. If this occurs, the receive function will exit after a 2-second timeout. It will still print the time it took but with an additional 2 seconds of timeout.

```sh
   ./stnc -s <port> -p
   ./stnc -c <ip> <port> -p <type> <param>
   ```
Time results will be printed on Server side.

Add the -q flag for a quiet run. On the client side, nothing will be printed. On the server side, only the time will be printed.

## Explanation

### Chat

This is a simple chat program between the client and server that works without threads. To achieve this, it uses the poll() function to listen to two file descriptors (stdin and socket) simultaneously. When an event occurs on one of them, it acts accordingly. The server can only listen to one client at a time. If a client disconnects, the server will listen for a new connection.
![image](https://github.com/NickBres/osIPC/assets/70432147/88f7cd28-f955-495e-8474-8d1815256d88)


### Performace test

To test the time it takes for the client to send a file to the server, the program uses the chat function to communicate and send commands to the server. The client sends the test type, file size, checksum, and the time it started sending the file. After that, the client and server open a new port (original port + 1) for file transfer. After the transfer is complete, the server checks the file by generating and comparing checksums, and it prints the time it took for the transfer, even if the checksums don't match or other issues occur.

The first six tests are very similar in their implementation, so they all use the same function with different parameters. When sending via UDP/dgram, packets may be lost because there is no reliable algorithm here to fix packet loss. If some packets are lost, the function will wait for two seconds and then finish after the timeout. This additional time will be included in the test result time.

MMAP: To send a file via MMAP, the client needs to specify a shared memory name and send it to the server. The client then copies the file to the shared memory, and the server copies the file from the shared memory. After sending, the server closes the shared memory.

PIPE: To send a file via pipes, the program uses FIFO pipes. The client needs to specify a FIFO pipe name and send it to the server. The client creates the given FIFO pipe, opens it for writing, and waits for the server to open it for reading. After sending, the server closes the pipe.

### Not quiet run demonstration
![image](https://github.com/NickBres/osIPC/assets/70432147/415fb07c-8f6e-430a-8626-4fd13aa06e1a)
![image](https://github.com/NickBres/osIPC/assets/70432147/26a5f925-52af-4869-85cf-a20f667c5d00)


### Quiet run with all possible ways

In this example, you can see that the IPv4 UDP packets were lost, resulting in a longer time. However, all other methods worked well.
![image](https://github.com/NickBres/osIPC/assets/70432147/ebb48f23-6355-4ca7-a5ad-268aefcd0f96)

### Problems

* There are some issues with program synchronization. Sometimes, one side may run faster than expected, causing bugs that can disrupt the test. Attempts have been made to address this by adding sleep functions in some places, but it may not always be sufficient. Due to time constraints, further improvements were not implemented.
* When the program fails, it may not free the port or other resources used, which can cause issues in subsequent runs. Usually, running "make clean" and rebuilding it resolves this problem.
* UDP is an unstable protocol, particularly when used without additional measures. Sometimes, packets may be lost, and packets may arrive in the wrong order. To address this, a sleep delay was added after sending each packet, which affects the overall transfer time.








