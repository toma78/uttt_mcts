#ifndef POSITION_H
#define POSITION_H
#include <inttypes.h>
#include "constants.h"

#define IBSTR "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0"
#define IMBSTR "-1,-1,-1,-1,-1,-1,-1,-1,-1"
#define ITURN 1
#define INMOVE 0

typedef struct {
	unsigned int nmove, status;
	unsigned int turn, validmb;
	board_type overs[3]; // draws, p1vics, p2vics
	board_type boards[3][9]; // p1|p2, p1, p2 
} Position;

#define BFULL 511

#define is_full(num) ((num)==BFULL)
#define isset(board, i) ((board)&(1<<(i)))

#define NOMOVE 255
#define code_move(mbi, bi) ((bi)|((mbi)<<4))
#define decode_move(mbi, bi, m) move_type bi=m&15; move_type mbi=(m>>4)&15;

#define P1VIC_STATUS 2 
#define P2VIC_STATUS 4 
#define DRAW_STATUS	 1 
#define player_index(status) (status>>1)

extern const char *init_board;
extern const char *init_mb;

// nmove = "human move number" - 1
void init_pos(Position *pos, const char *bstr, const char *mbstr, const unsigned int turn, const unsigned int nmove);

void make_move(Position *pos, move_type m);

int legal_moves(Position *pos, move_type *moves);

void print_pos(Position *pos);

int mcode2imove(move_type m);

move_type guess_move(Position *pos, Position *npos);

#endif
