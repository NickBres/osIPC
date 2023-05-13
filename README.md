# osIPC

This repository contains the code for the stnc project, which is a simple network transfer tool developed in C. The tool allows users to transfer files between a client and a server using different protocols and techniques. Also it shows how to control couple file descriptors without threads using poll function.


All tests has been made on MacBook Pro with arm proccesor using UTM virtual machine to run gcc (Ubuntu 11.3.0-1ubuntu1~22.04) 11.3.0.

## How to use

```sh
  git clone https://github.com/NickBres/osIPC.git
  cd osIPC/
  make
   ```

> Not recommended to change order of input.

First run the server.

```sh
   ./stnc -s <port>
   ```

After the server is running you can run the client. Ip and port of the server.

For the simple chat tool between server and client:
```sh
   ./stnc -c <ip> <port>
   ```

For the file transfer in a different ways:

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


Only file transfer will be in the given way. All other communications between server and client will be in chat using tcp.

All methods will generate a file 100mb to send it.

All generated and recieved files will be deleted after the test.

> When sending via udp protocol, may happen that packets will lost. If it happens recieve function will exit efter 2 seconds timeout. It will still prints the time it took but with +2 seconds of timeout.

```sh
   ./stnc -s <port> -p
   ./stnc -c <ip> <port> -p <type> <param>
   ```
Time results will be printed on Server side.

Add -q flag for quite run. On the client side nothing will be printed. On the server side only time will be printed. 

## Explanation

### Chat

Simple chat program between client and server that works without threads. To achive that it works with poll() function that listens to two file descriptors (stdin,socket) in the same time and when something happens in one of them it acts acoordingly. Server may listen only to one client in same time. If client will disconnect server will listen again.
![image](https://github.com/NickBres/osIPC/assets/70432147/88f7cd28-f955-495e-8474-8d1815256d88)


### Performace test

To test the time it takes to client send a file to a server, the program uses chat to communicate and send commands to the server. Client will send type of test, file size, checksum and time when it started to send a file. After that client and server will open a new port (original port +1) to transfer the file. After the transfer server will check the file by generating and comparing checksums and print time that transfer took even if checksums dont match or other issue happen.

First six tests are wery simmilar in their realization so they all use same function but with different parameters. When sending via udp/dgram packets may loose because there is no realaible algorithm that may fix it. If some packets will loose function will wait two seconds and will finish after timeout, this time will be included in the test result time.

MMAP: To send a file via MMAP client need to decide about shared memory name and send it to the server. After that client will copy the file to the shared memory and server will copy the file from shared memory. After sending server will close the shared memory.

PIPE: To send a file via PIPES program uses FIFO pipes, so client need to decide about fifo pipes name and send it to the server. After that client will create given fifo pipe and open it for writing and wait for the server to open it for reading. After sending server will close the pipe.

### Not quiet run demonstration
![image](https://github.com/NickBres/osIPC/assets/70432147/415fb07c-8f6e-430a-8626-4fd13aa06e1a)
![image](https://github.com/NickBres/osIPC/assets/70432147/26a5f925-52af-4869-85cf-a20f667c5d00)


### Quiet run with all possible ways

In this example you can see that ipv4 udp packets were lost thats why time is too big. All other ways worked good.
![image](https://github.com/NickBres/osIPC/assets/70432147/ebb48f23-6355-4ca7-a5ad-268aefcd0f96)

### Problems

* There is some issues with programs syncronization. Sometimes one side may run faster than expected and it will cause some bugs that will ruin the test. I tried to fix it with sleep function in some places, but it looks like sometimes its not enough. Because lack of time i cannot make it better.
* When program fails it may not free the port or other used resource so on the next run it may not work because of this. Usually "make clean" and built it agai fix this.
* UDP is very unstable protocol when using it clear. Sometimes packets will be lost. Also packets was arriving in wrong order, to fix it i added sleep after sending each packet so it will affect on the transfer time.








