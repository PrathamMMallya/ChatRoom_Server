# C++ Multiclient Chatroom Application 

```
Built with Winsock2 , threads , and custom BST for message storage
```
## Why This Project?

In a world where rapid development with Python or Node.js is the norm, **this project proves that C++ is
still a powerful and relevant choice** for building real-time, scalable networking applications—especially
when you want **fine-grained control over memory, performance, and socket behavior**.

```
When people say “C++ is not meant for modern development” , this chatroom proves otherwise.
```
Multithreaded\ Private messaging\ In-memory message history using **Binary Search Tree (BST)** \
Secure credential check from file\ ANSI colored console output for better UX

## Features

```
Multiclient Chat Server using Winsock
Thread-per-client model using std::thread
User authentication via credentials.txt
Public and private messaging
BST message storage (per user)
Colored message output based on client ID
Clean separation of concerns with helper functions
```
## Tech Stack

```
Language: C++ (17 or above)
Platform: Windows (WinSock2 API)
Threads: std::thread
Socket APIs: WSAStartup, socket, bind, connect, recv, send, etc.
Build System: MSVC / g++ (on Windows)
```

## How to Run (Windows only)

### Step 1: Setup

```
Ensure you have a C++ compiler like MSVC or g++ (MinGW).
Install any IDE or run from command line (Visual Studio recommended).
```
### Step 2: Create credentials

Create a file named credentials.txt with the following format:
(Just to store username and password) 
You can give your own usernames and passwords
```
User1 pass123
User2 secretalways
```
### Step 3: Compile

**Server:**

```
g++server.cpp -o server-lws2_32-std=c++
```
**Client:**

```
g++client.cpp -o client-lws2_32-std=c++
```
### Step 4: Run

```
Open 1 terminal and run:
```
```
./server
```
```
Open multiple terminals for clients:
```
```
./client
```
## Usage

```
Enter your name when prompted.
Send messages to everyone.
To send private messages , use:
```
```
@username your message here
```
```
To exit , type:
```
```
#exit
```
## Message Storage (BST)

Each message is inserted into a **Binary Search Tree** using the sender’s name as the key. This allows for:

```
Sorted display by username
Efficient future enhancements like search history per user
```
While Python/Node.js simplify development, C++ offers:

```
Raw control over sockets
Thread-level control for concurrency
Zero dependency binary
Faster execution in low-latency environments
```
```
This project shows that you can still build robust real-time apps in C++ with complete control
and performance benefits.
```

## Credits

Made by **Pratham Mallya** \ For all those who believe **Performance and elegance can coexist** —even
in C++.



