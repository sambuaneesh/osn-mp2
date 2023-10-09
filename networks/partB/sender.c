// Sending Different Packets
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <fcntl.h>

#include "common.h"

int main() {
    int sockfd;
    struct sockaddr_in receiver_addr;
    struct Packet packet;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    receiver_addr.sin_family = AF_INET;
    receiver_addr.sin_port = htons(12345);
    receiver_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    char message[] = "East or West KV sir is the best!";
    int total_chunks = (strlen(message) + MAX_CHUNK_SIZE - 1) / MAX_CHUNK_SIZE;

    printf("Total Chunks: %d\n", total_chunks);
    printf("Chunk Size: %d\n", MAX_CHUNK_SIZE);

    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK); // Set socket to non-blocking mode

    int seq_num = 0;
    int resend = 1;
    int ack_received[MAX_CHUNKS] = {0}; // Array to track acknowledgment status

    while (1) { // Run infinitely until all packets are acknowledged
        int any_unacknowledged = 0; // Flag to check if any unacknowledged packets exist

        for (int i = 0; i < total_chunks; ++i) {
            if (!ack_received[i]) {
                any_unacknowledged = 1;
                packet.seq_num = i;
                strncpy(packet.data, message + i * MAX_CHUNK_SIZE, MAX_CHUNK_SIZE);
                sendto(sockfd, &packet, sizeof(struct Packet), 0, (struct sockaddr *)&receiver_addr, sizeof(receiver_addr));
                printf("Sent chunk %d: %s\n", i, packet.data);
                // sleep for 0.5 seconds
                usleep(10000);
            }
        }

        if (!any_unacknowledged) {
            // All packets have been acknowledged, send termination signal
            packet.seq_num = -1;
            sendto(sockfd, &packet, sizeof(struct Packet), 0, (struct sockaddr *)&receiver_addr, sizeof(receiver_addr));
            printf("Sent termination signal.\n");
            break; // Exit the loop as all packets are acknowledged
        }

        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000; // 0.1 seconds timeout

        int activity = select(sockfd + 1, &readfds, NULL, NULL, &timeout);

        if (activity < 0) {
            perror("select error");
            break;
        } else if (activity > 0 && FD_ISSET(sockfd, &readfds)) {
            // Received an acknowledgment
            recvfrom(sockfd, &packet, sizeof(struct Packet), 0, NULL, NULL);
            printf("Received acknowledgment for chunk %d\n", packet.seq_num);
            // continue;
            usleep(100000);
            ack_received[packet.seq_num] = 1;
        }
    }

    close(sockfd);
    return 0;
}

// Sending Same Packet Multiple Times
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <arpa/inet.h>
// #include <sys/time.h>
// #include <sys/types.h>
// #include <sys/select.h>
// #include <fcntl.h>

// #include "common.h"

// int main()
// {
//     int sockfd;
//     struct sockaddr_in receiver_addr;
//     struct Packet packet;

//     sockfd = socket(AF_INET, SOCK_DGRAM, 0);

//     receiver_addr.sin_family = AF_INET;
//     receiver_addr.sin_port = htons(12345);
//     receiver_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

//     char message[] = "East or West KV sir is the best!";
//     int total_chunks = (strlen(message) + MAX_CHUNK_SIZE - 1) / MAX_CHUNK_SIZE;

//     printf("Total Chunks: %d\n", total_chunks);
//     printf("Chunk Size: %d\n", MAX_CHUNK_SIZE);

//     int flags = fcntl(sockfd, F_GETFL, 0);
//     fcntl(sockfd, F_SETFL, flags | O_NONBLOCK); // Set socket to non-blocking mode

//     int seq_num = 0;
//     int resend = 1;

//     while (seq_num < total_chunks || resend)
//     {
//         if (resend || (seq_num < total_chunks))
//         {
//             packet.seq_num = seq_num;
//             strncpy(packet.data, message + seq_num * MAX_CHUNK_SIZE, MAX_CHUNK_SIZE);

//             sendto(sockfd, &packet, sizeof(struct Packet), 0, (struct sockaddr *)&receiver_addr, sizeof(receiver_addr));
//             printf("Sent chunk %d: %s\n", seq_num, packet.data);

//             resend = 0;
//         }

//         fd_set readfds;
//         FD_ZERO(&readfds);
//         FD_SET(sockfd, &readfds);

//         struct timeval timeout;
//         timeout.tv_sec = 0;
//         timeout.tv_usec = 100000; // 0.1 seconds timeout

//         int activity = select(sockfd + 1, &readfds, NULL, NULL, &timeout);

//         if (activity < 0)
//         {
//             perror("select error");
//             break;
//         }
//         else if (activity > 0 && FD_ISSET(sockfd, &readfds))
//         {
//             // Received an acknowledgment
//             recvfrom(sockfd, &packet, sizeof(struct Packet), 0, NULL, NULL);
//             printf("Received acknowledgment for chunk %d\n", packet.seq_num);
//             seq_num++;
//             resend = 0;
//         }
//         else
//         {
//             // Timeout, need to resend the current chunk
//             resend = 1;
//         }
//     }

//     // Send termination signal to indicate end of data
//     packet.seq_num = -1; // Special sequence number indicating termination
//     sendto(sockfd, &packet, sizeof(struct Packet), 0, (struct sockaddr *)&receiver_addr, sizeof(receiver_addr));
//     printf("Sent termination signal.\n");

//     close(sockfd);
//     return 0;
// }
