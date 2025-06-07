#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <string>
#include <cstring>
#include <fstream>
#include <unordered_map>
#include <sstream>

#define MAX_LEN 200
#define NUM_COLORS 6

#pragma comment(lib, "Ws2_32.lib")
using namespace std;

struct Client {
    int id;
    string name;
    SOCKET socket;
    thread th;
};

struct BSTNode {
    string username;
    string message;
    BSTNode* left;
    BSTNode* right;
    BSTNode(string user, string msg) : username(user), message(msg), left(nullptr), right(nullptr) {}
};

BSTNode* root = nullptr;

void insert_message(BSTNode*& root, const string& username, const string& message) {
    if (!root) {
        root = new BSTNode(username, message);
        return;
    }
    if (username < root->username)
        insert_message(root->left, username, message);
    else
        insert_message(root->right, username, message);
}

void display_messages(BSTNode* root) {
    if (!root)
        return;
    display_messages(root->left);
    cout << root->username << ": " << root->message << endl;
    display_messages(root->right);
}

vector<Client> clients;
unordered_map<string, string> credentials;
string def_col = "\033[0m";
string colors[] = {"\033[31m", "\033[32m", "\033[33m", "\033[34m", "\033[35m", "\033[36m"};
int seed = 0;
mutex cout_mtx, clients_mtx;

void load_credentials();
string color(int code);
void set_name(int id, char name[]);
void shared_print(string str, bool endLine);
int broadcast_message(string message, int sender_id);
void end_connection(int id);
void handle_client(SOCKET client_socket, int id);

int main() {
    load_credentials();

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed" << endl;
        return 1;
    }

    SOCKET server_socket;
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        cerr << "Socket creation failed" << endl;
        WSACleanup();
        return 1;
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(10000); // Make sure this matches the client
    server.sin_addr.s_addr = INADDR_ANY;
    memset(&server.sin_zero, 0, sizeof(server.sin_zero));

    if (bind(server_socket, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
        cerr << "Bind failed" << endl;
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    if (listen(server_socket, 8) == SOCKET_ERROR) {
        cerr << "Listen failed" << endl;
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    struct sockaddr_in client;
    SOCKET client_socket;
    int len = sizeof(sockaddr_in);

    cout << colors[NUM_COLORS - 1] << "\n\t  ====== Welcome to the chat-room ======   " << endl
         << def_col;

    while (1) {
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client, &len)) == INVALID_SOCKET) {
            cerr << "Accept failed" << endl;
            continue;
        }
        seed++;
        thread t(handle_client, client_socket, seed);
        {
            lock_guard<mutex> guard(clients_mtx);
            clients.push_back({seed, string("Anonymous"), client_socket, move(t)});
        }
    }

    for (auto& client : clients) {
        if (client.th.joinable())
            client.th.join();
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}

void load_credentials() {
    ifstream file("credentials.txt");
    if (!file) {
        cerr << "Error loading credentials." << endl;
        exit(-1);
    }
    string username, password;
    while (file >> username >> password) {
        credentials[username] = password;
    }
    file.close();
}

string color(int code) {
    return colors[code % NUM_COLORS];
}

void set_name(int id, char name[]) {
    lock_guard<mutex> guard(clients_mtx);
    for (auto& client : clients) {
        if (client.id == id) {
            client.name = string(name);
        }
    }
}

void shared_print(string str, bool endLine = true) {
    lock_guard<mutex> guard(cout_mtx);
    cout << str;
    if (endLine)
        cout << endl;
}

int broadcast_message(string message, int sender_id) {
    char temp[MAX_LEN];
    strcpy(temp, message.c_str());
    lock_guard<mutex> guard(clients_mtx);
    for (const auto& client : clients) {
        if (client.id != sender_id) {
            send(client.socket, temp, sizeof(temp), 0);
        }
    }
    return 0;
}

void end_connection(int id) {
    lock_guard<mutex> guard(clients_mtx);
    for (size_t i = 0; i < clients.size(); ++i) {
        if (clients[i].id == id) {
            clients[i].th.detach();
            closesocket(clients[i].socket);
            clients.erase(clients.begin() + i);
            break;
        }
    }
}

void handle_client(SOCKET client_socket, int id) {
    char name[MAX_LEN], str[MAX_LEN];
    recv(client_socket, name, sizeof(name), 0);
    set_name(id, name);

    string welcome_message = string(name) + string(" has joined");
    broadcast_message(welcome_message, id);
    shared_print(color(id) + welcome_message + def_col);

    while (1) {
        memset(str, 0, sizeof(str)); // Clear the buffer before use
        int bytes_received = recv(client_socket, str, sizeof(str), 0);
        if (bytes_received <= 0)
            return;

        if (strcmp(str, "#exit") == 0) {
            string exit_message = string(name) + string(" has left");
            broadcast_message(exit_message, id);
            shared_print(color(id) + exit_message + def_col);
            end_connection(id);
            return;
        }

        if (str[0] == '@') {
            string recipient, private_msg;
            stringstream ss(str + 1); // Skip the '@'
            ss >> recipient;         // Extract the recipient username

            // Extract the rest of the message, handling the possible leading space
            if (ss.peek() == ' ') {
                ss.ignore(); // Ignore the leading space after the username
            }
            getline(ss, private_msg);

            lock_guard<mutex> guard(clients_mtx);
            bool found = false;
            for (const auto& client : clients) {
                if (client.name == recipient) {
                    string full_message = string(name) + " (private): " + private_msg + "\033[31m";
                    send(client.socket, full_message.c_str(), full_message.length(), 0);
                    found = true;
                    break;
                }
            }

            if (!found) {
                string error_msg = "User " + recipient + " not found.";
                send(client_socket, error_msg.c_str(), error_msg.length(), 0);
            }
        } else {
            // Store the message in the BST
            insert_message(root, string(name), string(str));

            // Broadcast the message to all other clients with unique colors
            string full_message = color(id) + string(name) + def_col + ": " + string(str);
            broadcast_message(full_message, id);
            shared_print(full_message);
        }
    }
}
