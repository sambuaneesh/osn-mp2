#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 6969
#define BUFFER_SIZE 256

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char input, playAgain = 'Y';

    // Create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to the server
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connection failed");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    while (playAgain == 'Y') {
        // Prompt user for input (R, P, or S)
        printf("Enter your choice (R for Rock, P for Paper, S for Scissors): ");
        scanf(" %c", &input);

        // Validate user input
        while (input != 'R' && input != 'P' && input != 'S') {
            printf("Invalid input. Please enter R, P, or S: ");
            scanf(" %c", &input);
        }

        // Send user input to the server
        send(client_socket, &input, sizeof(char), 0);

        // Receive and display response from the server
        memset(buffer, 0, sizeof(buffer));
        recv(client_socket, buffer, BUFFER_SIZE, 0);
        printf("Response from server: %s\n", buffer);

        // Prompt user for another game
        printf("Do you want to play again? (Y/N)\n");
        scanf(" %c", &playAgain);

        // Validate user input
        while (playAgain != 'Y' && playAgain != 'N') {
            printf("Invalid input. Please enter Y or N: ");
            scanf(" %c", &playAgain);
        }

        // Send play again decision to the server
        send(client_socket, &playAgain, sizeof(char), 0);
    }

    // Close the socket
    close(client_socket);

    return 0;
}
