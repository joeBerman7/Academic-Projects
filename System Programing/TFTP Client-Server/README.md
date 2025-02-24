# Extended TFTP Protocol with Multithreading

## Introduction

In this project, you will implement an **extended version of the TFTP (Trivial File Transfer Protocol)**, allowing clients to interact with the server for **file uploading, downloading, and file management**. The communication between the server and the client(s) will be carried out using a **binary communication protocol**, with a focus on **multithreading** to handle multiple clients concurrently.

### Why Multithreading?

**Multithreading** in network protocols is essential for handling multiple requests simultaneously. Without multithreading, a server can only handle one client at a time, which drastically reduces its ability to serve multiple clients concurrently. By utilizing threads, the server can handle **multiple users, file transfers, and commands** at the same time, making the system more efficient and scalable.

In this assignment, you will be using a **Thread-Per-Client (TPC)** server model, where each client connection is handled by a separate thread, enabling the server to interact with multiple clients simultaneously. This method improves the overall performance of the server and ensures a responsive and concurrent system.

---

## Project Overview

This project simulates a **file transfer system** where users can upload, download, and manage files. Users interact with the server using a **Login command** to set their username, which must be unique and cannot be changed after it is set. Once logged in, users can submit commands related to file transfer and file management.

### Server Design and Multithreading

The **server** in this system will use **TCP sockets** and be capable of handling multiple connections using **multithreading**. The server will implement the **Thread-Per-Client (TPC)** model, meaning that each client request is handled in its own thread.

1. **Multithreaded Handling**: Every time the server receives a message from a client, it creates a new thread to handle the request. This allows the server to handle multiple clients at once, efficiently managing concurrent file transfers and commands.
   
2. **Bi-directional Communication**: While the original TFTP protocol uses a simple request-response mechanism, this extended version supports **bi-directional communication** between clients and the server. It also enables broadcasting messages to all connected clients.

3. **File Operations**: Users will be able to upload, download, and look up files stored on the server. The server will maintain an updated list of files, and any changes (uploads or deletions) will be communicated to the connected clients.

---

## Features

- **Login Command**: Clients must send a **Login command** with a unique username. The server checks the validity of the username and allows the user to proceed with file operations once logged in.
- **File Operations**: Clients can upload files, download files, and look up available files on the server. The server keeps track of all files and their status.
- **Multithreading**: Each client interaction is handled in a separate thread, ensuring the server can handle multiple clients concurrently without blocking.
- **Bi-directional Communication**: Clients can send messages to each other or receive broadcasted announcements, enabling more interactive and dynamic interactions between clients.
- **Extended TFTP Protocol**: Unlike the standard TFTP, which uses UDP, this implementation uses TCP for reliable communication, allowing more sophisticated interactions.

---

## Why Multithreading is Important for This Project

In the context of network protocols, **multithreading** allows multiple tasks to run concurrently, which is essential when handling several clients simultaneously. For this project:

1. **Concurrent Client Handling**: Each client needs to be able to communicate with the server independently and without delays. By assigning a separate thread to each client, the server can handle multiple clients concurrently without blocking any user or operation.
   
2. **Efficient File Transfers**: The server may be handling file uploads, downloads, and other requests at the same time. By using multithreading, the server can ensure that each file transfer happens independently, without slowing down other clients.
   
3. **Server Scalability**: With multiple threads, the server can handle a growing number of clients without performance degradation, making it highly scalable.

### How Multithreading Works in This Project

In this project, the **Thread-Per-Client (TPC)** model will be used. This means that for each incoming client request:

- A new thread is created to handle that specific client.
- The client communicates with the server over a TCP socket.
- Once the task is completed (e.g., file upload or download), the thread is terminated.

This ensures that each client's requests are handled independently and concurrently, without any interference with other clients' requests.

---

## Implementation Details

### 1. **Server Implementation**:
- **Thread Management**: The server listens for incoming connections from clients. For each connection, it creates a new thread to handle the specific client’s requests.
- **File Handling**: The server processes file upload, download, and lookup requests. It maintains a list of available files and manages the file transfer process.
- **Bi-directional Communication**: The server supports sending messages between clients or broadcasting announcements to all connected clients.
- **User Authentication**: Upon receiving a **Login command**, the server verifies that the username is unique and valid. Once logged in, clients can interact with the server to perform file operations.

### 2. **Client Implementation**:
- **Commands**: The client sends commands to the server, including the **Login** command to authenticate, and file-related commands to upload, download, or list files.
- **Server Communication**: The client communicates with the server via a TCP connection, and the server responds to client requests, sending the necessary data (e.g., file contents or status updates).

---
