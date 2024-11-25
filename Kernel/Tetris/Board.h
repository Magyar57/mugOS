#ifndef __BOARD_H__
#define __BOARD_H__

#include <stdint.h>
#include <stdbool.h>

#include "Tetromino.h"

// ================ Board ================

#define BOARD_WIDTH 10
#define BOARD_HEIGHT 20
#define BOARD_SIZE BOARD_WIDTH*BOARD_HEIGHT
typedef TetrominoBlock Board[BOARD_SIZE];

void initalizeBoard(Board board);
void setBlock(Board b, int line, int column, TetrominoBlock new_block);
void putTetromino(Board board, Tetromino* t);
void removeTetromino(Board board, Tetromino* t);
bool collides(Board board, Tetromino* t, int line, int column, uint8_t rotation);
void removeFullLines(Board board);

// ================ Game ================

#endif
