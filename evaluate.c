#include "evaluate.h"
#include "mctrip.h"
#include "constants.h"
#include <stdlib.h>
#include <stdio.h>
/*
int rollout(Position *pos) {
    runp->roll_count++;
    while (!pos->status) {
        int mbi, bi;
        // select board
        board_type invalidmb = ~pos->validmb & BFULL;
        board_type mbvic = mbvmove_mb(pos);
        board_type overs = pos->overs[0] | pos->overs[1] | pos->overs[2];
        if (mask2num[pos->validmb] > 1) {
            if (mbvic && (MWCRAND & 31) < runp->roll_mbvic) // choose third board % of times
                invalidmb = ~mbvic & 511;
            else if (isset(pos->validmb, 4) && (MWCRAND & 31) < runp->roll_center) // choose central board % of times
                invalidmb = ~(1<<4) & BFULL;
            mbi = free_board_moves[invalidmb][1 + MWCRAND % free_board_moves[invalidmb][0]];            
            // select place
            board_type fullboard = pos->boards[0][mbi];
            // choose place for board victory % of times or always if macro board victory
            board_type vboard = third_place[pos->boards[pos->turn][mbi]] & ~fullboard;
            if (vboard && isset(mbvic, mbi)) // || (MWCRAND & 31) < runp->roll_vic))
                fullboard = ~vboard & 511;
            bi = free_board_moves[fullboard][1 + MWCRAND % free_board_moves[fullboard][0]];
        }
        else {
            mbi = free_board_moves[invalidmb][1];            
            // select place
            board_type fullboard = pos->boards[0][mbi];
            // avoid central place, overs and this board if over % of times (if there are other options)            
            //if (!isset(mbvic, mbi) && (~(1<<4) & ~fullboard & BFULL) && (MWCRAND & 31) < runp->roll_cplace)
            //    fullboard = fullboard | (1<<4);
            // mask free-move places % of times if there are other options
            //if (!isset(mbvic, mbi) && (~overs & ~fullboard & BFULL) && (MWCRAND & 31) < runp->roll_fm)
            //    fullboard = overs | fullboard;
            // STRONG/WEAK BOARD VICTORY?? add chosen board to free-move?


            // choose place for board victory % of times or always if macro board victory
            board_type vboard = third_place[pos->boards[pos->turn][mbi]] & ~fullboard;
            if (vboard && (isset(mbvic, mbi) || (MWCRAND & 31) < runp->roll_vic))
                fullboard = ~vboard & 511;
            bi = free_board_moves[fullboard][1 + MWCRAND % free_board_moves[fullboard][0]];
        }
        // execute selected move
        pos->boards[pos->turn][mbi] |= (1 << bi);
        pos->boards[0][mbi] |= (1 << bi);
        pos->overs[pos->turn] |= (board_victory[pos->boards[pos->turn][mbi]] << mbi);
        board_type fullb = pos->boards[0][mbi];
        pos->overs[0] |= ((mask2num[fullb] >> 3) & mask2num[fullb]) << mbi; // hack for full boards
        overs = pos->overs[0] | pos->overs[1] | pos->overs[2];
        pos->status = (board_victory[pos->overs[pos->turn]] << pos->turn) | ((mask2num[overs] >> 3) & mask2num[overs]);
        pos->validmb = isset(overs, bi) ? ~overs&BFULL : (1 << bi);
        pos->turn = 1 + (pos->turn & 1);
        //pos->nmove++; no need to update this
    }
    return pos->status;
}
*/

int rollout(Position *pos) {
    runp->roll_count++;
    while (!pos->status) {
        int mbi, bi;
        // select board
        board_type mbvic = mbvmove_mb(pos);
        if (mask2num[pos->validmb] > 1) {
            // free move on entire macroboard
            if (mbvic) {
                // potential macroboard victory
                board_type fullmb = ~mbvic & 511;
                int i = 1;
                while(i <= free_board_moves[fullmb][0]) {
                    mbi = free_board_moves[fullmb][i];
                    if (third_place[pos->boards[pos->turn][mbi]] & ~pos->boards[0][mbi]) {
                        // macroboard victory move
                        board_type vboard = third_place[pos->boards[pos->turn][mbi]] & ~pos->boards[0][mbi];
                        bi = free_board_moves[~vboard & 511][1];
                        goto execute_selected_move;
                    }
                    i++;
                }
            }
            // select random board
			board_type fullmb = ~pos->validmb & BFULL;
			mbi = free_board_moves[fullmb][1 + MWCRAND % free_board_moves[fullmb][0]];
		} else {
            // single board
            mbi = free_board_moves[~pos->validmb & BFULL][1];
            if (isset(mbvic, mbi)) {
                // macroboard victory move
                board_type vboard = third_place[pos->boards[pos->turn][mbi]] & ~pos->boards[0][mbi];
                if (vboard) {
                    bi = free_board_moves[~vboard & BFULL][1];
                    goto execute_selected_move;                    
                }
            }
        }
        // select random place
        board_type fullboard = pos->boards[0][mbi];
        bi = free_board_moves[fullboard][1 + MWCRAND % free_board_moves[fullboard][0]];
        
        // reselect if move misses a clean board victory
        board_type vboard = third_place[pos->boards[pos->turn][mbi]] & ~(pos->boards[0][mbi] | pos->overs[0] | pos->overs[1] | pos->overs[2]);
        if (vboard && !isset(vboard, bi))
            bi = free_board_moves[fullboard][1 + MWCRAND % free_board_moves[fullboard][0]];
        
    execute_selected_move:        
        pos->boards[pos->turn][mbi] |= (1 << bi);
        pos->boards[0][mbi] |= (1 << bi);
        pos->overs[pos->turn] |= (board_victory[pos->boards[pos->turn][mbi]] << mbi);
        board_type fullb = pos->boards[0][mbi];
        pos->overs[0] |= ((mask2num[fullb] >> 3) & mask2num[fullb]) << mbi; // hack for full boards
        board_type overs = pos->overs[0] | pos->overs[1] | pos->overs[2];
        pos->status = (board_victory[pos->overs[pos->turn]] << pos->turn) | ((mask2num[overs] >> 3) & mask2num[overs]);
        pos->validmb = isset(overs, bi) ? ~overs&BFULL : (1 << bi);
        pos->turn = 1 + (pos->turn & 1);
        pos->nmove++; //no need to update this
    }
    return pos->status;
}
