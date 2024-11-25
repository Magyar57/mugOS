#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

// ┌──┐
// │█ │
// └──┘

#define BLOCK_SIZE 2

void clearScreen();

// Get the printable char* buff_out from a given line in the game board g_Board
void getBoardLine(Board board, int line, char* buff_out);

// Get the printable char* menu_out from a given line in the game draw window
void getMenuLine(int line, char* buff_out, TetrominoType next, TetrominoType storage);

void setColors(Board board, TetrominoType next, TetrominoType storage);

#endif
