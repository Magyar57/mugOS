#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__

// ┌──┐
// │█ │
// └──┘

// At maximum, in the line string, printing a colorized block will take up 21B:
// sizeof(BLOCK_STR) + sizeof(COLOR_whatever) + sizeof(COLOR_RESET) = 6 + 11 + 4 = 21
#define MAX_BLOCK_SIZE 21

void clearScreen();

// Get the printable char* buff_out from a given line in the game board g_Board
void getBoardLine(Board board, int line, char* buff_out);

// Get the printable char* menu_out from a given line in the game draw window
void getMenuLine(int line, char* buff_out, TetrominoType next, TetrominoType storage);

#endif
