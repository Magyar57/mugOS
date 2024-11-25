#include <stddef.h>
#include <stdio.h>
// #include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include "Board.h"

#include "Tetromino.h"

#pragma region Layouts and rotations

static const uint8_t Layout_I_r0[] = {
	1, 1, 1, 1,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
};
static const uint8_t Layout_I_r1[] = {
	1, 0, 0, 0,
	1, 0, 0, 0,
	1, 0, 0, 0,
	1, 0, 0, 0,
};

static const uint8_t Layout_O[] = {
	1, 1,
	1, 1,
};

static const uint8_t Layout_T_r0[] = {
	0, 0, 0,
	1, 1, 1,
	0, 1, 0,
};
static const uint8_t Layout_T_r1[] = {
	0, 1, 0,
	1, 1, 0,
	0, 1, 0,
};
static const uint8_t Layout_T_r2[] = {
	0, 1, 0,
	1, 1, 1,
	0, 0, 0,
};
static const uint8_t Layout_T_r3[] = {
	0, 1, 0,
	0, 1, 1,
	0, 1, 0,
};

static const uint8_t Layout_L_r0[] = {
	0, 0, 0,
	1, 1, 1,
	1, 0, 0,
};
static const uint8_t Layout_L_r1[] = {
	1, 1, 0,
	0, 1, 0,
	0, 1, 0,
};
static const uint8_t Layout_L_r2[] = {
	0, 0, 1,
	1, 1, 1,
	0, 0, 0,
};
static const uint8_t Layout_L_r3[] = {
	0, 1, 0,
	0, 1, 0,
	0, 1, 1,
};

static const uint8_t Layout_J_r0[] = {
	1, 0, 0,
	1, 1, 1,
	0, 0, 0,
};
static const uint8_t Layout_J_r1[] = {
	0, 1, 1,
	0, 1, 0,
	0, 1, 0,
};
static const uint8_t Layout_J_r2[] = {
	0, 0, 0,
	1, 1, 1,
	0, 0, 1,
};
static const uint8_t Layout_J_r3[] = {
	0, 1, 0,
	0, 1, 0,
	1, 1, 0,
};

static const uint8_t Layout_Z_r0[] = {
	1, 1, 0,
	0, 1, 1,
	0, 0, 0,
};
static const uint8_t Layout_Z_r1[] = {
	0, 0, 1,
	0, 1, 1,
	0, 1, 0,
};
static const uint8_t Layout_Z_r2[] = {
	0, 0, 0,
	1, 1, 0,
	0, 1, 1,
};
static const uint8_t Layout_Z_r3[] = {
	0, 1, 0,
	1, 1, 0,
	1, 0, 0,
};

static const uint8_t Layout_S_r0[] = {
	0, 1, 1,
	1, 1, 0,
	0, 0, 0,
};
static const uint8_t Layout_S_r1[] = {
	0, 1, 0,
	0, 1, 1,
	0, 0, 1,
};
static const uint8_t Layout_S_r2[] = {
	0, 0, 0,
	0, 1, 1,
	1, 1, 0,
};
static const uint8_t Layout_S_r3[] = {
	1, 0, 0,
	1, 1, 0,
	0, 1, 0,
};

#pragma endregion

static inline int getTetrominoSpawnOffset(TetrominoType t){
	switch (t){
		case TETROMINO_I:
			return BOARD_WIDTH/2 -2;
		case TETROMINO_O:
			return BOARD_WIDTH/2 -1;
		default:
			return BOARD_WIDTH/2 -2;
	}
}

void initalizeTetromino(Tetromino* out, TetrominoType t){
	assert(t>=0 && t<TETROMINO_AMOUNT);
	assert(out != NULL);

	out->type = t;
	out->rotation = 0;
	out->layout = getTetrominoLayout(out->type, 0);
	out->pos_line = 0;
	out->pos_column = getTetrominoSpawnOffset(t);
}

void rotateTetromino(Tetromino* out){
	assert(out != NULL);
	out->rotation = (out->rotation+1) % 4;
	out->layout = getTetrominoLayout(out->type, out->rotation);
}

extern inline TetrominoBlock getBaseBlock(TetrominoType t);

TetrominoLayout getTetrominoLayout(TetrominoType t, uint8_t rotation){
	assert(rotation<4);
	
	switch(t*4 + rotation){
		case TETROMINO_I*4+0: return Layout_I_r0;
		case TETROMINO_I*4+1: return Layout_I_r1;
		case TETROMINO_I*4+2: return Layout_I_r0;
		case TETROMINO_I*4+3: return Layout_I_r1;

		case TETROMINO_O*4+0: return Layout_O;
		case TETROMINO_O*4+1: return Layout_O;
		case TETROMINO_O*4+2: return Layout_O;
		case TETROMINO_O*4+3: return Layout_O;

		case TETROMINO_T*4+0: return Layout_T_r0;
		case TETROMINO_T*4+1: return Layout_T_r1;
		case TETROMINO_T*4+2: return Layout_T_r2;
		case TETROMINO_T*4+3: return Layout_T_r3;

		case TETROMINO_L*4+0: return Layout_L_r0;
		case TETROMINO_L*4+1: return Layout_L_r1;
		case TETROMINO_L*4+2: return Layout_L_r2;
		case TETROMINO_L*4+3: return Layout_L_r3;

		case TETROMINO_J*4+0: return Layout_J_r0;
		case TETROMINO_J*4+1: return Layout_J_r1;
		case TETROMINO_J*4+2: return Layout_J_r2;
		case TETROMINO_J*4+3: return Layout_J_r3;

		case TETROMINO_Z*4+0: return Layout_Z_r0;
		case TETROMINO_Z*4+1: return Layout_Z_r1;
		case TETROMINO_Z*4+2: return Layout_Z_r2;
		case TETROMINO_Z*4+3: return Layout_Z_r3;

		case TETROMINO_S*4+0: return Layout_S_r0;
		case TETROMINO_S*4+1: return Layout_S_r1;
		case TETROMINO_S*4+2: return Layout_S_r2;
		case TETROMINO_S*4+3: return Layout_S_r3;
	}

	// fprintf(stderr, "getTetrominoLayout error: invalid combination of TetrominoType t, uint8_t rotation");
	puts("getTetrominoLayout error: invalid combination of TetrominoType t, uint8_t rotation");
	PANIC();

	return 0; // never reached
}

extern inline int getLayoutSize(TetrominoType t);
