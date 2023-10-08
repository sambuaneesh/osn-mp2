#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 12345
#define BUFFER_SIZE 1024

void handleError(const char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

int setupUDPClientSocket() {
    int clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSocket == -1) {
        handleError("Error creating socket");
    }
    return clientSocket;
}

struct sockaddr_in setupServerAddress() {
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    return serverAddr;
}

void communicateWithServer(int clientSocket, const struct sockaddr_in *serverAddr) {
    char buffer[BUFFER_SIZE];
    socklen_t serverAddrLen = sizeof(*serverAddr);

    // Send data to server
    strcpy(buffer, "Hello, server!");
    if (sendto(clientSocket, buffer, sizeof(buffer), 0, (struct sockaddr *)serverAddr, serverAddrLen) == -1) {
        handleError("Error sending data to server");
    }
    printf("Message sent to server: %s\n", buffer);

    // Receive response from server
    memset(buffer, 0, sizeof(buffer));
    if (recvfrom(clientSocket, buffer, sizeof(buffer), 0, (struct sockaddr *)serverAddr, &serverAddrLen) == -1) {
        handleError("Error receiving data from server");
    }
    printf("Server says: %s\n", buffer);
}

int main() {
    int clientSocket = setupUDPClientSocket();
    struct sockaddr_in serverAddr = setupServerAddress();

    communicateWithServer(clientSocket, &serverAddr);

    // Close socket
    if (close(clientSocket) == -1) {
        handleError("Error closing socket");
    }

    return 0;
}
