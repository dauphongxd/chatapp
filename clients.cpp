#include <iostream>
#include <cstring> // this one for manipulation C-style
#include <sys/types.h>
#include <sys/socket.h> // socket
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread> // using thread library
#include <signal.h>

#define MAX_LEN 200 // maximum length for message

using namespace std;

bool exit_flag = false; // flag to indicate exit
thread t_send, t_recv; // thread for sending and receving message
int client_socket;
string client_name;

void handle_exit_signal(int signal);
void send_message(int client_socket);
void recv_message(int client_socket);
void exit_chat();

int main() {
    // create socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket: ");
        exit(-1);
    }
    // define the client address
    struct sockaddr_in client;
    client.sin_family = AF_INET;
    client.sin_port = htons(9999); // set port to any available port, this example use port 9999
    client.sin_addr.s_addr = INADDR_ANY; // use any available IP address
    //client.sin_addr.s_addr = inet_addr("192.168.1.2"); // change the IP if testing on different machine on the same network

    memset(&client.sin_zero, 0, sizeof(client.sin_zero)); // clear structure

    // connect to the server
    if (connect(client_socket, (struct sockaddr *)&client, sizeof(struct sockaddr_in)) == -1) {
        // return if error
        perror("connect: ");
        exit(-1);
    }

    // signal for exit
    signal(SIGINT, handle_exit_signal);

    // get the client's name
    char name_c[MAX_LEN];
    cout << "Enter your name: ";
    cin.getline(name_c, MAX_LEN);
    client_name = name_c;

    // send the client's name to server
    send(client_socket, name_c, sizeof(name_c), 0);

    // create threads for sending and receiving messages
    thread t1(send_message, client_socket);
    thread t2(recv_message, client_socket);

    // move the thread to global variables
    t_send = std::move(t1);
    t_recv = std::move(t2);

    // join all threads
    if (t_send.joinable()) t_send.join();
    if (t_recv.joinable()) t_recv.join();

    return 0;
}

// function to handle the exit signal
void handle_exit_signal(int signal) {
    exit_chat();
    exit(signal);
}

// function to handle exiting the chat
void exit_chat() {
    char str[MAX_LEN] = "!exit";
    send(client_socket, str, sizeof(str), 0); // send exit to the server
    exit_flag = true;
    if (t_send.joinable()) t_send.detach(); // detach send thread
    if (t_recv.joinable()) t_recv.detach(); // detach recv threac
    close(client_socket);
    exit(0);
}

// function to send messages
void send_message(int client_socket) {
    while (1) {
        char str[MAX_LEN];
        cout << "\rYou: ";
        cin.getline(str, MAX_LEN); // get message input from the user/client
        send(client_socket, str, sizeof(str), 0); // send message to server
        cout << "\33[2K\r"; // formatted line after sending message
        if (strcmp(str, "!exit") == 0) {
            exit_chat(); // exit if user enter special command
            return;
        }
    }
}

// function to receive messages
void recv_message(int client_socket) {
    while (1) {
        if (exit_flag) return; // exit if exit_flag set to true
        char name[MAX_LEN];
        char str[MAX_LEN];
        int bytes_received = recv(client_socket, name, sizeof(name), 0); // get sender's name
        if (bytes_received <= 0) continue;
        bytes_received = recv(client_socket, str, sizeof(str), 0); // get the message
        if (bytes_received <= 0) continue;
        if (client_name == name && strcmp(str, (string(client_name) + " has left the chat.").c_str()) != 0) continue; // skip the message sent by this client except logout message
        cout << "\r\33[2K" << name << ": " << str << endl; // formatted line and print received message
        cout << "You: " << flush; // prompt for new input
    }
}
