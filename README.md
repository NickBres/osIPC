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
First 6 options will generate 100mb file automatically in the project folder. 2 last options will need path to an existing file in <param>, if you will not write a filename it will be generated as in other methods.
All generated files will be deleted after the test.

> When sending file via udp or uds dgram time shown will not show the real time it took to transfer the file it because it waiting 2 seconds timeout to finish the file transfer from the server side. So real time will be approximately (given - 2000).

```sh
   ./stnc -c <ip> <port> -p <type> <param>
   ```
Time results will be printed on Client side.
You can add -q flag for quite run (Only time result will be printed).

## Screenshots
![image](https://user-images.githubusercontent.com/70432147/236688965-f214716d-f9f5-40ff-a159-8a0d77e59254.png)
![image](https://user-images.githubusercontent.com/70432147/236689005-82417f31-ba57-461f-897e-6ff77a4abf3f.png)
![image](https://user-images.githubusercontent.com/70432147/236689023-9baf0f81-6fbf-414b-a4b1-96ec6ac54b8d.png)
![image](https://user-images.githubusercontent.com/70432147/236689050-ddbf0b9e-aba4-4339-ae12-77e1d7f96313.png)
![image](https://user-images.githubusercontent.com/70432147/236689182-bcd89702-3a87-4fee-b257-5c481eae0ba3.png)
![image](https://user-images.githubusercontent.com/70432147/236689222-126ec1d5-69f7-4001-a433-9b2272a036ee.png)
![image](https://user-images.githubusercontent.com/70432147/236689240-97a431e8-06b1-47a3-9497-d5928cb45675.png)
![image](https://user-images.githubusercontent.com/70432147/236689265-1be85d93-4d1e-4d49-980f-5cda25ad035a.png)
![image](https://user-images.githubusercontent.com/70432147/236689286-5dec142b-7c09-4509-969a-ccdc07034012.png)





