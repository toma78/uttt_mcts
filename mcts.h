#ifndef MCTS_H
#define MCTS_H
#include "tree.h"

// expands MCTS by nnodes (rolls), returns <>0 if search is over
int expand_search(int nnodes);

// returns first / second nsims ratio
double confident(SearchTreeNode *node);

// advances MCTS to given position or restarts it with given position
int setup_search(Position *pos);

// prints main line
void print_pv();

#endif
