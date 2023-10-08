#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 6969
#define BUFFER_SIZE 256

int main() {
    int server_socket;
    struct sockaddr_in server_addr, client_1_addr, client_2_addr;
    char buffer[BUFFER_SIZE];
    socklen_t addr_size = sizeof(struct sockaddr_in);

    // Create socket
    server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket == -1) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Binding error");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d\n", PORT);

    while (1) {
        // Receive client 1 input
        memset(buffer, 0, sizeof(buffer));
        recvfrom(server_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client_1_addr, &addr_size);
        char inputClient_1 = buffer[0];

        // Receive client 2 input
        memset(buffer, 0, sizeof(buffer));
        recvfrom(server_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client_2_addr, &addr_size);
        char inputClient_2 = buffer[0];

        // Game logic
        char resClient_1, resClient_2;
        if (inputClient_1 == inputClient_2) {
            resClient_1 = 'D'; // Draw
            resClient_2 = 'D'; // Draw
            printf("It's a draw.\n");
        } else if ((inputClient_1 == 'R' && inputClient_2 == 'S') ||
                   (inputClient_1 == 'S' && inputClient_2 == 'P') ||
                   (inputClient_1 == 'P' && inputClient_2 == 'R')) {
            resClient_1 = 'W'; // Win
            resClient_2 = 'L'; // Lose
            printf("Player one wins.\n");
        } else {
            resClient_1 = 'L'; // Lose
            resClient_2 = 'W'; // Win
            printf("Player two wins.\n");
        }

        // Send responses to clients
        sendto(server_socket, &resClient_1, sizeof(char), 0, (struct sockaddr*)&client_1_addr, addr_size);
        sendto(server_socket, &resClient_2, sizeof(char), 0, (struct sockaddr*)&client_2_addr, addr_size);
    }

    // Close the socket
    close(server_socket);

    return 0;
}
