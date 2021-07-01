#ifndef EVALUATE_H
#define EVALUATE_H
#include "position.h"

// fast random generator from http://www.cse.yorku.ca/~oz/marsaglia-rng.html
#define znew (zseed=36969*(zseed&65535)+(zseed>>16))
#define wnew (wseed=18000*(wseed&65535)+(wseed>>16))
#define MWCRAND ((znew<<16)+wnew)
static unsigned int zseed = 362436069, wseed = 521288629;

// returns game result
int rollout(Position *pos);

#endif

