#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <cstring>
#include <string>
#include <sstream>

#pragma comment(lib, "Ws2_32.lib")
using namespace std;

#define MAX_LEN 200

void connect_to_server(const char* ip, int port);
void send_message(SOCKET& sock, const char* msg);
void receive_message(SOCKET& sock);
void private_message(SOCKET& sock, const char* recipient, const char* msg);

int main() {
    WSADATA wsaData;
    SOCKET client_socket = INVALID_SOCKET;
    struct sockaddr_in server_addr;
    char msg[MAX_LEN];

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed!" << endl;
        return 1;
    }

    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) {
        cerr << "Socket creation failed!" << endl;
        WSACleanup();
        return 1;
    }

    // Define the server address and port
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(10000);  // Match server port
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to the server
    connect_to_server("127.0.0.1", 10000);

    // Send initial message to server
    cout << "Enter your name: ";
    cin.getline(msg, MAX_LEN);
    send_message(client_socket, msg);

    // Receive messages from server in a loop
    while (true) {
        receive_message(client_socket);

        cout << "Enter message (type #exit to quit or @username to send private): ";
        cin.getline(msg, MAX_LEN);
        
        if (strcmp(msg, "#exit") == 0) {
            send_message(client_socket, msg);
            break;
        }
        
        send_message(client_socket, msg);
    }

    closesocket(client_socket);
    WSACleanup();
    return 0;
}

void connect_to_server(const char* ip, int port) {
    struct sockaddr_in server_addr;
    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    if (connect(client_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        cerr << "Unable to connect to server. Error: " << WSAGetLastError() << endl;
        closesocket(client_socket);
        WSACleanup();
        exit(1);
    }
}

void send_message(SOCKET& sock, const char* msg) {
    if (send(sock, msg, strlen(msg) + 1, 0) == SOCKET_ERROR) {
        cerr << "Send failed" << endl;
    }
}

void receive_message(SOCKET& sock) {
    char msg[MAX_LEN];
    int result = recv(sock, msg, MAX_LEN, 0);
    if (result > 0) {
        cout << "Received: " << msg << endl;
    }
}

void private_message(SOCKET& sock, const char* recipient, const char* msg) {
    char full_msg[MAX_LEN];
    snprintf(full_msg, MAX_LEN, "@%s %s", recipient, msg);
    send_message(sock, full_msg);
}
