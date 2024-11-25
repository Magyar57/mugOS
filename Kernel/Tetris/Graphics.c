#include <stddef.h>
#include "string.h"
#include "assert.h"
#include "Board.h"

#include "Graphics.h"

#define COLOR_CYAN		0x03
#define COLOR_YELLOW	0x0e
#define COLOR_PURPLE	0x05
#define COLOR_ORANGE	0x0c
#define COLOR_BLUE		0x01
#define COLOR_RED		0x04
#define COLOR_GREEN		0x02
#define COLOR_RESET		0x07

static int putTetrominosBlock(TetrominoBlock block, char* out){
	// const char BLOCK_STR[] = {219, 219}; // "€€"

	switch (block){
		case BLOCK_NONE:
			// strcpy(out, " .");
			out[0] = ' ';
			out[1] = '.';
			break;
		case BLOCK_CYAN:
		case BLOCK_YELLOW:
		case BLOCK_PURPLE:
		case BLOCK_ORANGE:
		case BLOCK_BLUE:
		case BLOCK_RED:
		case BLOCK_GREEN:
			// strcpy(out, BLOCK_STR);
			out[0] = 219;
			out[1] = 219;
			break;
		default:
			return 0;
	}
	return 2;
}

static uint8_t getBlockColor(TetrominoBlock t){
	switch (t){
		case BLOCK_NONE:
			return COLOR_RESET;
		case BLOCK_CYAN:
			return COLOR_CYAN;
		case BLOCK_YELLOW:
			return COLOR_YELLOW;
		case BLOCK_PURPLE:
			return COLOR_PURPLE;
		case BLOCK_ORANGE:
			return COLOR_ORANGE;
		case BLOCK_BLUE:
			return COLOR_BLUE;
		case BLOCK_RED:
			return COLOR_RED;
		case BLOCK_GREEN:
			return COLOR_GREEN;

		default:
			return COLOR_RESET;
	}
}

void getBoardLine(Board board, int line, char* buff_out){
	// Note: we assume that buff has sufficient size to hold all the escaped characters

	const int line_offset = line*BOARD_WIDTH;

	int buffer_index = 0;
	for(int i=0 ; i<BOARD_WIDTH ; i++){
		TetrominoBlock block = board[line_offset + i];
		buffer_index += putTetrominosBlock(block, buff_out+buffer_index);
	}
}

static void getTetrominoLine(char* buff_out, TetrominoType tetro, int line){
	int layout_size = getLayoutSize(tetro);
	int padding = 2 + 4-layout_size;
	TetrominoBlock block = getBaseBlock(tetro);
	const int rotation = (tetro==TETROMINO_T || tetro==TETROMINO_L) ? 2 : 0;
	const TetrominoLayout layout = getTetrominoLayout(tetro, rotation);
	// const TetrominoLayout layout = getTetrominoLayout(tetro, 0);
	int buffer_index = 0;

	// Write border and padding
	// strcpy(buff_out, "≥");
	strcpy(buff_out, "≥");
	buffer_index += 1;
	for(int i=0 ; i<padding ; i++) buff_out[buffer_index+i] = ' ';
	buffer_index += padding;

	if (line == 0 || (layout_size==2 && line==3)){
		strcpy(buff_out, "≥            ≥");
		return;
	}

	// Rectify line to point to the line offset in the layout
	line--;

	// Draw tetro line
	for(int i=0 ; i<layout_size ; i++){
		if (layout[line*layout_size + i] != 0){
			buffer_index += putTetrominosBlock(block, buff_out+buffer_index);
		}
		else{
			buff_out[buffer_index++] = ' ';
			buff_out[buffer_index++] = ' ';
		}
	}

	// Padding and border
	for(int i=0 ; i<padding ; i++) buff_out[buffer_index+i] = ' ';
	buffer_index += padding;
	strcpy(buff_out+buffer_index, "≥");
}

// This function is horrible, I love technical debt
void getMenuLine(int line, char* buff_out, TetrominoType next, TetrominoType storage){
	assert(buff_out != NULL);
	assert(next != TETROMINO_NONE);

	// Next tetromino
	if (line == 0 || line == 1 || line == 2 || line == 3)
		getTetrominoLine(buff_out, next, line);
	if (line == 4)
		strcpy(buff_out, "¿ƒƒƒƒƒƒƒƒƒƒƒƒŸ");

	// Stored tetromino
	#define STORED_OFFSET 5
	if (line==STORED_OFFSET)
		strcpy(buff_out, "⁄ƒƒƒStoredƒƒƒø");
	if (line == STORED_OFFSET+1 || line == STORED_OFFSET+2 || line == STORED_OFFSET+3 || line == STORED_OFFSET+4){
		// Empty storage
		if (storage==TETROMINO_NONE) {
			strcpy(buff_out, "≥            ≥");
			return;
		}
		getTetrominoLine(buff_out, storage, line-STORED_OFFSET-1);
	}
	if (line == STORED_OFFSET+5)
		strcpy(buff_out, "¿ƒƒƒƒƒƒƒƒƒƒƒƒŸ");

	// Help
	if (line == 11)
		strcpy(buff_out, "⁄ƒƒƒƒHelpƒƒƒƒø");
	if (line == 12)
		strcpy(buff_out, "≥ Rotate   o ≥");
	if (line == 13)
		strcpy(buff_out, "≥ Left     k ≥");
	if (line == 14)
		strcpy(buff_out, "≥ Down     l ≥");
	if (line == 15)
		strcpy(buff_out, "≥ Right    m ≥");
	if (line == 16)
		strcpy(buff_out, "≥ Drop space ≥");
	if (line == 17)
		strcpy(buff_out, "≥ Store    j ≥");
	if (line == 18)
		strcpy(buff_out, "≥ Quit   esc ≥");
	if (line == 19)
		strcpy(buff_out, "¿ƒƒƒƒƒƒƒƒƒƒƒƒŸ");
}

// Note: uses directly the VGA video memory
void setColors(Board board, TetrominoType next, TetrominoType storage){
	uint8_t* const VIDEO_MEMORY = (uint8_t*) 0xb8000;

	for(int i=0 ; i<BOARD_HEIGHT ; i++){
		for (int j=0 ; j<BOARD_WIDTH ; j++){
			TetrominoBlock cur = board[i*BOARD_WIDTH + j];
			if (cur != BLOCK_NONE){
				// index in board: (i, j)
				// index in VIDEO_MEMORY: (i+1, 4j + 6)
				// flattened: (i+1)*SCREEN_WIDTH*2 + 4j+6
				// And add the offset 1 for color
				VIDEO_MEMORY[(i+1)*80*2 + 4*j+6 +1] = getBlockColor(cur);
				// Second character (blocks are two char wide)
				VIDEO_MEMORY[(i+1)*80*2 + 4*j+6 +3] = getBlockColor(cur);
			}
		}
	}

	int colorNext = getBlockColor(next);
	int colorStorage = getBlockColor(storage);
	for (int i=0 ; i<8 ; i++){
		VIDEO_MEMORY[(2)*80*2 + (29+i)*2 +1] = colorNext;
		VIDEO_MEMORY[(3)*80*2 + (29+i)*2 +1] = colorNext;
		VIDEO_MEMORY[(8)*80*2 + (29+i)*2 +1] = colorStorage;
		VIDEO_MEMORY[(9)*80*2 + (29+i)*2 +1] = colorStorage;
	}
}
