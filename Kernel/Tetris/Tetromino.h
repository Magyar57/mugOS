#ifndef __TETROMINO_H__
#define __TETROMINO_H__

#include <stdint.h>

typedef enum e_TetrominoBlock {
	BLOCK_CYAN,		// TetrominoBlock I (Bar)
	BLOCK_YELLOW,	// TetrominoBlock O (Block)
	BLOCK_PURPLE,	// TetrominoBlock T
	BLOCK_ORANGE,	// TetrominoBlock L
	BLOCK_BLUE,		// TetrominoBlock J (Inverted L)
	BLOCK_RED,		// TetrominoBlock Z (Biais)
	BLOCK_GREEN,	// TetrominoBlock S (Inversed biais)
	BLOCK_NONE
} TetrominoBlock;

typedef enum e_Tetromino {
	TETROMINO_I,
	TETROMINO_O,
	TETROMINO_T,
	TETROMINO_L,
	TETROMINO_J,
	TETROMINO_Z,
	TETROMINO_S,
	TETROMINO_NONE // Not present
} TetrominoType;

// Number of available Tetrominos
#define TETROMINO_AMOUNT 7

// Size will be variable: 2x2, 3x3 or 4x4
typedef uint8_t const* TetrominoLayout;

typedef struct s_Tetromino {
	TetrominoType type;
	TetrominoLayout layout;
	int pos_line;
	int pos_column;
	uint8_t rotation;
} Tetromino;

// Initalize a tetromino
void initalizeTetromino(Tetromino* out, TetrominoType t);

// Rotate (modifies the layout field) clockwise
void rotateTetromino(Tetromino* out);

// Return the block which the TetrominoType is made of
inline TetrominoBlock getBaseBlock(TetrominoType t){
	// We can do this because we set the order the same in the enums
	return (TetrominoBlock) t;
}

// Return the layout associated with a given tetromino and rotation
TetrominoLayout getTetrominoLayout(TetrominoType t, uint8_t rotation);

// Get the size (square size) of the layout of a given tetromino type
// For example, for a tetromino T, will return 3
// As the array is of size 3*3 = 9 values
inline int getLayoutSize(TetrominoType t){
	switch (t){
		case TETROMINO_I:
			return 4;
		case TETROMINO_O:
			return 2;
		case TETROMINO_T:
			return 3;
		case TETROMINO_L:
			return 3;
		case TETROMINO_J:
			return 3;
		case TETROMINO_Z:
			return 3;
		case TETROMINO_S:
			return 3;
		case TETROMINO_NONE:
			return 0;
		default:
			return 0;
	}
}

#endif
