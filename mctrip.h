#ifndef MCTRIP_H
#define MCTRIP_H
#include "position.h"

//#define SERVER_DEBUG

typedef struct {
    /* external params */
    int tleft, tottime, timeinc;
	char instr[4096], field[1024], mbfield[1024];
	int turn, nmove; // nmove is always engine move - 1

    /* engine params */
    int talk;
	int risky;
	int tc_interval;	// number of rollouts between clock/input checks
	double uct_C;		// C parameter in UCT
	double exex_P;      // ExEx power parameter
	int rlimit;			// limit number of rolls for root node, 0 = no limit
	int expand_rolls;	// minimum rolls to expand new leaf

    /* running params */
    int start_time, stop_time, timeformove, spent_time;
	int exit_bot;		// exits main loop
	int roll_count;		// tracks number of rollouts during search
} TripParams;

extern TripParams *runp;

// start bot and MCST from initial position
void start_bot();

// resets MCST to initial position
void reset_bot();

// setups current position
void setup_position();

// check stdin for input
int has_input();

// expand tree until stdin input occures
void infinite_search();

// read and process line from stdin
void process_input();

#endif
