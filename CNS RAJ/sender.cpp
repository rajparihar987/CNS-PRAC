#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "crc.h"

#define SERVER_IP "127.0.0.43"
#define SERVER_PORT 5000

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr) <= 0) {
        std::cerr << "Invalid address / Address not supported\n";
        return 1;
    }

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Connect failed");
        return 1;
    }

    std::string message;
    std::cout << "Enter message to send: ";
    std::getline(std::cin, message);

    uint8_t crc = computeCRC(message);
    std::string packet = message + static_cast<char>(crc);

    while (true) {
        ssize_t sent = send(sock, packet.c_str(), packet.size(), 0);
        if (sent < 0) {
            perror("Send failed");
            close(sock);
            return 1;
        }
        std::cout << "Sent message + CRC (0x" << std::hex << (int)crc << std::dec << ")\n";

        char response[4] = {0};
        ssize_t recvd = recv(sock, response, sizeof(response) - 1, 0);
        if (recvd <= 0) {
            std::cout << "No response or connection closed\n";
            break;
        }
        std::string resp(response, recvd);

        if (resp == "ACK") {
            std::cout << "ACK received. Transmission successful.\n";
            break;
        } else if (resp == "NAK") {
            std::cout << "NAK received. Resending message...\n";
            // loop continues to resend
        } else {
            std::cout << "Unknown response: " << resp << ", resending...\n";
        }
    }

    close(sock);
    return 0;
}
