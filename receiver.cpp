#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdlib>
#include "crc.h"

#define SERVER_IP "127.0.0.43"
#define SERVER_PORT 5000

// Function to flip one random bit in the message string
void flipOneBit(std::string& msg) {
    if (msg.empty()) return;
    size_t bitPos = rand() % (msg.size() * 8);
    size_t bytePos = bitPos / 8;
    size_t bitInByte = bitPos % 8;

    msg[bytePos] ^= (1 << bitInByte);
}

int main() {
    srand(time(nullptr)); // seed random

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        return 1;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &addr.sin_addr) <= 0) {
        std::cerr << "Invalid address / Address not supported\n";
        return 1;
    }

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Bind failed");
        return 1;
    }

    if (listen(server_fd, 1) < 0) {
        perror("Listen failed");
        return 1;
    }

    std::cout << "Waiting for connection on " << SERVER_IP << ":" << SERVER_PORT << "...\n";

    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len);
    if (client_fd < 0) {
        perror("Accept failed");
        return 1;
    }

    char buffer[1024];
    ssize_t len = recv(client_fd, buffer, sizeof(buffer), 0);
    if (len <= 0) {
        std::cerr << "Receive failed or connection closed\n";
        close(client_fd);
        close(server_fd);
        return 1;
    }

    std::string data(buffer, len);
    if (data.size() < 1) {
        std::cerr << "No data received\n";
        close(client_fd);
        close(server_fd);
        return 1;
    }

    // Separate message and CRC
    char received_crc = data.back();
    std::string message = data.substr(0, data.size() - 1);

    uint8_t calc_crc = computeCRC(message);

    std::cout << "Received message: " << message << "\n";
    std::cout << "Received CRC: 0x" << std::hex << (unsigned char)received_crc
              << ", Calculated CRC: 0x" << (int)calc_crc << std::dec << "\n";

    // Ask user whether to flip a bit or not
    std::cout << "Select option:\n1. No error (check normal)\n2. Flip one bit (simulate error)\nChoice: ";
    int choice;
    std::cin >> choice;
    std::cin.ignore(); // clear newline

    switch(choice) {
        case 1: {
            if ((unsigned char)received_crc == calc_crc) {
                std::cout << "✅ CRC matched. Message is valid.\n";
                send(client_fd, "ACK", 3, 0);
            } else {
                std::cout << "❌ CRC mismatch! Message invalid.\n";
                send(client_fd, "NAK", 3, 0);
            }
            break;
        }

        case 2: {
            flipOneBit(message);
            std::cout << "Message after flipping 1 bit: " << message << "\n";
            uint8_t crc_flipped = computeCRC(message);
            std::cout << "New CRC after flipping: 0x" << std::hex << (int)crc_flipped << std::dec << "\n";

            if ((unsigned char)received_crc == calc_crc) {
                std::cout << "But original CRC doest not matches CRC after error generation, so message is INVALID (bit flip simulated).\n";
                send(client_fd, "NAK", 3, 0);
            } else {
                std::cout << "CRC mismatch after bit flip. Invalid message.\n";
                send(client_fd, "NAK", 3, 0);
            }
            break;
        }

        default: {
            std::cout << "Invalid choice. Closing connection.\n";
            send(client_fd, "NAK", 3, 0);
            break;
        }
    }

    close(client_fd);
    close(server_fd);
    return 0;
}
