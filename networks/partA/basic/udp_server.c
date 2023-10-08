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

int setupUDPServerSocket() {
    int serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket == -1) {
        handleError("Error creating socket");
    }
    return serverSocket;
}

void bindUDPSocket(int serverSocket) {
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        handleError("Error binding");
    }
}

void handleUDPClientCommunication(int serverSocket) {
    char buffer[BUFFER_SIZE];
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    memset(buffer, 0, sizeof(buffer));

    // Receive data from client
    if (recvfrom(serverSocket, buffer, sizeof(buffer), 0, (struct sockaddr *)&clientAddr, &clientAddrLen) == -1) {
        handleError("Error receiving data from client");
    }
    printf("Client says: %s\n", buffer);

    // Send response to client
    const char *response = "Hello from server!";
    if (sendto(serverSocket, response, strlen(response), 0, (struct sockaddr *)&clientAddr, clientAddrLen) == -1) {
        handleError("Error sending data to client");
    }
}

int main() {
    int serverSocket = setupUDPServerSocket();

    bindUDPSocket(serverSocket);

    printf("Server listening on port %d...\n", PORT);

    handleUDPClientCommunication(serverSocket);

    // Close socket
    if (close(serverSocket) == -1) {
        handleError("Error closing socket");
    }

    return 0;
}
