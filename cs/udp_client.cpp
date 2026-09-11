#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    int client_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_fd == -1) {
        perror("socket");
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8081);

    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) != 1) {
        std::cerr << "invalid server ip\n";
        close(client_fd);
        return 1;
    }

    const char* msg = "Hello from UDP client";
    ssize_t sent = sendto(
        client_fd,
        msg,
        strlen(msg),
        0,
        reinterpret_cast<sockaddr*>(&server_addr),
        sizeof(server_addr)
    );

    if (sent == -1) {
        perror("sendto");
        close(client_fd);
        return 1;
    }

    char buffer[1024] = {0};
    sockaddr_in from_addr{};
    socklen_t from_len = sizeof(from_addr);

    ssize_t n = recvfrom(
        client_fd,
        buffer,
        sizeof(buffer) - 1,
        0,
        reinterpret_cast<sockaddr*>(&from_addr),
        &from_len
    );

    if (n == -1) {
        perror("recvfrom");
        close(client_fd);
        return 1;
    }

    buffer[n] = '\0';
    std::cout << "Server says: " << buffer << "\n";

    close(client_fd);
    return 0;
}
