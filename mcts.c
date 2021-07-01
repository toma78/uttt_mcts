#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mctrip.h"
#include "mcts.h"
#include "evaluate.h"
#include "constants.h"

double uct_score(double logt, double uctc, Child *c) {
    return c->score + uctc * sqrt(logt / c->nsims);
}

int uct_child(SearchTreeNode *node) {
    if (node->nchild == 1 || node->nsims == 0)
        return 0;        

	int ibest = 0;
	double logt = log(node->nsims);
	if (node->turn == 1) {
		double uctc = runp->uct_C;
        double best = uct_score(logt, uctc, node->children);
		for (int c = 1; c < node->nchild; c++) {
            if (node->children[c].nsims == 0)
                return c;
			double ucts = uct_score(logt, uctc, node->children + c);
			if (ucts > best) {
				ibest = c;
				best = ucts;
			}
		}
	}
	else {
		double uctc = -runp->uct_C;
		double best = uct_score(logt, uctc, node->children);
        for (int c = 1; c < node->nchild; c++) {
            if (node->children[c].nsims == 0)
                return c;
			double ucts = uct_score(logt, uctc, node->children + c);
            if (ucts < best) {
                ibest = c;
                best = ucts;
            }
        }
	}
	return ibest;
}

#define EXEX_MINSIMS 10000
// 10000, d^5,  vs TRIP short, tot [832, 222, 302, 308] -9.6%
// 10000, d^4,  tot [324, 110, 57, 157] 16%
// 10000, d^3,  tot [950, 295, 144, 511] 16%
// 1000, d^3, tot [208, 56, 37, 115] 9%

double exex_uctc(SearchTreeNode *node) {
	//double uctc = runp->uct_C  + (EXEX_LAMBDA * node->exex) / node->nsims;
	double delta = ((double)(node->nsims + node->exex)) / node->nsims;
    //return delta < 1.0 ? 0.0 : runp->uct_C;
	return runp->uct_C * pow(delta, runp->exex_P);
}

int exex_child(SearchTreeNode *node, int *explore) {
    *explore = 1;
	if (node->nchild == 1 || node->nsims == 0)
        return 0;

	int ibest = 0;
	double logt = log(node->nsims);
	if (node->turn == 1) {
        double maxscore = node->children[0].score;
		double uctc = exex_uctc(node);
		double best = uct_score(logt, uctc, node->children);
		for (int c = 1; c < node->nchild; c++) {
			if (node->children[c].nsims == 0)
                return c;
			double ucts = uct_score(logt, uctc, node->children + c);
			if (ucts > best) {
				ibest = c;
				best = ucts;
			}
			if (maxscore < node->children[c].score)
                maxscore = node->children[c].score;
		}
		*explore = maxscore > node->children[ibest].score;
	}
	else {
        double minscore = node->children[0].score;
		double uctc = -exex_uctc(node);
		double best = uct_score(logt, uctc, node->children);
		for (int c = 1; c < node->nchild; c++) {
			if (node->children[c].nsims == 0)
				return c;
			double ucts = uct_score(logt, uctc, node->children + c);
			if (ucts < best) {
				ibest = c;
				best = ucts;
			}
            if (minscore > node->children[c].score)
                minscore = node->children[c].score;
		}
        *explore = minscore < node->children[ibest].score;
	}
	return ibest;
}

void select_and_expand(SearchTreeNode *node, Position *pos) {
	int lcnt = 0;
	SearchTreeNode *line_state[81];
    Child *line_child[81];
    int line_explore[81];
	// select node
	while (node) {
		int nchild = uct_child(node); //  runp->risky ? exex_child(node, line_explore + lcnt) : 
        line_state[lcnt] = node;
        line_child[lcnt] = &(node->children[nchild]);
		make_move(pos, line_child[lcnt]->move);
		lcnt++;
		if (pos->status)        
			break;
		node = line_child[lcnt - 1]->child;
	}
	// expand and rollout
    if (!pos->status) {
        // expand child
        if (line_child[lcnt - 1]->child == NULL && line_child[lcnt - 1]->nsims >= runp->expand_rolls) {
            line_child[lcnt - 1]->child = create_node(pos);
        }
        rollout(pos);
    }
	double reward = result2value[pos->status];
	
	// backpropagate reward
	for (int t = lcnt - 1; t >= 0; t--) {
        /*
		if (runp->risky && line_state[t]->nsims > EXEX_MINSIMS) {
			if (line_state[t]->turn == 1) {
				// max
                if (line_explore[t]) {
					// explore 
                    if(reward > line_child[t]->score)
                        line_state[t]->exex++; // success => increase C
                    else if (reward < line_child[t]->score)
                        line_state[t]->exex--;  // fail => decrease C
                } else {
					// exploit
					if(reward > line_child[t]->score)
						line_state[t]->exex--; // success => decrease C
                    else if(reward < line_child[t]->score)
						line_state[t]->exex++;  // fail => increase C                    
                }
            } else {
				// min
                if (line_explore[t]) {
					// explore 
					if (reward < line_child[t]->score)
						line_state[t]->exex++; // success => increase C
					else if (reward > line_child[t]->score)
						line_state[t]->exex--;  // fail => decrease C
				}
				else {
					// exploit
					if (reward < line_child[t]->score)
						line_state[t]->exex--; // success => decrease C
					else if (reward > line_child[t]->score)
						line_state[t]->exex++;  // fail => increase C                    
				}
			}
		}*/
		// TODO: backup with (explored + best) / 2 reward?
        line_state[t]->nsims += 1;
        line_child[t]->nsims += 1;
        line_child[t]->score += (reward - line_child[t]->score) / line_child[t]->nsims;
    }
}

int expand_search(int nrolls) {
	SearchTreeNode *root = tree_root();
	Position rpos = *(root_position());
    if (!root->children) // terminal position
        return 1;
	for (int r = 0; r < nrolls; r++) {
		Position pos = rpos;
		select_and_expand(root, &pos);
	}
	return 0;
}

double confident(SearchTreeNode *node) {
	if (!node->children || node->nchild < 2)
		return 1000.0;
	int first = node->children[0].nsims > node->children[1].nsims ? node->children[0].nsims : node->children[1].nsims;
	int second = node->children[0].nsims < node->children[1].nsims ? node->children[0].nsims : node->children[1].nsims;
	for (int c = 2; c < node->nchild; c++) {
		if (node->children[c].nsims > first) {
			second = first;
			first = node->children[c].nsims;
		}
		else if (node->children[c].nsims > second) {
			second = node->children[c].nsims;
		}
	}
	return ((double)first) / second;
}

int setup_search(Position *pos) {
	SearchTreeNode *root = tree_root();
	Position *rpos = root_position();
	if (root != NULL && pos->nmove == rpos->nmove + 1) {
		// same initial position
		if (pos->nmove == 0 && rpos->nmove == 0)
			return 0;
		// try to guess opponent move and descend to child
		move_type omove = guess_move(rpos, pos);
		if (omove != NOMOVE) {
			int ichild = root_child(omove);
			if (ichild >= 0) {
				advance_root(ichild);
				return 1;
			}
		}
	}
	// replace tree	
	SearchTreeNode *newroot = create_node(pos);
	init_tree(newroot, pos);
	return 2;
}

void print_pv() {
	SearchTreeNode *node = tree_root();
    if (!node || !node->children) {
        printf("Game over!\n");
        return;
    }
    int ichild = best_child(node);
    int m = mcode2imove(node->children[ichild].move);
    printf("PV=(%d,%d), v=%lf, rsims %d, ts=%d, uctc=%lf\n", m%9, m/9, node->children[ichild].score, node->nsims, tree_size(node), exex_uctc(node));

    for(int c = 0; c < node->nchild; c++) {
        m = mcode2imove(node->children[c].move);
        printf("(%d,%d), v=%lf, csims=%d\n", m%9, m/9, node->children[c].score, node->children[c].nsims);
    }
}
