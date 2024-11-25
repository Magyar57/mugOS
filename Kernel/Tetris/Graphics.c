// #include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include "Board.h"

#include "Graphics.h"

#define COLOR_CYAN		"" //"\033[38:5:51m"
#define COLOR_YELLOW	"" //"\033[38:5:226m"
#define COLOR_PURPLE	"" //"\033[38:5:57m"
#define COLOR_ORANGE	"" //"\033[38:5:202m"
#define COLOR_BLUE		"" //"\033[38:5:20m"
#define COLOR_RED		"" //"\033[38:5:196m"
#define COLOR_GREEN		"" //"\033[38:5:46m"
#define COLOR_RESET		"" //"\033[0m"

#define FINAL_SIZEOF(COLOR) sizeof(COLOR) + 7 + sizeof(COLOR_RESET) -3
#define FINAL_SIZEOF(COLOR) sizeof(COLOR) + 7 + sizeof(COLOR_RESET) -3

static int putTetrominosBlock(TetrominoBlock block, char* out){
	const char BLOCK_STR[] = {219, 219}; // "€€"

	switch (block){
		case BLOCK_NONE:
			// strcpy(out, " .");
			out[0] = ' ';
			out[1] = '.';
			break;
		case BLOCK_CYAN:
			// strcpy(out, BLOCK_STR);
			out[0] = 219;
			out[1] = 219;
			break;
		case BLOCK_YELLOW:
			// strcpy(out, BLOCK_STR);
			out[0] = 219;
			out[1] = 219;
			break;
		case BLOCK_PURPLE:
			// strcpy(out, BLOCK_STR);
			out[0] = 219;
			out[1] = 219;
			break;
		case BLOCK_ORANGE:
			// strcpy(out, BLOCK_STR);
			out[0] = 219;
			out[1] = 219;
			break;
		case BLOCK_BLUE:
			// strcpy(out, BLOCK_STR);
			out[0] = 219;
			out[1] = 219;
			break;
		case BLOCK_RED:
			// strcpy(out, BLOCK_STR);
			out[0] = 219;
			out[1] = 219;
			break;
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
