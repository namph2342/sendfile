#include <iostream>
#include <winsock2.h>
#include <map>
#include <thread>
#include <mutex>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 1024

struct ClientInfo {
    char ip[16];
    int port;
};

std::map<std::string, ClientInfo> client_info;
std::mutex client_info_mutex;

void error(const char* msg) {
    perror(msg);
    exit(1);
}

void handle_client(SOCKET client_socket) {
    ClientInfo client_info_buffer;
    int valread;

    // Read client info
    valread = recv(client_socket, (char*)&client_info_buffer, sizeof(ClientInfo), 0);
    if (valread <= 0) {
        closesocket(client_socket);
        return;
    }

    std::string client_ip_port = std::string(client_info_buffer.ip) + ":" + std::to_string(client_info_buffer.port);

    {
        std::lock_guard<std::mutex> guard(client_info_mutex);
        client_info[client_ip_port] = client_info_buffer;
    }

    // Keep connection open to receive further requests
    char buffer[BUFFER_SIZE] = { 0 };
    while ((valread = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        // Process requests (e.g., list clients, request file transfer)
    }

    closesocket(client_socket);
}

void start_server() {
    WSADATA wsa;
    SOCKET server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        error("WSAStartup failed");
    }

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        error("Socket creation error");
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        error("Bind failed");
    }

    if (listen(server_fd, 3) < 0) {
        error("Listen failed");
    }

    while (true) {
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (int*)&addrlen)) < 0) {
            error("Accept failed");
        }

        std::thread(handle_client, new_socket).detach();
    }

    closesocket(server_fd);
    WSACleanup();
}

int main() {
    start_server();
    return 0;
}
