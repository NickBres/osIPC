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
First 6 options will generate 100mb file automatically in the project folder. 2 last options will need path to an existing file in <param>, if you will write "default" instead, it will be generated as in other methods.
All generated and recieved files will be deleted after the test.

> When sending via udp protocol, may happen that packets will lost. If it happens recieve function will exit efter 2 seconds timeout. It will still prints the time it took but with +2 seconds of timeout.

```sh
   ./stnc -c <ip> <port> -p <type> <param>
   ```
Time results will be printed on Server side.

Add -q flag for quite run (Only time result will be printed). 

## Screenshots






