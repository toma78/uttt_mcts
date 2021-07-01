#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mctrip.h"
#include "mcts.h"
#include "tree.h"
#include "evaluate.h"
#include "constants.h"

#define time_now() ((int)(clock()*1000/CLOCKS_PER_SEC))

// central parameters for entire bot
TripParams params = {
	.field = IBSTR, .mbfield = IMBSTR, .turn = ITURN, .nmove = 0,
	.tc_interval = 1000, .risky = 0, .talk = 0, .uct_C = 0.6, .exex_P = 5.0, .rlimit = 0,
	.tleft = 0, .tottime = 0, .timeinc = 500, .timeformove = 0,	.exit_bot = 0, .expand_rolls = 3
};
TripParams *runp = &params;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
#include <windows.h>
#include <wincon.h>
int has_input() {
	/*
    DWORD ne;
    HANDLE sin = GetStdHandle(STD_INPUT_HANDLE);
    GetNumberOfConsoleInputEvents(sin, &ne);
    return (ne > 0);*/
	HANDLE sin = GetStdHandle(STD_INPUT_HANDLE);
	return WaitForSingleObject(sin, 0) == WAIT_OBJECT_0;
}
#else
#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/time.h>

int has_input() {
    fd_set readfds;
    FD_ZERO(&readfds);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    FD_SET(STDIN_FILENO, &readfds);
    
    return select(1, &readfds, NULL, NULL, &timeout);
}
#endif

void start_bot() {
    setbuf(stdout, 0);
    srand((unsigned int)time(0));
	zseed = rand()*rand();
	wseed = rand()*rand();
	setup_position();
}

void reset_bot() {
	strcpy(runp->field, IBSTR);
	strcpy(runp->mbfield, IMBSTR);
	runp->turn = ITURN;
	runp->nmove = INMOVE;
	runp->roll_count = 0;
	setup_position();
}

void setup_position() {
	Position pos;
	init_pos(&pos, runp->field, runp->mbfield, runp->turn, runp->nmove);
	runp->roll_count = 0;
	int np = setup_search(&pos);
	if (runp->talk)
		printf("new position (tree replaced = %d)\n", np);
}

void allocate_time(int nmove) {
    runp->start_time = time_now();
    int alltime = 0;
    int delta = (runp->tleft - runp->timeinc) / 100;
    const int opening = 16, peak = 28;
    if (nmove < opening)    
        alltime = runp->timeinc + delta;
    else if (nmove < peak)
        alltime = runp->timeinc + 2 * (1+nmove-opening) * delta;
    else
        alltime = runp->timeinc + 25 * delta;
    runp->stop_time = runp->start_time + alltime;
    runp->timeformove = runp->stop_time - runp->start_time; 
	if (runp->talk)
		printf("Move %d, tinc %d, tleft %d, time for move %d\n", nmove, runp->timeinc, runp->tleft, runp->timeformove);
}

// spit move before advancing tree
void print_and_advance() {
	int ibest = best_child(tree_root());
	move_type bestm = tree_root()->children[ibest].move;
	int ind = mcode2imove(bestm);
	printf("place_move %d %d\n", ind % 9, ind / 9);
	advance_root(ibest);
}

void infinite_search() {
	while (!has_input()) {
		expand_search(runp->tc_interval);
	}
}

void limited_search() {
	runp->roll_count = 0;
	expand_search(runp->rlimit);
	if (runp->talk) {
		printf("Rolls %d / %d\n", runp->roll_count, runp->rlimit);
		print_pv();
	}
	print_and_advance();
}

void timed_search() {
	runp->roll_count = 0;
	allocate_time(root_position()->nmove);
	// expand tree
	while (1) {
        if(expand_search(runp->tc_interval))
            break;
        if (root_position()->nmove < 2 && runp->roll_count > 30000) // early stop for first move with any side
            break;
		int tnow = time_now();
		if (tnow >= runp->stop_time)
			break;
		if (tnow + runp->timeformove / 2 > runp->stop_time && confident(tree_root()) > 3.0) { // early stop for confident move
			break;
		}
	}
	// print move
	runp->spent_time = (time_now() - runp->start_time) + 1; // avoid div by 0
	if (runp->talk) {
		printf("Time spent %d, conf %lf, rolls %d, RPS %dK\n", runp->spent_time, confident(tree_root()), runp->roll_count, (int)(runp->roll_count / runp->spent_time));
		print_pv();
	}
	print_and_advance();
}

void process_input() {
	char *ptr, instr[4096];
    do {
		ptr = fgets(instr, 4095, stdin);
        if (!strncmp("action move", instr, 11)) {
            ptr = 0;
            runp->tleft = strtol(instr + 12, &ptr, 10);
            setup_position();
			if (!runp->rlimit)
				timed_search();
			else
				limited_search();
        }
        else if (!strncmp("update game field", instr, 17)) {
            strcpy(runp->field, instr + 18);
        }
        else if (!strncmp("update game macroboard", instr, 22)) {
            strcpy(runp->mbfield, instr + 23);
        }
        else if (!strncmp("update game move", instr, 16)) {
            ptr = 0;
            runp->nmove = strtol(instr + 17, &ptr, 10) - 1;
        }
        else if (!strncmp("settings timebank", instr, 17)) {
            ptr = 0;
            runp->tottime = strtol(instr + 18, &ptr, 10);
        }
        else if (!strncmp("settings time_per_move", instr, 22)) {
            ptr = 0;
            runp->timeinc = strtol(instr + 23, &ptr, 10);
        }
        else if (!strncmp("settings your_botid", instr, 19)) {
            ptr = 0;
            runp->turn = strtol(instr + 20, &ptr, 10);
        }
        else if (!strncmp("talk", instr, 4)) {
            ptr = 0;
            runp->talk = strtol(instr + 5, &ptr, 10);
        }
        else if (!strncmp("reset", instr, 5)) {
            reset_bot();
        }
        else if (!strncmp("exit", instr, 4)) {
            runp->exit_bot = 1;
			return;
        }
        else if (!strncmp("risky", instr, 5)) {
            ptr = 0;
            runp->risky = strtol(instr + 6, &ptr, 10);
        }
        else if (!strncmp("uctc", instr, 4)) {
            ptr = 0;
            runp->uct_C = strtod(instr + 5, &ptr);
        }
		else if (!strncmp("expr", instr, 4)) {
			ptr = 0;
			runp->expand_rolls = strtol(instr + 5, &ptr, 10);
		}
		else if (!strncmp("exex", instr, 4)) {
            ptr = 0;
            runp->exex_P = strtod(instr + 5, &ptr);
        }
		else if (!strncmp("rlimit", instr, 6)) {
			ptr = 0;
			runp->rlimit = strtol(instr + 7, &ptr, 10);
			reset_bot();
		}
	} while (has_input());
}