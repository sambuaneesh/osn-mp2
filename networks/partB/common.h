#ifndef COMMON_H
#define COMMON_H

#define MAX_CHUNK_SIZE 5
#define MAX_CHUNKS 100
#define TIMEOUT_SEC 0
#define TIMEOUT_USEC 100000

struct Packet {
    int seq_num;
    char data[MAX_CHUNK_SIZE];
};

#endif
