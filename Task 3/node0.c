#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern struct rtpkt {
    int sourceid;
    int destid;
    int mincost[4];
};

extern int TRACE;
extern int YES;
extern int NO;
extern void tolayer2(struct rtpkt packet);


struct distance_table {
    int costs[4][4];
}dt0;

/* *************** DIAGNOSTIC OUTPUT *************** */
void printdt0(struct distance_table *dtptr) {
    printf("----------------\n");
    printf("Dest | Via0 | Via1 | Via2 | Via3\n");
    printf("-----|------|------|------|------\n");
    for(int dest=0; dest<4; dest++) {
        printf("%3d  |", dest);
        for(int via=0; via<4; via++) {
            printf(" %5d", dtptr->costs[dest][via]);
        }
        printf("\n");
    }
    printf("----------------\n");
}


// Helper prototypes
void send_to_neighbors0();
static int min_cost(int dest);

/* ***************** INITIALIZATION ROUTINE ******************* */
void rtinit0() {
    // Initialize direct costs: [0, 1, 3, 7]
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            dt0.costs[i][j] = 999;
        }
    }
    dt0.costs[0][0] = 0;
    // Set direct links
    dt0.costs[1][1] = 1;  // Node 0->1
    dt0.costs[2][2] = 3;  // Node 0->2
    dt0.costs[3][3] = 7;  // Node 0->3
    
    // Send initial distance vector to neighbors (1, 2, 3)
    send_to_neighbors0();
}

/* *************** UPDATE HANDLER **************** */
void rtupdate0(struct rtpkt *rcvdpkt) {
    int source = rcvdpkt->sourceid;
    int updated = 0;
    
    // Bellman-Ford algorithm implementation
    for(int dest = 0; dest < 4; dest++) {
        // Calculate new cost through this neighbor
        int new_cost = rcvdpkt->mincost[dest] + dt0.costs[source][source];
        
        // Update if new cost is better
        if(new_cost < dt0.costs[dest][source]) {
            dt0.costs[dest][source] = new_cost;
            updated = 1;
        }
    }
    
    // Propagate changes if any updates occurred
    if(updated) {
        send_to_neighbors0();
    }
    if (updated && TRACE > 0) { // Only print when changes occur
    printf("\nNode0 updated via Node%d:", source);
    }
    printdt0(&dt0);
}

/* *************** HELPER FUNCTIONS *************** */
void send_to_neighbors0() {
    // Compute minimum costs and best next hops for each destination
    int mincost[4];
    int next_hop[4];
    
    for(int dest = 0; dest < 4; dest++) {
        int min = 999;
        int via = -1;
        for(int v = 0; v < 4; v++) {
            if(dt0.costs[dest][v] < min) {
                min = dt0.costs[dest][v];
                via = v;
            }
        }
        mincost[dest] = min;
        next_hop[dest] = via;
    }
    
    // Send to each neighbor with poison reverse applied
    struct rtpkt pkt;
    pkt.sourceid = 0;
    
    // For each neighbor (1, 2, 3)
    for(int neighbor = 1; neighbor < 4; neighbor++) {
        // Only process direct neighbors
        if (dt0.costs[neighbor][neighbor] < 999) {
            // Create a copy of mincost for this neighbor
            int neighbor_mincost[4];
            memcpy(neighbor_mincost, mincost, 4*sizeof(int));
            
            // Apply poison reverse: If best path to dest is through this neighbor,
            // advertise it with infinite cost
            for(int dest = 0; dest < 4; dest++) {
                if (next_hop[dest] == neighbor && dest != neighbor) {
                    neighbor_mincost[dest] = 999; // Infinity
                }
            }
            
            // Send to this neighbor
            pkt.destid = neighbor;
            memcpy(pkt.mincost, neighbor_mincost, 4*sizeof(int));
            tolayer2(pkt);
        }
    }
}



/* *********** LINK CHANGE HANDLER (EXTRA CREDIT) *********** */
void linkhandler0(int linkid, int newcost) {
    // Update direct cost to neighbor
    dt0.costs[linkid][linkid] = newcost;

    // Recompute minimum costs and send updates
    send_to_neighbors0();

}



