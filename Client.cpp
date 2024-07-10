#include <iostream>
#include <string>
#include <thread>
#include <netinet/in.h>
#include <unistd.h>

void receive_messages(int socket) {
    char buffer[1024];
    while (true) {
        ssize_t bytes_received = recv(socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            std::cout << "Disconnected from server." << std::endl;
            break;
        }
        buffer[bytes_received] = '\0';
        std::cout << "Server: " << buffer << std::endl;
    }
}

int main() {
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server_address{};
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(12345);

    if (connect(client_socket, reinterpret_cast<sockaddr*>(&server_address), sizeof(server_address)) < 0) {
        std::cerr << "Connection failed." << std::endl;
        return 1;
    }

    std::cout << "Connected to server." << std::endl;
    std::thread(receive_messages, client_socket).detach();

    std::string message;
    while (true) {
        std::getline(std::cin, message);
        if (message == "/quit") {
            break;
        }
        send(client_socket, message.c_str(), message.length(), 0);
    }

    close(client_socket);
    return 0;
}
