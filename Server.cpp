#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <algorithm>
#include <mutex>
#include <netinet/in.h>
#include <unistd.h>

std::vector<int> clients;
std::mutex clients_mutex;

void handle_client(int client_socket) {
    char buffer[1024];
    while (true) {
        ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients.erase(std::remove(clients.begin(), clients.end(), client_socket), clients.end());
            close(client_socket);
            break;
        }
        buffer[bytes_received] = '\0';

        std::lock_guard<std::mutex> lock(clients_mutex);
        for (int client : clients) {
            if (client != client_socket) {
                send(client, buffer, bytes_received, 0);
            }
        }
    }
}

int main() {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server_address{};
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(12345);

    bind(server_socket, reinterpret_cast<sockaddr*>(&server_address), sizeof(server_address));
    listen(server_socket, 5);

    std::cout << "Server started on port 12345" << std::endl;

    while (true) {
        int client_socket = accept(server_socket, nullptr, nullptr);
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients.push_back(client_socket);
        std::thread(handle_client, client_socket).detach();
    }

    close(server_socket);
    return 0;
}
