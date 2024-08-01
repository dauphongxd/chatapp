#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define MAX_LEN 200 // maximum length for messages

using namespace std;

vector<thread> threads; // vector to store threads handling clients
vector<int> client_sockets; // vector to store client sockets
mutex clients_mtx; // mutex to protect shared resources

// function to handle communication with a client
void handle_client(int client_socket);

int main() {
    int server_socket;
    // create a socket for the server
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket: ");
        exit(-1);
    }

    // define the server address
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(9999); // set port to any available port, this example use port 9999
    server.sin_addr.s_addr = INADDR_ANY; // use any available IP address
    //server.sin_addr.s_addr = inet_addr("192.168.1.2"); // change the IP if testing on different machine on the same network
    memset(&server.sin_zero, 0, sizeof(server.sin_zero)); // clear structure

    // bind the socket to the server address
    if (::bind(server_socket, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) == -1) {
        // return if error
        perror("bind error: ");
        exit(-1);
    }

    // listen for incoming connections
    if (listen(server_socket, 8) == -1) {
        // return if error
        perror("listen error: ");
        exit(-1);
    }

    while (1) {
        struct sockaddr_in client;
        int client_socket;
        socklen_t len = sizeof(sockaddr_in);

        // accept a new client connection
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client, &len)) == -1) {
            // return if error
            perror("accept error: ");
            exit(-1);
        }

        // add the client socket to the vector and create a new thread to handle the client
        {
            lock_guard<mutex> guard(clients_mtx);
            client_sockets.push_back(client_socket);
        }
        // create new thread to handle client
        threads.emplace_back([client_socket]() { handle_client(client_socket); });
    }

    // join all threads before exit
    for (thread &th : threads) {
        if (th.joinable()) th.join();
    }

    // close the socket
    close(server_socket);
    return 0;
}

// function to handle communication with a client
void handle_client(int client_socket) {
    char name[MAX_LEN], str[MAX_LEN];
    // receive the client's name
    recv(client_socket, name, sizeof(name), 0);

    // broadcast join message
    string join_msg = string(name) + " has joined the chat.";
    cout << join_msg << endl; // display join message on the server
    {
        lock_guard<mutex> guard(clients_mtx); // protect shared resources
        for (int sock : client_sockets) { // iterate through all client sockets
            if (sock != client_socket) { // skip the current one
                send(sock, name, sizeof(name), 0); // send client's name
                send(sock, join_msg.c_str(), join_msg.length() + 1, 0); // send join message
            }
        }
    }

    while (1) {
        // receive messages from the client
        int bytes_received = recv(client_socket, str, sizeof(str), 0);
        if (bytes_received <= 0 || strcmp(str, "!exit") == 0) {
            // if an error occurs or the client sends an exit message, close the connection
            close(client_socket);
            {
                lock_guard<mutex> guard(clients_mtx); // protect shared resources
                client_sockets.erase(remove(client_sockets.begin(), client_sockets.end(), client_socket), client_sockets.end()); // remove client socket from vector
            }
            // broadcast logout message
            string logout_msg = string(name) + " has left the chat.";
            cout << logout_msg << endl; // display leave message on the server
            for (int sock : client_sockets) { // iterate through all client sockets
                send(sock, name, sizeof(name), 0); // send client's name
                send(sock, logout_msg.c_str(), logout_msg.length() + 1, 0); // send exit/logout message
            }
            return;
        }
        // print the received message
        cout << name << ": " << str << endl;

        // broadcast the message to all other clients
        lock_guard<mutex> guard(clients_mtx); // protect shared resources
        for (int sock : client_sockets) { // iterate through all client sockets
            if (sock != client_socket) { // skip the current one
                send(sock, name, sizeof(name), 0); // send client's name
                send(sock, str, sizeof(str), 0); // send received message
            }
        }
    }
}
