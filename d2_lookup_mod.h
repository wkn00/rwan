/* ======================================================================
 * YOU CAN MODIFY THIS FILE.
 * ====================================================================== */

#ifndef D2_LOOKUP_MOD_H
#define D2_LOOKUP_MOD_H

#include "d1_udp.h"
#include "stdint.h" // For uint32_t

struct D2Client
{
    D1Peer* peer;
};

typedef struct D2Client D2Client;

struct LocalTreeStore {
    NetNode *nodes;          // Pointer to dynamically allocated array of nodes
    int number_of_nodes;     // Total number of nodes in the tree
    int current_count;       // Current count of nodes stored
    int status;              // For error handling or status indication
};

struct NetNode {
    uint32_t id;
    uint32_t value;
    uint32_t num_children;
    uint32_t child_id[5]; // Can be adjusted based on maximum expected children
};

typedef struct LocalTreeStore LocalTreeStore;

#endif /* D2_LOOKUP_MOD_H */

