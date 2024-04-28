Directory Service Client Implementation
=======================================

Overview:
---------
This implementation provides a client library for a directory service based on UDP. It consists of two main components:
1. A lower layer (`d1_udp.c`) handling UDP communication including sending, receiving, and acknowledgment of data packets.
2. An upper layer (`d2_lookup.c`) that manages directory requests and processes tree-structured directory data from the server.

Both components utilize structures defined and potentially modified in `d1_udp_mod.h` and `d2_lookup_mod.h` to manage state and data.

Files Included:
---------------
- `d1_udp.c`: Implements the UDP communication protocols.
- `d1_udp.h`: Header file with function prototypes and definitions for UDP communication.
- `d1_udp_mod.h`: Modifiable header file containing the `D1Peer` structure for UDP client state management.
- `d2_lookup.c`: Implements the logic for handling directory service requests and tree data processing.
- `d2_lookup.h`: Header file with function prototypes and definitions for directory data handling.
- `d2_lookup_mod.h`: Modifiable header file containing the `D2Client` and `LocalTreeStore` structures.
- `Makefile`: For compiling the library and test clients.
- `README.txt`: This document.

Functional Description:
-----------------------
### Lower Layer - UDP Communication (`d1_udp.c`):
- **Creation and Deletion of Client**: Functions to initialize and tear down UDP client.
- **Data Transmission**: Sends data packets with headers and awaits acknowledgment.
- **Data Reception**: Receives data packets, checks for correctness, and handles ACKs.

### Upper Layer - Directory Service Handling (`d2_lookup.c`):
- **Client Creation and Deletion**: Manages connections to the directory service.
- **Request Sending**: Sends structured requests to retrieve directory data.
- **Response Handling**: Receives and processes the number of expected tree nodes and individual tree nodes.

Structure Details:
------------------
- `D1Peer` includes the socket descriptor, server address, sequence number control for packets, and potentially retry and timeout settings.
- `D2Client` links directly to `D1Peer` and facilitates higher-level operations specific to the directory service.
- `LocalTreeStore` manages the dynamically allocated tree nodes and provides functionality for adding nodes and printing the tree structure.

Implemented Features:
---------------------
- UDP client setup and teardown.
- Sending and receiving of UDP packets with basic protocol handling.
- Basic tree structure management for directory data.
- Sending directory requests and processing incoming tree data.

Not Implemented Features:
-------------------------
- Comprehensive error handling and retry mechanisms in UDP communication.
- Advanced memory management strategies in tree data storage.
- Detailed timeout and retry configurations in `D1Peer` for robust network communication.

Compilation and Running:
------------------------
Use the provided Makefile to compile the libraries and test clients:
  $ make

This builds `libhe.a` and the test clients (`d1_test_client` and `d2_test_client`). You can run these clients to interact with the configured test servers.

Usage:
------
To use the library in other applications, include `d1_udp.h` and `d2_lookup.h` in your source files and link against `libhe.a` during compilation.

End Note:
---------
This README covers the essential elements of the implementation. For any further details, refer to comments in the individual source files.
