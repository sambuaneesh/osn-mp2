#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define PORT 12345
#define BUFFER_SIZE 1024

void handleError(const char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

int setupServerSocket() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        handleError("Error creating socket");
    }
    return serverSocket;
}

void bindSocket(int serverSocket) {
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        handleError("Error setting socket option");
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        handleError("Error binding");
    }
}


int acceptClientConnection(int serverSocket) {
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
    if (clientSocket == -1) {
        handleError("Error accepting connection");
    }
    return clientSocket;
}

void handleClientCommunication(int clientSocket) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));

    // Receive data from client
    if (recv(clientSocket, buffer, sizeof(buffer), 0) == -1) {
        handleError("Error receiving data from client");
    }
    printf("Client says: %s\n", buffer);

    // Send response to client
    const char *response = "Hello from server!";
    if (send(clientSocket, response, strlen(response), 0) == -1) {
        handleError("Error sending data to client");
    }
}

int main() {
    int serverSocket = setupServerSocket();

    bindSocket(serverSocket);

    // Listen
    if (listen(serverSocket, 5) == -1) {
        handleError("Error listening");
    }

    printf("Server listening on port %d...\n", PORT);

    int clientSocket = acceptClientConnection(serverSocket);

    handleClientCommunication(clientSocket);

    // Close sockets
    if (close(clientSocket) == -1) {
        handleError("Error closing client socket");
    }
    if (close(serverSocket) == -1) {
        handleError("Error closing server socket");
    }

    return 0;
}
