#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "position.h"

int mbi2imove(move_type mbi, move_type bi) {
    unsigned int mbx = mbi%3; unsigned int mby = mbi/3;
    unsigned int bix = bi%3; unsigned int biy = bi/3;
    return 27*mby + 9*biy + 3*mbx + bix;
}

void imove2mbi(unsigned int i, unsigned int *mbi, unsigned int *bi) {
    unsigned int x = i%9; unsigned int y = i/9;
    unsigned int mbx = x/3; unsigned int mby = y/3;
    unsigned int bix = x - 3*mbx; int biy = y - 3*mby;
    *mbi = mby*3 + mbx;
    *bi = 3*biy + bix ;
}

void init_pos(Position *pos, const char *bstr, const char *mbstr, const unsigned int turn, const unsigned int nmove) {
	pos->status = 0;
	pos->nmove = nmove;
	pos->turn = turn;
	pos->validmb = pos->overs[0] = pos->overs[1] = pos->overs[2] = 0;
	for(unsigned int i=0; i<9; i++) {
		pos->boards[0][i] = pos->boards[1][i] = pos->boards[2][i] = 0;
	}

    unsigned int mbi, bi;
	for(unsigned int i=0; *bstr!='\0'; bstr++) {
        if(*bstr=='1') {
            imove2mbi(i, &mbi, &bi);
            pos->boards[1][mbi] |= (1 << bi);
			pos->boards[0][mbi] |= (1 << bi);
			i++;
        } else if(*bstr=='2') {
            imove2mbi(i, &mbi, &bi);
            pos->boards[2][mbi] |= (1 << bi);
			pos->boards[0][mbi] |= (1 << bi);
			i++;
        } else if(*bstr=='0') {
            i++;
        }
	}
	for(unsigned int i=0; *mbstr!='\0'; mbstr++) {
		if(*mbstr=='-') {
			pos->validmb |= (1<<i);
			mbstr++; // skip 1 char
			i++;
		} else if(*mbstr=='0' && is_full(pos->boards[0][i])) {
			pos->overs[0] |= (1<<i);
			i++;
		} else if(*mbstr=='1') {
			pos->overs[1] |= (1<<i);
			i++;
		} else if(*mbstr=='2') {
			pos->overs[2] |= (1<<i);
			i++;
		} else if(*mbstr=='0') {
			i++;
		}
	}
	if(board_victory[pos->overs[1]])
		pos->status = P1VIC_STATUS;
	else if(board_victory[pos->overs[2]])
		pos->status = P2VIC_STATUS;
	else if(is_full(pos->overs[0] | pos->overs[1] | pos->overs[2]))
		pos->status = DRAW_STATUS;
	else
		pos->status = 0;
}

void make_move(Position *pos, move_type m) {
	decode_move(mbi, bi, m)
	// update boards
	pos->boards[pos->turn][mbi] |= (1 << bi);
	pos->boards[0][mbi] |= (1 << bi);
	// update macroboards, status, validmb, turn and move number	
	pos->overs[pos->turn] |= (board_victory[pos->boards[pos->turn][mbi]] << mbi);
    board_type fullb = pos->boards[0][mbi] | pos->boards[1][mbi];
    pos->overs[0] |= ((mask2num[fullb] >> 3) & mask2num[fullb]) << mbi;
	board_type overs = pos->overs[0] | pos->overs[1] | pos->overs[2];
    pos->status = (board_victory[pos->overs[pos->turn]] << pos->turn) | ((mask2num[overs] >> 3) & mask2num[overs]);
	pos->validmb = isset(overs, bi) ? ~overs&BFULL : (1 << bi);
	pos->turn = 1 + (pos->turn&1);
	pos->nmove++;
}

int legal_moves(Position *pos, move_type *moves) {
	int nm = 0;
	const char *vmbs = free_board_moves[~pos->validmb & BFULL];
	for (int vi = 1; vi <= vmbs[0]; vi++) {
		move_type mbi = (move_type)vmbs[vi];
		const char *fbs = free_board_moves[pos->boards[0][mbi]];
		for (int bi = 1; bi <= fbs[0]; bi++) {
			moves[nm] = code_move(mbi, (move_type)fbs[bi]);
			nm++;
		}
	}
	return nm;
}

void print_pos(Position *pos) {
	for (unsigned int i = 0; i<81; i++) {
		if (i % 3 == 0)
			printf(" ");
		if (i % 9 == 0)
			printf("\n");
		if (i % 27 == 0)
			printf("\n");
		unsigned int mbi; unsigned int bi;
		imove2mbi(i, &mbi, &bi);
		if (pos->boards[1][mbi] & (1 << bi))
			printf("1");
		else if (pos->boards[2][mbi] & (1 << bi))
			printf("2");
		else if (pos->boards[1][mbi] & (1 << bi) && pos->boards[2][mbi] & (1 << bi))
			printf("3");
		else
			printf("0");
	}
	printf("\n");
	for (unsigned int i = 0; i<9; i++) {
		if (pos->validmb&(1 << i))
			printf("-1,");
		else if (pos->overs[1]&(1 << i))
			printf("1,");
		else if (pos->overs[2]&(1 << i))
			printf("2,");
		else
			printf("0,");
	}
	printf("\nnmove=%d status=%d\n", pos->nmove, pos->status);
}

int mcode2imove(move_type m) {
    decode_move(mbi, bi, m)
    return mbi2imove(mbi, bi);
}

move_type guess_move(Position *pos, Position *npos) {
	move_type move = NOMOVE;
	for (move_type mbi = 0; mbi<9; mbi++)
		if (pos->boards[pos->turn][mbi] != npos->boards[pos->turn][mbi]) {
			for (move_type bi = 0; bi<9; bi++) {
				if (!isset(pos->boards[pos->turn][mbi], bi) && isset(npos->boards[pos->turn][mbi], bi)) {
					if (move != NOMOVE) // there can be only one!
						return NOMOVE;
					move = code_move(mbi, bi);
				}
			}
		}
	return move;
}