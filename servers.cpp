#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define MAX_LEN 200

using namespace std;

vector<thread> threads; // Vector to store threads handling clients
vector<int> client_sockets; // Vector to store client sockets
mutex clients_mtx; // Mutex to protect shared resources

// Function to handle communication with a client
void handle_client(int client_socket);

int main() {
    int server_socket;
    // Create a socket for the server
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket: ");
        exit(-1);
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(10000);
    server.sin_addr.s_addr = INADDR_ANY;
    memset(&server.sin_zero, 0, sizeof(server.sin_zero)); // Clear structure

    // Bind the socket to the server address
    if (::bind(server_socket, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) == -1) {
        perror("bind error: ");
        exit(-1);
    }

    // Listen for incoming connections
    if (listen(server_socket, 8) == -1) {
        perror("listen error: ");
        exit(-1);
    }

    while (1) {
        struct sockaddr_in client;
        int client_socket;
        socklen_t len = sizeof(sockaddr_in);

        // Accept a new client connection
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client, &len)) == -1) {
            perror("accept error: ");
            exit(-1);
        }

        // Add the client socket to the vector and create a new thread to handle the client
        {
            lock_guard<mutex> guard(clients_mtx);
            client_sockets.push_back(client_socket);
        }
        threads.emplace_back([client_socket]() { handle_client(client_socket); });
    }

    // Join all threads before exiting
    for (thread &th : threads) {
        if (th.joinable()) th.join();
    }

    // Close the server socket
    close(server_socket);
    return 0;
}

// Function to handle communication with a client
void handle_client(int client_socket) {
    char name[MAX_LEN], str[MAX_LEN];
    // Receive the client's name
    recv(client_socket, name, sizeof(name), 0);

    // Broadcast join message
    string join_msg = string(name) + " has joined the chat.";
    cout << join_msg << endl; // Display join message on the server
    {
        lock_guard<mutex> guard(clients_mtx);
        for (int sock : client_sockets) {
            if (sock != client_socket) {
                send(sock, name, sizeof(name), 0);
                send(sock, join_msg.c_str(), join_msg.length() + 1, 0);
            }
        }
    }

    while (1) {
        // Receive messages from the client
        int bytes_received = recv(client_socket, str, sizeof(str), 0);
        if (bytes_received <= 0 || strcmp(str, "!exit") == 0) {
            // If an error occurs or the client sends an exit message, close the connection
            close(client_socket);
            {
                lock_guard<mutex> guard(clients_mtx);
                client_sockets.erase(remove(client_sockets.begin(), client_sockets.end(), client_socket), client_sockets.end());
            }
            // Broadcast logout message
            string logout_msg = string(name) + " has left the chat.";
            cout << logout_msg << endl; // Display leave message on the server
            for (int sock : client_sockets) {
                send(sock, name, sizeof(name), 0);
                send(sock, logout_msg.c_str(), logout_msg.length() + 1, 0);
            }
            return;
        }
        // Print the received message
        cout << name << ": " << str << endl;

        // Broadcast the message to all other clients
        lock_guard<mutex> guard(clients_mtx);
        for (int sock : client_sockets) {
            if (sock != client_socket) {
                send(sock, name, sizeof(name), 0);
                send(sock, str, sizeof(str), 0);
            }
        }
    }
}
