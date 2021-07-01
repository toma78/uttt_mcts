#include <stdio.h>
#include "mctrip.h"

// gcc -std=c99 -O2 -static ./src/*.c -o mcts.exe

int main() {
	// start bot and enter main loop
	start_bot();
	while (!runp->exit_bot) {
		if (!runp->rlimit)
			infinite_search();
		process_input();
	}
	return 0;
}
