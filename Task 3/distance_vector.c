#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINKCHANGES 1
#define FROM_LAYER2 0
/* **************************************************************
Programming assignment 3: distance vector routing emulator
*************************************************************** */

/* Packet structure */
struct rtpkt {
    int sourceid;     /* Sending router ID */
    int destid;       /* Destination router ID */
    int mincost[4];   /* Minimum costs to nodes 0-3 */
};

/* Global configuration */
int TRACE = 1;        /* Debugging trace level */
int YES = 1;
int NO = 0;

/* Event structure */
struct event {
    float evtime;         /* Event time */
    int evtype;           /* Event type code */
    int eventity;         /* Entity where event occurs */
    struct rtpkt *rtpktptr;  /* Associated packet */
    struct event *prev;
    struct event *next;
};

struct event *evlist = NULL;  /* Event list */
float clocktime = 0.000;      /* Global simulation time */

/* Function prototypes */
void creatertpkt(struct rtpkt *initrtpkt, int srcid, int destid, int mincosts[]);
void rtinit0(), rtinit1(), rtinit2(), rtinit3();
void rtupdate0(struct rtpkt *rcvdpkt);
void rtupdate1(struct rtpkt *rcvdpkt);
void rtupdate2(struct rtpkt *rcvdpkt);
void rtupdate3(struct rtpkt *rcvdpkt);
void linkhandler0(int linkid, int newcost);
void linkhandler1(int linkid, int newcost);
float jimsrand(void);
void insertevent(struct event *p);
void tolayer2(struct rtpkt packet);
void printevlist(void);
void init(void);

/* ********************** CORE FUNCTIONS *********************** */

void creatertpkt(struct rtpkt *initrtpkt, int srcid, int destid, int mincosts[]) {
    initrtpkt->sourceid = srcid;
    initrtpkt->destid = destid;
    memcpy(initrtpkt->mincost, mincosts, 4*sizeof(int));
}

int main(void) {
    struct event *eventptr;
    
    init();
    while (1) {
        eventptr = evlist;
        if (!eventptr) break;
        
        evlist = evlist->next;
        if (evlist) evlist->prev = NULL;
        
        clocktime = eventptr->evtime;
        
        if (eventptr->evtype == FROM_LAYER2) {
            /* Packet processing */
            if (TRACE > 1) {
		printf("\nMAIN: rcv event t=%.3f src:%d → dest:%d costs: [%3d, %3d, %3d, %3d]\n", 
		clocktime, 
		eventptr->rtpktptr->sourceid,
		eventptr->rtpktptr->destid,
		eventptr->rtpktptr->mincost[0],
		eventptr->rtpktptr->mincost[1],
		eventptr->rtpktptr->mincost[2],
		eventptr->rtpktptr->mincost[3]);

            }
            
            switch(eventptr->eventity) {
                case 0: rtupdate0(eventptr->rtpktptr); break;
                case 1: rtupdate1(eventptr->rtpktptr); break;
                case 2: rtupdate2(eventptr->rtpktptr); break;
                case 3: rtupdate3(eventptr->rtpktptr); break;
                default: 
                    printf("Invalid event entity!\n");
                    exit(1);
            }
        } 
        else if (eventptr->evtype == LINKCHANGES) {
            /* Dynamic link cost changes (for extra credit) */
            if (clocktime < 10001.0) {
                linkhandler0(1, 20);  /* Node 0->1 cost becomes 20 */
                linkhandler1(0, 20);  /* Node 1->0 cost becomes 20 */
            } else {
                linkhandler0(1, 1);   /* Restore original costs */
                linkhandler1(0, 1);
            }
        }
        
        if (eventptr->rtpktptr) free(eventptr->rtpktptr);
        free(eventptr);
    }
    
    printf("\nSimulation terminated at t=%.3f\n", clocktime);
    return 0;
}

/* ********************** HELPER FUNCTIONS ********************* */
void init(void) {
    printf("Enter TRACE level (0-3): ");
    scanf("%d", &TRACE);
    
    srand(9999);  /* Seed random number generator */
    
    /* Initialize routing tables */
    rtinit0();
    rtinit1();
    rtinit2();
    rtinit3();
    
    /* Schedule future link changes */
    if (LINKCHANGES) {
        struct event *ev = malloc(sizeof(struct event));
        ev->evtime = 10000.0;
        ev->evtype = LINKCHANGES;
        ev->eventity = -1;
        ev->rtpktptr = NULL;
        insertevent(ev);
        
        ev = malloc(sizeof(struct event));
        ev->evtime = 20000.0;
        ev->evtype = LINKCHANGES;
        ev->eventity = -1;
        ev->rtpktptr = NULL;
        insertevent(ev);
    }
}

float jimsrand(void) {
    return (float)rand() / RAND_MAX;
}

void insertevent(struct event *p) {
    struct event *q, *qold;
    
    if (TRACE > 3) {
        printf("INSERT EVENT: t=%.3f type=%d\n", p->evtime, p->evtype);
    }
    
    if (!evlist) {  /* Empty list */
        evlist = p;
        p->next = p->prev = NULL;
        return;
    }
    
    q = evlist;
    while (q && q->evtime < p->evtime) {
        qold = q;
        q = q->next;
    }
    
    if (!q) {  /* End of list */
        qold->next = p;
        p->prev = qold;
        p->next = NULL;
    } 
    else if (q == evlist) {  /* Front of list */
        p->next = evlist;
        p->prev = NULL;
        evlist->prev = p;
        evlist = p;
    } 
    else {  /* Middle of list */
        p->next = q;
        p->prev = q->prev;
        q->prev->next = p;
        q->prev = p;
    }
}

void tolayer2(struct rtpkt packet) {
    static int connectcosts[4][4] = {
        {0, 1, 3, 7},
        {1, 0, 1, 999},
        {3, 1, 0, 2},
        {7, 999, 2, 0}
    };
    
    /* Validate packet */
    if (packet.sourceid < 0 || packet.sourceid > 3 || 
        packet.destid < 0 || packet.destid > 3 ||
        packet.sourceid == packet.destid) {
        printf("Invalid packet! src:%d dest:%d\n", 
              packet.sourceid, packet.destid);
        return;
    }
    
    /* Check physical connection */
    if (connectcosts[packet.sourceid][packet.destid] == 999) {
        if (TRACE) {
            printf("WARNING: No connection %d->%d\n",
                  packet.sourceid, packet.destid);
        }
        return;
    }
    
    /* Create packet copy */
    struct rtpkt *mypkt = malloc(sizeof(struct rtpkt));
    memcpy(mypkt, &packet, sizeof(struct rtpkt));
    
    /* Schedule arrival event */
    struct event *ev = malloc(sizeof(struct event));
    ev->evtype = FROM_LAYER2;
    ev->eventity = packet.destid;
    ev->rtpktptr = mypkt;
    ev->evtime = clocktime + 1 + 9*jimsrand();  /* 1-10 time units */
    
    insertevent(ev);
    
    if (TRACE > 2) {
	printf("QUEUED: t=%.3f src:%d → dest:%d costs: [%3d, %3d, %3d, %3d]\n", 
	ev->evtime, 
	packet.sourceid, 
	packet.destid,
	packet.mincost[0],
	packet.mincost[1],
	packet.mincost[2],
	packet.mincost[3]);

    }
}

