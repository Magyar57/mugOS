#include <stdio.h>
// #include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include "Tetromino.h"
#include "Board.h"

#include "TetrominoManager.h"

TetrominoType g_storedTetro;

TetrominoType g_tetrominoBucket[TETROMINO_AMOUNT];
#define BUCKET_SIZE sizeof(g_tetrominoBucket)/sizeof(TetrominoType)

int g_bucketIndex;

// Shuffles the bucket with the [Fisher–Yates](https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle) algorithm
// Not available yet since rand is not implemented by the kernel
static void shuffleBucket(){
	// for(int i=0 ; i<BUCKET_SIZE ; i++){
	// 	int j = rand() % (i+1);

	// 	int temp = g_tetrominoBucket[i];
	// 	g_tetrominoBucket[i] = g_tetrominoBucket[j];
	// 	g_tetrominoBucket[j] = temp;
	// }
}

static void resetBucket(){
	g_bucketIndex = 0;

	g_tetrominoBucket[0] = TETROMINO_I;
	g_tetrominoBucket[1] = TETROMINO_O;
	g_tetrominoBucket[2] = TETROMINO_T;
	g_tetrominoBucket[3] = TETROMINO_L;
	g_tetrominoBucket[4] = TETROMINO_J;
	g_tetrominoBucket[5] = TETROMINO_Z;
	g_tetrominoBucket[6] = TETROMINO_S;
	shuffleBucket();
}

void initalizeTetrominoManager(){
	g_storedTetro = TETROMINO_NONE;
	resetBucket();
}

void nextTetromino(Tetromino* cur){
	TetrominoType new_type = g_tetrominoBucket[g_bucketIndex];
	initalizeTetromino(cur, new_type);

	g_bucketIndex++;
	if (g_bucketIndex >= BUCKET_SIZE) resetBucket();
}

void storeTetromino(Tetromino* cur){
	// For 1st storage, behaviour is different
	if (g_storedTetro == TETROMINO_NONE){
		g_storedTetro = cur->type;
		nextTetromino(cur);
		return;
	}

	// Swap with current storage
	TetrominoType temp = g_storedTetro;
	g_storedTetro = cur->type;
	initalizeTetromino(cur, temp);
}

TetrominoType getNextTetrominoType(){
	return g_tetrominoBucket[g_bucketIndex];
}

TetrominoType getStoredTetrominoType(){
	return g_storedTetro;
}
