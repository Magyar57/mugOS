#ifndef __TETROMINO_MANAGER_H__
#define __TETROMINO_MANAGER_H__

#include "Tetromino.h"
#include "Board.h"

void initalizeTetrominoManager();
void nextTetromino(Tetromino* cur);
void storeTetromino(Tetromino* cur);
TetrominoType getNextTetrominoType();
TetrominoType getStoredTetrominoType();

#endif
