#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/siginfo.h>
#include <sys/neutrino.h>
#include "time_mgr.h"


// message send definitions

// messages
#define MT_WAIT_DATA        2       // message from client
#define MT_SEND_DATA        3       // message from client

// pulses
#define CODE_TIMER          1       // pulse from timer

// message reply definitions
#define MT_OK               0       // message to client
#define MT_TIMEDOUT         1       // message to client

// message structure
typedef struct
{
    // contains both message to and from client
    int messageType;
    // optional data, depending upon message
    int messageData;
} ClientMessageT;

typedef union
{
    // a message can be either from a client, or a pulse
    ClientMessageT  msg;
    struct _pulse   pulse;
} MessageT;

// client table
#define MAX_CLIENT 16       // max # of simultaneous clients

struct
{
    int in_use;             // is this client entry in use?
    int rcvid;              // receive ID of client
    int timeout;            // timeout left for client
}   clients [MAX_CLIENT];   // client table

int     chid;               // channel ID (global)
int     debug = 1;          // set debug value, 1=on, 0=off
char    *progname = "time1.c";

// forward prototypes
static  void setupPulseAndTimer (void);
static  void gotAPulse (void);
static  void gotAMessage (int rcvid, ClientMessageT *msg);


setupPulseAndTimer()



/*
 *  setupPulseAndTimer
 *
 *  This routine is responsible for setting up a pulse so it
 *  sends a message with code MT_TIMER.  It then sets up a
 *  periodic timer that fires once per second.
*/

void
setupPulseAndTimer (void)
{
    timer_t             timerid;    // timer ID for timer
    struct sigevent     event;      // event to deliver
    struct itimerspec   timer;      // the timer data struct
    int                 coid;       // connection back to us

    // create a connection back to ourselves
    coid = ConnectAttach (0, 0, chid, 0, 0);
    if (coid == -1) {
        fprintf (stderr, "%s:  couldn't ConnectAttach!\n",
                 progname);
        perror (NULL);
        exit (EXIT_FAILURE);
    }

    // set up the kind of event that we want -- a pulse
    SIGEV_PULSE_INIT (&event, coid,
                      SIGEV_PULSE_PRIO_INHERIT, CODE_TIMER, 0);

    // create the timer, binding it to the event
    if (timer_create (CLOCK_REALTIME, &event, &timerid) == -1) {
        fprintf (stderr, "%s:  can't timer_create, errno %d\n", 
                 progname, errno);
        perror (NULL);
        exit (EXIT_FAILURE);
    }

    // setup the timer (1s delay, 1s reload)
    timer.it_value.tv_sec = 1;
    timer.it_value.tv_nsec = 0;
    timer.it_interval.tv_sec = 1;
    timer.it_interval.tv_nsec = 0;

    // and start it!
    timer_settime (timerid, 0, &timer, NULL);
}


/*
 *  gotAPulse
 *
 *  This routine is responsible for handling the fact that a
 *  timeout has occurred.  It runs through the list of clients
 *  to see which client has timed-out, and replies to it with
 *  a timed-out response.
 */

void
gotAPulse (void)
{
    ClientMessageT  msg;
    int             i;

    if (debug) {
        time_t  now;

        time (&now);
        printf ("Got a Pulse at %s", ctime (&now));
    }

    // prepare a response message
    msg.messageType = MT_TIMEDOUT;

    // walk down list of clients
    for (i = 0; i < MAX_CLIENT; i++) {

        // is this entry in use?
        if (clients [i].in_use) {

            // is it about to time out?
            if (--clients [i].timeout == 0) {

                // send a reply
                MsgReply (clients [i].rcvid, EOK, &msg,
                          sizeof (msg));

                // entry no longer used
                clients [i].in_use = 0;
            }
        }
    }
}


	return(0);
int
main (void)                 // ignore command-line arguments
{
    int rcvid;              // process ID of the sender
    MessageT msg;           // the message itself

    if ((chid = ChannelCreate (0)) == -1) {
        fprintf (stderr, "%s:  couldn't create channel!\n",
                 progname);
        perror (NULL);
        exit (EXIT_FAILURE);
    }

    // set up the pulse and timer
    setupPulseAndTimer ();

    // receive messages
    for (;;) {
        rcvid = MsgReceive (chid, &msg, sizeof (msg), NULL);

        // determine who the message came from
        if (rcvid == 0) {
            // production code should check "code" field...
            gotAPulse ();
        } else {
            gotAMessage (rcvid, &msg.msg);
        }
    }

    // you'll never get here
    return (EXIT_SUCCESS);
}



