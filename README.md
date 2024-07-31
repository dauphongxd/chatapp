# Server-Client Application



## Overview



This project includes a server-client application written in C++. The server and client communicate using POSIX threads for concurrency. The application demonstrates basic networking concepts and multithreading.



## Requirements



- C++11 or higher

- POSIX Threads (pthread)



## Compilation



To compile the server and client applications, use the following commands:



```bash

# Compile the server application

g++ -std=c++11 -o servers servers.cpp -pthread



# Compile the client application

g++ -std=c++11 -o clients clients.cpp -pthread
