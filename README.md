# CN_Assignment_3

## Overview

This repository contains the code and supporting files for Assignment 3 of the Computer Networks course (CS 331) at IIT Gandhinagar.

**Submitted by:**
*   Group 1:
    *   Mitansh Patel (24120033)
    *   Chinteshwar Dhakate (24120024)

**Course Instructor:** Prof. Sameer Kulkarni

**Assignment Focus:**
This assignment covers three core networking concepts:
1.  **Network Loops (Task 1):** Analyzing broadcast storm issues in a looped Layer 2 topology and implementing a solution using Spanning Tree Protocol (STP).
2.  **Host-based NAT Configuration (Task 2):** Setting up Network Address Translation (NAT) on a host to enable communication between a private internal network and an external network.
3.  **Network Routing (Task 3):** Implementing a distributed asynchronous distance vector routing algorithm based on the Bellman-Ford equation, including handling link cost changes and preventing routing loops using poison reverse.

**Environment Note:**
*   Tasks 1 & 2 were implemented and tested using Python scripts within a Mininet environment on Linux (Fedora 41).
*   Task 3 was implemented in C and tested on Linux (Fedora 41).

## Repository Structure
```
├── Task1/                                 # Looped Topology, STP
│   └── topology_t1.py                     # Python script for topology implementation
├── Task2/                                 # NAT Implementation
│   └── nat_t2.py                          # Python script for nat implementation
├── Task3/                                 # Distance Vector Routing
│   ├── distance_vector.c                  # C file for distance vector routing algorithm
│   ├── node0.c                            # C file for Node 0 Implementation
│   ├── node1.c                            # C file for Node 1 Implementation
│   ├── node2.c                            # C file for Node 2 Implementation
│   ├── node3.c                            # C file for Node 3 Implementation
│   └── run_p                              # Runnable/Executable program
└── README.md
```
## Setup and Usage

### Prerequisites

*   **Tasks 1 & 2:** A Linux environment with Mininet and Open vSwitch (OVS) installed. Tools like `brctl`, `sysctl`, and `iptables` are also required.
*   **Task 3:** A C compiler (like `gcc`) on a Linux environment.

### Task 1 & 2: Mininet Tasks (Network Loops & NAT)

1.  **Setup:** Use the Python scripts (as described in the PDF report) to build the respective network topologies within Mininet.
2.  **Task 1 (Loops) Fix:**
    *   Enable STP on all switches using OVS commands within Mininet:
        ```
        mininet> sh ovs-vsctl set Bridge <switch_name> stp_enable=true
        # Repeat for s1, s2, s3, s4
        ```
    *   Verify STP status:
        ```
        mininet> sh ovs-vsctl get Bridge <switch_name> stp_enable
        ```
3.  **Task 2 (NAT) Configuration:**
    *   Execute the necessary commands on the designated NAT host (h9) and other relevant hosts within Mininet, as detailed in the PDF report. Key commands involve:
        *   `brctl` (for bridge setup)
        *   `sysctl` (for IP forwarding and proxy ARP)
        *   `iptables` (for NAT MASQUERADE and FORWARD rules)
        *   `ip route` (for static routing)
4.  **Testing (Tasks 1 & 2):**
    *   Check connectivity:
        ```
        mininet> pingall
        ```
    *   Perform specific pings (e.g., `h3 ping h1`)
    *   Run bandwidth tests using `iperf3`:
        ```
        # Example: Server on h1, Client on h6
        mininet> h1 iperf3 -s -p 1111 &
        mininet> h6 iperf3 -c h1 -p 1111 -t 120
        ```

### Task 3: Distance Vector Routing

1.  **Clone the Repository:**
    ```
    git clone https://github.com/Mitansh-Patel-24120033/CN_Assignment_3.git
    cd CN_Assignment_3
    ```
2.  **Compile the Code:**
    ```
    gcc distance_vector.c node0.c node1.c node2.c node3.c -o run_p
    ```
3.  **Run the Simulation:**
    ```
    ./run_p
    ```
    *(You can modify the `TRACE` variable in `distance_vector.c` to control the level of output detail.)*

## Task Details Summary

### Task 1: Network Loops

*   **Objective:** Analyze and fix connectivity issues caused by broadcast storms in a topology with redundant switch loops.
*   **Problem:** Standard switches flood broadcast packets, leading to infinite loops and network saturation, causing ping failures (100% packet loss).
*   **Solution:** Enable Spanning Tree Protocol (STP) on all OVS switches (`s1`, `s2`, `s3`, `s4`). STP logically blocks redundant paths to create a loop-free topology.
*   **Result:** Successful pings between all hosts (0% packet loss) with measurable RTT delays (e.g., ~34ms, ~63ms depending on the path).

### Task 2: Host-based NAT Configuration

*   **Objective:** Configure host `h9` to act as a NAT gateway, allowing private hosts (`h1`, `h2` with 10.1.1.x IPs) to communicate with external hosts (`h3`-`h8` with 172.16.10.x IPs) and vice-versa.
*   **Solution:**
    *   Create a network bridge (`br-private`) for the internal network.
    *   Enable IP forwarding and proxy ARP on `h9`.
    *   Configure `iptables` rules on `h9`:
        *   `POSTROUTING` chain with `MASQUERADE` target for outbound traffic.
        *   `FORWARD` chain rules to allow traffic between interfaces.
        *   `PREROUTING` rules (optional, for specific inbound access if needed, e.g., DNAT for pings in the report).
    *   Add static routes on external hosts pointing the internal network (`10.1.1.0/24`) via `h9`'s external IP (`172.16.10.10`).
    *   Add default routes on internal hosts (`h1`, `h2`) via `h9`'s internal IP (`10.1.1.1`).
    *   Ensure STP is enabled on switches to prevent loops from the modified topology.
*   **Result:** Successful communication (ping, iperf) between internal and external hosts in both directions, with expected NAT translation observed implicitly through connectivity. Measured RTTs (e.g., ~44ms to ~73ms) and iperf bandwidths (e.g., ~1.1 Gbps to ~1.6 Gbps).

### Task 3: Network Routing (Distance Vector)

*   **Objective:** Implement the distributed asynchronous distance vector routing algorithm for a 4-node network.
*   **Implementation:**
    *   Defined `struct rtpkt` for exchanging distance vectors between nodes.
    *   Each node maintains a `struct distance_table` (`dtX`).
    *   `rtinitX()`: Initializes the distance table with direct link costs and sends initial distance vectors to neighbors.
    *   `rtupdateX()`: Processes received distance vectors (`rtpkt`), updates the local distance table using the Bellman-Ford logic (`new_cost = neighbor_cost + reported_cost`), and sends updates to neighbors if its own minimum costs change.
    *   Implemented **Poison Reverse** to mitigate the count-to-infinity problem by advertising an infinite cost (999) to a neighbor for destinations routed *through* that same neighbor.
*   **Simulation:** The `distance_vector.c` framework simulates packet delivery and timing.
*   **Result:** The algorithm successfully converges to stable shortest path routes (e.g., Node0 converges to `[0, 1, 2, 4]`). The implementation correctly handles simulated link cost changes by updating routes and propagating changes. Poison reverse effectively prevents routing loops during convergence and after link changes. Convergence time observed to be around 15 simulation time units in the initial phase.

## Conclusion

This assignment provided practical experience with:
*   Diagnosing and resolving Layer 2 loops using STP.
*   Configuring host-based NAT for network segmentation and connectivity.
*   Implementing and observing the behavior of the distance vector routing algorithm, including convergence, dynamic updates, and loop prevention mechanisms like poison reverse.
