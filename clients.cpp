#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <signal.h>

#define MAX_LEN 200

using namespace std;

bool exit_flag = false;
thread t_send, t_recv;
int client_socket;
string client_name;

void handle_exit_signal(int signal);
void send_message(int client_socket);
void recv_message(int client_socket);
void exit_chat();

int main() {
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket: ");
        exit(-1);
    }

    struct sockaddr_in client;
    client.sin_family = AF_INET;
    client.sin_port = htons(10000);
    client.sin_addr.s_addr = INADDR_ANY;
    //client.sin_addr.s_addr = inet_addr("192.168.1.2"); // Change the IP if testing on different machine on the same network

    memset(&client.sin_zero, 0, sizeof(client.sin_zero)); // Clear structure

    if (connect(client_socket, (struct sockaddr *)&client, sizeof(struct sockaddr_in)) == -1) {
        perror("connect: ");
        exit(-1);
    }

    signal(SIGINT, handle_exit_signal);
    char name_c[MAX_LEN];
    cout << "Enter your name: ";
    cin.getline(name_c, MAX_LEN);
    client_name = name_c;
    send(client_socket, name_c, sizeof(name_c), 0);

    thread t1(send_message, client_socket);
    thread t2(recv_message, client_socket);

    t_send = std::move(t1);
    t_recv = std::move(t2);

    if (t_send.joinable()) t_send.join();
    if (t_recv.joinable()) t_recv.join();

    return 0;
}

// Function to handle the exit signal (Ctrl+C)
void handle_exit_signal(int signal) {
    exit_chat();
    exit(signal);
}

// Function to handle exiting the chat
void exit_chat() {
    char str[MAX_LEN] = "!exit";
    send(client_socket, str, sizeof(str), 0);
    exit_flag = true;
    if (t_send.joinable()) t_send.detach();
    if (t_recv.joinable()) t_recv.detach();
    close(client_socket);
    exit(0);
}

void send_message(int client_socket) {
    while (1) {
        char str[MAX_LEN];
        cout << "\rYou: ";
        cin.getline(str, MAX_LEN);
        send(client_socket, str, sizeof(str), 0);
        cout << "\33[2K\r"; // Clear line after sending message
        if (strcmp(str, "!exit") == 0) {
            exit_chat();
            return;
        }
    }
}

void recv_message(int client_socket) {
    while (1) {
        if (exit_flag) return;
        char name[MAX_LEN];
        char str[MAX_LEN];
        int bytes_received = recv(client_socket, name, sizeof(name), 0);
        if (bytes_received <= 0) continue;
        bytes_received = recv(client_socket, str, sizeof(str), 0);
        if (bytes_received <= 0) continue;
        if (client_name == name && strcmp(str, (string(client_name) + " has left the chat.").c_str()) != 0) continue; // Skip the message sent by this client except logout message
        cout << "\r\33[2K" << name << ": " << str << endl; // Clear line and print received message
        cout << "You: " << flush; // Prompt for new input
    }
}
