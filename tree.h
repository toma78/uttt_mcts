#ifndef TREE_H
#define TREE_H
#include "position.h"
#include <inttypes.h>

struct SearchTreeNode;

typedef struct Child {
	move_type move;					// move from parent to this child
	double score;					// MC estimate Q(s,a)
	int nsims;						// MC count
	struct SearchTreeNode *child;	// pointer to child node or NULL
} Child;

typedef struct SearchTreeNode {
	int exex;						// node's count of succ/fail x explore/exploit table
	int turn;						// turn to move
	int nchild;						// number of children
	int nsims;						// number of simulations trough this node
	struct Child *children;			// vector of children
} SearchTreeNode;

// gets tree root
SearchTreeNode* tree_root();

// gets tree root position
Position* root_position();

// initializes root with node
void init_tree(SearchTreeNode *node, Position *pos);

// creates new node from position
SearchTreeNode* create_node(Position *pos);

// best child index according to MC score
int best_child(SearchTreeNode *node);

// promotes child to root deleting all alternatives
void advance_root(int child);

// returns root's child index equal to move
int root_child(move_type omove);

// count all nodes in tree
int tree_size(SearchTreeNode *node);

#endif
