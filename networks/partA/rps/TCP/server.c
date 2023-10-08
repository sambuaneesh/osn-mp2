#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 6969
#define BUFFER_SIZE 256

int main() {
    int welcome_socket, client_1, client_2;
    struct sockaddr_in server_addr, client_1_addr, client_2_addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    char buffer[BUFFER_SIZE];

    // Create socket
    welcome_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (welcome_socket == -1) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket
    if (bind(welcome_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Binding error");
        close(welcome_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(welcome_socket, 2) == -1) {
        perror("Listening error");
        close(welcome_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d\n", PORT);

    while (1) {
        // Accept client 1
        client_1 = accept(welcome_socket, (struct sockaddr*)&client_1_addr, &addr_size);
        if (client_1 == -1) {
            perror("Acceptance error for client 1");
            close(welcome_socket);
            exit(EXIT_FAILURE);
        }

        printf("Player one connected.\n");

        // Accept client 2
        client_2 = accept(welcome_socket, (struct sockaddr*)&client_2_addr, &addr_size);
        if (client_2 == -1) {
            perror("Acceptance error for client 2");
            close(welcome_socket);
            exit(EXIT_FAILURE);
        }

        printf("Player two connected.\n");

        // Game loop
        char playAgainClient_1 = 'Y', playAgainClient_2 = 'Y';
        while (playAgainClient_1 == 'Y' && playAgainClient_2 == 'Y') {
            // Get client inputs
            recv(client_1, buffer, BUFFER_SIZE, 0);
            char inputClient_1 = buffer[0];
            memset(buffer, 0, sizeof(buffer));
            recv(client_2, buffer, BUFFER_SIZE, 0);
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

            // Send responses
            send(client_1, &resClient_1, sizeof(char), 0);
            send(client_2, &resClient_2, sizeof(char), 0);

            // Prompt clients for another game
            printf("Do you want to play again? (Y/N)\n");
            recv(client_1, &playAgainClient_1, sizeof(char), 0);
            recv(client_2, &playAgainClient_2, sizeof(char), 0);
        }

        // Close client sockets
        close(client_1);
        close(client_2);

        printf("Waiting for new players...\n");
    }

    // Close the welcome socket
    close(welcome_socket);

    return 0;
}
