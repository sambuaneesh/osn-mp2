#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#include "common.h"

void send_ack(int sockfd, int seq_num, struct sockaddr_in sender_addr)
{
    sendto(sockfd, &seq_num, sizeof(int), 0, (struct sockaddr *)&sender_addr, sizeof(sender_addr));
}

void receive_data_packets(int sockfd)
{
    struct Packet received_packets[MAX_CHUNKS];
    int ack_status[MAX_CHUNKS] = {0};
    struct sockaddr_in sender_addr;

    int highest_seq_num = -1;
    int total_chunks = 0;

    srand(time(NULL)); // Seed for random number generation

    while (1)
    {
        struct Packet received_packet;
        socklen_t sender_len = sizeof(sender_addr);

        if (recvfrom(sockfd, &received_packet, sizeof(struct Packet), 0, (struct sockaddr *)&sender_addr, &sender_len) > 0)
        {
            if (received_packet.seq_num == -1)
            {
                // Termination signal received, exit the loop
                break;
            }

            // Check if the packet is already acknowledged
            if (ack_status[received_packet.seq_num]) {
                // printf("Duplicate chunk %d received and ignored.\n", received_packet.seq_num);
                continue; // Skip the packet, it's a duplicate
            }

            total_chunks++; // Increment total chunks received
            // Skip every third packet
            // if (total_chunks % 3 == 0) {
            //     printf("Skipping chunk %d.\n", received_packet.seq_num);
            //     continue; // Skip the packet, don't acknowledge
            // }

            ack_status[received_packet.seq_num] = 1; // Mark the packet as acknowledged
            highest_seq_num = received_packet.seq_num; // Update the highest sequence number
            received_packets[highest_seq_num] = received_packet;

            printf("Received chunk %d: %s\n", highest_seq_num, received_packet.data);
            send_ack(sockfd, highest_seq_num, sender_addr); // Send acknowledgment for the current packet
        }
    }

    // Reconstruct the original data
    char decoded_message[MAX_CHUNKS * MAX_CHUNK_SIZE];
    memset(decoded_message, 0, sizeof(decoded_message));

    for (int i = 0; i <= total_chunks; ++i)
    {
        strncat(decoded_message, received_packets[i].data, MAX_CHUNK_SIZE);
    }

    printf("Received termination signal. Decoded Message: %s\n", decoded_message);
}

int main()
{
    int sockfd;
    struct sockaddr_in receiver_addr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    receiver_addr.sin_family = AF_INET;
    receiver_addr.sin_port = htons(12345);
    receiver_addr.sin_addr.s_addr = INADDR_ANY;

    bind(sockfd, (struct sockaddr *)&receiver_addr, sizeof(receiver_addr));

    printf("Receiver waiting for data...\n");

    receive_data_packets(sockfd);

    close(sockfd);
    return 0;
}
