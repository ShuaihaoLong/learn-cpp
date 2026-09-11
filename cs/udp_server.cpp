#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    int server_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_fd == -1) {
        perror("socket");
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8081);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    std::cout << "UDP server listening on port 8081...\n";

    char buffer[1024] = {0};
    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);

    ssize_t n = recvfrom(
        server_fd,
        buffer,
        sizeof(buffer) - 1,
        0,
        reinterpret_cast<sockaddr*>(&client_addr),
        &client_len
    );

    if (n == -1) {
        perror("recvfrom");
        close(server_fd);
        return 1;
    }

    buffer[n] = '\0';

    char client_ip[INET_ADDRSTRLEN] = {0};
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));

    std::cout << "Client " << client_ip << ":" << ntohs(client_addr.sin_port)
              << " says: " << buffer << "\n";

    const char* reply = "Hello from UDP server";
    ssize_t sent = sendto(
        server_fd,
        reply,
        strlen(reply),
        0,
        reinterpret_cast<sockaddr*>(&client_addr),
        client_len
    );

    if (sent == -1) {
        perror("sendto");
        close(server_fd);
        return 1;
    }

    close(server_fd);
    return 0;
}
