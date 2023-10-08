#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 12345
#define BUFFER_SIZE 1024

void sendMessage(int socket, const char *message) {
    if (send(socket, message, strlen(message), 0) == -1) {
        perror("Error sending data to server");
        close(socket);
        exit(EXIT_FAILURE);
    }
}

void receiveMessage(int socket, char *buffer) {
    if (recv(socket, buffer, BUFFER_SIZE, 0) == -1) {
        perror("Error receiving data from server");
        close(socket);
        exit(EXIT_FAILURE);
    }
}

int main() {
    int clientSocket;
    struct sockaddr_in serverAddr;
    char buffer[BUFFER_SIZE];

    // Create socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Setup server address structure
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Connect to server
    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Error connecting to server");
        close(clientSocket);
        exit(EXIT_FAILURE);
    }

    // Send data to server
    char message[] = "Hello, server!";
    sendMessage(clientSocket, message);
    printf("Message sent to server: %s\n", message);

    // Receive response from server
    receiveMessage(clientSocket, buffer);
    printf("Server says: %s\n", buffer);

    // Close socket
    if (close(clientSocket) == -1) {
        perror("Error closing socket");
        exit(EXIT_FAILURE);
    }

    return 0;
}
