#include <stdlib.h>
#include <stdio.h>
#include "tree.h"
#include "mctrip.h"
#include "constants.h"

Position root_pos;
SearchTreeNode *root = NULL;

SearchTreeNode* tree_root() {
	return root;
}

Position* root_position() {
	return &root_pos;
}

void delete_subtree(SearchTreeNode *node) {
	for (int c = 0; c < node->nchild; c++) {
		if (node->children[c].child)
			delete_subtree(node->children[c].child);
	}
	if (node->children)
        free(node->children);
	free(node);
}

void init_tree(SearchTreeNode *node, Position *pos) {
	if (root != NULL)
		delete_subtree(root);
	root_pos = *pos;
	root = node;
}

SearchTreeNode* create_node(Position *pos) {
	SearchTreeNode* node = (SearchTreeNode*) malloc(sizeof(SearchTreeNode));
	if (node == NULL) {
		fprintf(stderr, "Malloc failed!");
		return NULL;
	}
	node->exex = 0;
	node->nsims = 0;
	node->turn = pos->turn;
	if (!pos->status) {
		move_type moves[81];
        node->nchild = legal_moves(pos, moves);
        node->children = (Child*) calloc(sizeof(Child), node->nchild);
        if (node->children == NULL) {
            fprintf(stderr, "Calloc failed!");
            return NULL;
        }
        for (int c = 0; c < node->nchild; c++) {
            node->children[c].move = moves[c];
        }        
    }
    else {
		node->nchild = 0;
        node->children = NULL;
    }
	return node;
}

int best_child(SearchTreeNode *node) {
	int ibest = 0;
	if (node->turn == 1) {
		for (int c = 0; c < node->nchild; c++) {
			ibest = (node->children[c].score > node->children[ibest].score) ? c : ibest;
		}
	}
	else {
		for (int c = 0; c < node->nchild; c++) {
			ibest = (node->children[c].score < node->children[ibest].score) ? c : ibest;
		}
	}
	return ibest;
}

void advance_root(int ichild) {
	SearchTreeNode *child = root->children[ichild].child;
	move_type move = root->children[ichild].move;
	// delete rest of the tree and root
	for (int c = 0; c < root->nchild; c++) {
		if (c != ichild && root->children[c].child)
			delete_subtree(root->children[c].child);
	}
    free(root->children);
	free(root);
	// set new root and position
	make_move(&root_pos, move);
	root = (child == NULL) ? create_node(&root_pos) : child;
}

int root_child(move_type omove) {
	for (int c = 0; c < root->nchild; c++) {
		if (root->children[c].move == omove)
			return c;
	}
	return -1;
}

int tree_size(SearchTreeNode *node) {
	int cnt = 1;
	for (int c = 0; c < node->nchild; c++) {
		if (node->children[c].child)
			cnt += tree_size(node->children[c].child);
	}
	return cnt;
}
