// #include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include "Board.h"

void initalizeBoard(Board board){
	for(int i=0 ; i<BOARD_SIZE ; i++) {
		board[i] = BLOCK_NONE;
	}
}

void setBlock(Board b, int line, int column, TetrominoBlock new_block){
	b[line*BOARD_WIDTH + column] = new_block;
}

static void setTetromino(Board board, Tetromino* t, TetrominoBlock b){
	assert(t!=NULL);
	int layout_size = getLayoutSize(t->type);
	uint8_t quotient, remainder;

	// We assume that we CAN place the tetro here
	// Check is supposed to be done before

	for(int i=0 ; i<layout_size*layout_size ; i++){
		quotient = i / layout_size;
		remainder = i % layout_size;
		if (t->layout[i] != 0){
			setBlock(board, t->pos_line + quotient, t->pos_column + remainder, b);
		}
	}
}

void putTetromino(Board board, Tetromino* t){
	assert(t!=NULL);
	TetrominoBlock block = getBaseBlock(t->type);
	setTetromino(board, t, block);
}

void removeTetromino(Board board, Tetromino* t){
	assert(t!=NULL);
	setTetromino(board, t, BLOCK_NONE);
}

static inline bool isOutOfBoard(int line, int column){
	return (line<0 || line>=BOARD_HEIGHT || column<0 || column>=BOARD_WIDTH);
}

bool collides(Board board, Tetromino* tetro, int line, int column, uint8_t rotation){
	assert(tetro!=NULL && tetro->layout != NULL);
	const TetrominoLayout layout = getTetrominoLayout(tetro->type, rotation);
	const int layout_size = getLayoutSize(tetro->type);
	int absolute_i, absolute_j;

	for (int i=0 ; i<layout_size ; i++){
		for (int j=0 ; j<layout_size ; j++){
			if (layout[i*layout_size + j] != 0){
				absolute_i = line + i;
				absolute_j = column + j;

				// Check for border collision
				if (isOutOfBoard(absolute_i, absolute_j))
					return true;

				// Check for blocks collision
				if (board[absolute_i*BOARD_WIDTH + absolute_j] != BLOCK_NONE){
					return true;
				}
			}
		}
	}

	return false;
}

static void moveDown(Board board, int last_line){
	for(int line=last_line ; line>0 ; line--){
		// Copy previous line into line
		memcpy(board+(line*BOARD_WIDTH), board+((line-1)*BOARD_WIDTH), sizeof(TetrominoBlock)*BOARD_WIDTH);
	}

	// Clear top line
	for(int i=0 ; i<BOARD_WIDTH ; i++)
		board[0*BOARD_WIDTH + i] = BLOCK_NONE;
}

static inline bool isLineFull(Board board, int line){
	assert(line>=0 && line<BOARD_HEIGHT);

	for(int i=0 ; i<BOARD_WIDTH ; i++){
		if (board[line*BOARD_WIDTH + i] == BLOCK_NONE){
			return false;
		}
	}

	return true;
}

void removeFullLines(Board board){
	for (int i=0 ; i<BOARD_HEIGHT ; i++){
		if (isLineFull(board, i)){
			moveDown(board, i);
		}
	}
}
