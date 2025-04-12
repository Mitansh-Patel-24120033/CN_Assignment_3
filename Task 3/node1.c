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
}dt1;

// Helper prototypes
void send_to_neighbors1();
static int min_cost(int dest);

/* *************** DIAGNOSTIC OUTPUT *************** */
void printdt1(struct distance_table *dtptr) {
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


/* ***************** INITIALIZATION ROUTINE ******************* */
void rtinit1() {
    // Initialize direct costs: [1, 0, 1, 999]
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            dt1.costs[i][j] = 999;
        }
    }
    dt1.costs[1][1] = 0;
    // Set direct links
    dt1.costs[0][0] = 1;  // Node 1->0
    dt1.costs[2][2] = 1;  // Node 1->2
    
    // Send initial distance vector to neighbors (0 & 2)
    send_to_neighbors1();
}

/* *************** UPDATE HANDLER **************** */
void rtupdate1(struct rtpkt *rcvdpkt) {
    int source = rcvdpkt->sourceid;
    int updated = 0;
    
    // Bellman-Ford algorithm implementation
    for(int dest = 0; dest < 4; dest++) {
        // Calculate new cost through this neighbor
        int new_cost = rcvdpkt->mincost[dest] + dt1.costs[source][source];
        
        // Update if new cost is better
        if(new_cost < dt1.costs[dest][source]) {
            dt1.costs[dest][source] = new_cost;
            updated = 1;
        }
    }
    
    // Propagate changes if any updates occurred
    if(updated) {
        send_to_neighbors1();
    }
    if (updated && TRACE > 0) { // Only print when changes occur
    printf("\nNode1 updated via Node%d:", source);
    }
    printdt1(&dt1);
}

/* *************** HELPER FUNCTIONS *************** */
void send_to_neighbors1() {
    // Compute minimum costs and best next hops for each destination
    int mincost[4];
    int next_hop[4];
    
    for(int dest = 0; dest < 4; dest++) {
        int min = 999;
        int via = -1;
        for(int v = 0; v < 4; v++) {
            if(dt1.costs[dest][v] < min) {
                min = dt1.costs[dest][v];
                via = v;
            }
        }
        mincost[dest] = min;
        next_hop[dest] = via;
    }
    
    // Send to each neighbor with poison reverse applied
    struct rtpkt pkt;
    pkt.sourceid = 1;
    
    // To each direct neighbor (0 & 2)
    int neighbors[] = {0, 2};
    for(int i = 0; i < 2; i++) {
        int neighbor = neighbors[i];
        // Create a copy of mincost for this neighbor
        int neighbor_mincost[4];
        memcpy(neighbor_mincost, mincost, 4*sizeof(int));
        
        // Apply poison reverse
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



/* *********** LINK CHANGE HANDLER (EXTRA CREDIT) *********** */
void linkhandler1(int linkid, int newcost) {
    // Update direct cost to neighbor
    dt1.costs[linkid][linkid] = newcost;

    // Recompute minimum costs and send updates
    send_to_neighbors1();
}



