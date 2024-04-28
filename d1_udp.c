#include "d1_udp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

// Helper function to calculate checksum
static uint16_t calculate_checksum(const void *data, size_t len) {
    const uint8_t *bytes = data;
    uint16_t checksum = 0;
    for (size_t i = 0; i < len; i++) {
        checksum ^= bytes[i];
    }
    return checksum;
}

// Initialize the client UDP socket and return the structure
D1Peer* d1_create_client() {
    D1Peer *peer = malloc(sizeof(D1Peer));
    if (peer == NULL) {
        perror("Failed to allocate D1Peer");
        return NULL;
    }

    peer->socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (peer->socket < 0) {
        perror("Failed to create UDP socket");
        free(peer);
        return NULL;
    }

    // Non-blocking mode may be used or not depending on your specific requirement
    // fcntl(peer->socket, F_SETFL, O_NONBLOCK);

    peer->next_seqno = 0;
    memset(&peer->server_addr, 0, sizeof(peer->server_addr));
    return peer;
}

// Clean up and close the socket
D1Peer* d1_delete(D1Peer* peer) {
    if (peer) {
        close(peer->socket);
        free(peer);
    }
    return NULL;
}

// Set server details into the peer structure
int d1_get_peer_info(D1Peer* peer, const char* servername, uint16_t server_port) {
    struct hostent *server = gethostbyname(servername);
    if (server == NULL) {
        fprintf(stderr, "No such host named %s\n", servername);
        return 0;
    }

    peer->server_addr.sin_family = AF_INET;
    memcpy(&peer->server_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    peer->server_addr.sin_port = htons(server_port);

    return 1;
}

// Send data with D1 header
int d1_send_data(D1Peer* peer, char* buffer, size_t sz) {
    if (sz + sizeof(D1Header) > 1024) {
        fprintf(stderr, "Data too large to send\n");
        return -1;
    }

    D1Header header;
    header.flags = htons(FLAG_DATA | (peer->next_seqno << 7));
    header.size = htons(sz + sizeof(D1Header));
    header.checksum = calculate_checksum(buffer, sz);

    char packet[1024];
    memcpy(packet, &header, sizeof(D1Header));
    memcpy(packet + sizeof(D1Header), buffer, sz);

    ssize_t sent = sendto(peer->socket, packet, sz + sizeof(D1Header), 0,
                          (struct sockaddr*)&peer->server_addr, sizeof(peer->server_addr));
    if (sent < 0) {
        perror("Failed to send data");
        return -1;
    }

    return d1_wait_ack(peer, buffer, sz);
}

// Receive data and strip D1 header
int d1_recv_data(D1Peer* peer, char* buffer, size_t sz) {
    char packet[1024];
    struct sockaddr_in from;
    socklen_t fromlen = sizeof(from);

    ssize_t recvd = recvfrom(peer->socket, packet, 1024, 0, (struct sockaddr*)&from, &fromlen);
    if (recvd < 0) {
        perror("Failed to receive data");
        return -1;
    }

    D1Header *header = (D1Header *)packet;
    if (ntohs(header->size) != recvd) {
        fprintf(stderr, "Packet size mismatch\n");
        return -1;
    }

    if (header->checksum != calculate_checksum(packet + sizeof(D1Header), recvd - sizeof(D1Header))) {
        fprintf(stderr, "Checksum error\n");
        return -1;
    }

    if (ntohs(header->flags) & FLAG_ACK) {
        return 0; // Just an ACK
    }

    size_t payload_size = recvd - sizeof(D1Header);
    if (payload_size > sz) {
        fprintf(stderr, "Buffer too small\n");
        return -1;
    }

    memcpy(buffer, packet + sizeof(D1Header), payload_size);
    return payload_size;
}

// Send an ACK for a given sequence number
void d1_send_ack(D1Peer* peer, int seqno) {
    D1Header header;
    header.flags = htons(FLAG_ACK | (seqno & ACKNO));
    header.size = htons(sizeof(D1Header));
    header.checksum = 0;  // No data, checksum is trivial

    sendto(peer->socket, &header, sizeof(header), 0, (struct sockaddr*)&peer->server_addr, sizeof(peer->server_addr));
}

// Wait for an ACK and handle retransmission if necessary
int d1_wait_ack(D1Peer* peer, char* buffer, size_t sz) {
    struct timeval timeout = {1, 0}; // 1 second timeout
    setsockopt(peer->socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));

    D1Header ack;
    ssize_t recv_sz = recv(peer->socket, &ack, sizeof(D1Header), 0);
    if (recv_sz < 0 || ntohs(ack.flags) & FLAG_ACK == 0 || ntohs(ack.flags) & ACKNO != peer->next_seqno) {
        fprintf(stderr, "Failed to receive correct ACK, resending data\n");
        return d1_send_data(peer, buffer, sz);
    }

    peer->next_seqno ^= 1; // Toggle sequence number for next transmission
    return sz;
}
