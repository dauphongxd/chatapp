# Server-Client Application



## Overview



This project includes a server-client application written in C++. The server and client communicate using POSIX threads for concurrency. The application demonstrates basic networking concepts and multithreading.



## Requirements



- C++11 or higher

- POSIX Threads (pthread)



## Compilation



To compile the server and client applications, use the following commands:



```bash
g++ -std=c++11 -o servers servers.cpp -pthread
g++ -std=c++11 -o clients clients.cpp -pthread
```

# Running the Application
## Start the Server
First, start the server application:

 ```bash
./servers
```
## Start the Client
Then, in a separate terminal, start the client application:

```bash
./clients
```

# Files
- servers.cpp: The server application source code.
- clients.cpp: The client application source code.

# Features
- Server: Handles incoming client connections and processes their requests concurrently using threads.
- Client: Connects to the server and communicates by sending requests and receiving responses.

# Usage
1. Compile both server and client applications.
2. Run the server application.
3. Run the client application in a separate terminal window.
4. Observe the interaction between the client and the server.

# License
This project is licensed under the MIT License - see the LICENSE file for details.

