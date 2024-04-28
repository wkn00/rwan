#include "d2_lookup.h"
#include "d1_udp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

// Helper function to create and manage connections to the server
D2Client* d2_client_create(const char* server_name, uint16_t server_port) {
    D2Client *client = malloc(sizeof(D2Client));
    if (!client) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    client->peer = d1_create_client();
    if (!client->peer || !d1_get_peer_info(client->peer, server_name, server_port)) {
        fprintf(stderr, "Failed to set up UDP client\n");
        if (client->peer) {
            d1_delete(client->peer);
        }
        free(client);
        return NULL;
    }

    return client;
}

// Function to delete client and free resources
D2Client* d2_client_delete(D2Client* client) {
    if (client) {
        d1_delete(client->peer);
        free(client);
    }
    return NULL;
}

// Function to send a directory request to the server
int d2_send_request(D2Client* client, uint32_t id) {
    PacketRequest req;
    req.type = htons(TYPE_REQUEST);
    req.id = htonl(id);

    if (d1_send_data(client->peer, (char *)&req, sizeof(req)) < 0) {
        fprintf(stderr, "Failed to send request\n");
        return -1;
    }
    return 1;
}

// Function to receive the initial response size packet
int d2_recv_response_size(D2Client* client) {
    char buffer[1024];
    int len = d1_recv_data(client->peer, buffer, sizeof(buffer));
    if (len <= 0) {
        return -1;
    }

    PacketResponseSize *resp_size = (PacketResponseSize *)buffer;
    if (ntohs(resp_size->type) != TYPE_RESPONSE_SIZE) {
        fprintf(stderr, "Incorrect packet type\n");
        return -1;
    }

    return ntohs(resp_size->size);
}

// Function to receive the actual data packets containing tree nodes
int d2_recv_response(D2Client* client, char* buffer, size_t sz) {
    return d1_recv_data(client->peer, buffer, sz);
}

// Function to allocate memory for the tree structure
LocalTreeStore* d2_alloc_local_tree(int num_nodes) {
    LocalTreeStore *tree = malloc(sizeof(LocalTreeStore) + num_nodes * sizeof(NetNode));
    if (!tree) {
        fprintf(stderr, "Memory allocation failed for tree nodes\n");
        return NULL;
    }

    tree->nodes = (NetNode *)(tree + 1);
    tree->size = num_nodes;
    tree->count = 0;
    return tree;
}

// Function to free the memory allocated for the tree
void d2_free_local_tree(LocalTreeStore* nodes) {
    free(nodes);
}

// Function to add nodes to the local tree
int d2_add_to_local_tree(LocalTreeStore* nodes, int node_idx, char* buffer, int buflen) {
    while (buflen > 0) {
        if (buflen < sizeof(NetNode)) {
            fprintf(stderr, "Buffer size is too small for NetNode\n");
            return -1;
        }

        NetNode *node = (NetNode *)buffer;
        if (node_idx >= nodes->size) {
            fprintf(stderr, "Local tree store is full\n");
            return -1;
        }

        nodes->nodes[node_idx++] = *node;
        nodes->count++;
        buffer += sizeof(NetNode);
        buflen -= sizeof(NetNode);
    }
    return node_idx;
}

// Function to print the local tree
void d2_print_tree(LocalTreeStore* nodes) {
    for (int i = 0; i < nodes->count; i++) {
        NetNode *node = &nodes->nodes[i];
        printf("id %u value %u children %u\n", node->id, node->value, node->num_children);
        for (uint32_t j = 0; j < node->num_children; j++) {
            printf("-- id %u\n", node->child_id[j]);
        }
    }
}
