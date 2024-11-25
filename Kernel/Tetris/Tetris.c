#include <stddef.h>
#include <stdbool.h>
#include "stdio.h"
#include "string.h"
#include "TetrisKeyboard.h"
#include "Board.h"
#include "TetrominoManager.h"
#include "Graphics.h"
#include "Drivers/Keycodes.h"
#include "Panic.h"

#include "Tetris.h"

// Note: we use the nanosleep and clock_gettime POSIX functions

#define DEBUG false

// Game variables
bool g_ShouldQuit = false;
bool g_gameOver;
bool g_tetroWasStoredThisStep;
Board g_Board;
Tetromino g_curTetro;
char g_debugString[256];

#define GAME_STEP			1000	// Step every 1000 ms
#define TARGET_FRAMETIME	16.667	// 16 ms => 60 fps

// Mapping
#define CMD_UP		KEY_I
#define CMD_ROTATE	KEY_O
#define CMD_DOWN	KEY_L
#define CMD_LEFT	KEY_K
#define CMD_RIGHT	KEY_M
#define CMD_STORE	KEY_J
#define CMD_QUIT	KEY_ESC
#define CMD_RESTART	KEY_R
#define CMD_PLACE	KEY_SPACE
// KEY_SPACE

void initalizeGame(){
	g_gameOver = false;
	g_tetroWasStoredThisStep = false;
	// srand(time(NULL));
	TetrisKeyboard_initalize();
	memset(g_debugString, 0, sizeof(g_debugString));
	initalizeBoard(g_Board);
	initalizeTetrominoManager();
	nextTetromino(&g_curTetro);
}

void updateDebugString(double dt, double waited){
	// sprintf(g_debugString, "[DEBUG] %d UPS (%.2f ms = dt:%.2f ms + wait:%.2f ms) - next=%d stored=%d",
	// 	(int)(1000.0/(dt+waited)), dt+waited, dt, waited, getNextTetrominoType(), getStoredTetrominoType());
}

void step(double dt, double waited){
	// First we update the debug string
	if (DEBUG) updateDebugString(dt, waited);

	// Check if we collide by going down
	bool collide = collides(g_Board, &g_curTetro, g_curTetro.pos_line+1, g_curTetro.pos_column, g_curTetro.rotation);

	// If not, we fall
	if(!collide){
		g_curTetro.pos_line++;
		return;
	}

	// Otherwise, we can step to the next tetromino
	putTetromino(g_Board, &g_curTetro);
	removeFullLines(g_Board);
	nextTetromino(&g_curTetro);
	g_tetroWasStoredThisStep = false;

	// If the new tetro collides, we lost !
	if (collides(g_Board, &g_curTetro, g_curTetro.pos_line, g_curTetro.pos_column, g_curTetro.rotation)){
		g_gameOver = true;
	}
}

void update_gameOver(){
	Keycode key = TetrisKeyboard_getKey();

	switch (key){
		case CMD_QUIT:
			g_ShouldQuit = true;
			break;
		case CMD_RESTART:
			initalizeGame();
			break;
	default:
		break;
	}
}

void update_playing(double dt, double waited){
	static double time_since_step = 0.0;

	Keycode key = TetrisKeyboard_getKey();
	time_since_step += dt+waited;

	if (time_since_step >= GAME_STEP){
		step(dt, waited);
		time_since_step = 0.0;
		return;
	}

	switch (key){
		case KEY_RESERVED:
			break;
		case CMD_PLACE:
			while(!collides(g_Board, &g_curTetro, g_curTetro.pos_line+1, g_curTetro.pos_column, g_curTetro.rotation)){
				g_curTetro.pos_line++;
			}
			step(dt, waited);
			break;
		#if DEBUG
		case CMD_UP:
			if(!collides(g_Board, &g_curTetro, g_curTetro.pos_line-1, g_curTetro.pos_column, g_curTetro.rotation))
				g_curTetro.pos_line--;
			break;
		#endif
		case CMD_STORE:
			if (!g_tetroWasStoredThisStep){
				g_tetroWasStoredThisStep = true;
				storeTetromino(&g_curTetro);
				time_since_step = 0.0; // Reset step time
			}
			break;
		case CMD_LEFT:
			if(!collides(g_Board, &g_curTetro, g_curTetro.pos_line, g_curTetro.pos_column-1, g_curTetro.rotation))
				g_curTetro.pos_column--;
			break;
		case CMD_DOWN:
			if(!collides(g_Board, &g_curTetro, g_curTetro.pos_line+1, g_curTetro.pos_column, g_curTetro.rotation)){
				g_curTetro.pos_line++;
				time_since_step = 0.0; // Reset step time to prevent a surprising fall
			}
			break;
		case CMD_RIGHT:
			if(!collides(g_Board, &g_curTetro, g_curTetro.pos_line, g_curTetro.pos_column+1, g_curTetro.rotation))
				g_curTetro.pos_column++;
			break;
		case CMD_ROTATE:
			if (!collides(g_Board, &g_curTetro, g_curTetro.pos_line, g_curTetro.pos_column, (g_curTetro.rotation+1) % 4))
				rotateTetromino(&g_curTetro);
			break;
		case CMD_QUIT:
			g_ShouldQuit = true;
			break;
		default:
			break;
	}
}

void update(double dt, double waited){
	g_gameOver ? update_gameOver() : update_playing(dt, waited);
}

void draw(){
	clear_screen();

	// Put the tetromino for drawing, we'll remove it after
	// We suppose that update does not put it in a colliding position
	putTetromino(g_Board, &g_curTetro);

	const int BOARD_BUFFSIZE = BOARD_WIDTH*BLOCK_SIZE + 1; // +1 for the final '\0'
	const int MENU_BUFFSIZE = 4 * BLOCK_SIZE + 4;
	char board_line_buff[BOARD_BUFFSIZE];
	char menu_buff[MENU_BUFFSIZE];

	// puts("  ┌───────Tetris───────┐  ┌────Next────┐");
	const char HEADER[] = {' ', ' ', 218, 196, 196, 196, 196, 196, 196, 196, 'T', 'e', 't', 'r', 'i', 's',
		196, 196, 196, 196, 196, 196, 196, 191, ' ', ' ', 218, 196, 196, 196, 196, 'N', 'e', 'x', 't', 196, 196, 196, 196, 191,'\0'};
	// puts("  └────────────────────┘");
	const char FOOTER[] = {' ', ' ', 192, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196, 196,
		196, 196, 196, 196, 196, 196, 196, 217, '\0'};
	// printf("  │%s│  %s\n", board_line_buff, menu_buff);
	const char LINE[] = {' ', ' ', 179, '%', 's', 179, ' ', ' ', '%', 's', '\n', '\0'};

	puts(HEADER);
	for(int i=0 ; i<BOARD_HEIGHT ; i++){
		memset(board_line_buff, 0, BOARD_BUFFSIZE);
		memset(menu_buff, 0, MENU_BUFFSIZE);

		getBoardLine(g_Board, i, board_line_buff);
		getMenuLine(i, menu_buff, getNextTetrominoType(), getStoredTetrominoType());

		printf(LINE, board_line_buff, menu_buff);
	}
	puts(FOOTER);
	if (DEBUG) puts(g_debugString);
	if (g_gameOver) puts("Game over ! Press Escape to quit or R to replay :)");

	setColors(g_Board, getNextTetrominoType(), getStoredTetrominoType());
	removeTetromino(g_Board, &g_curTetro);
}

void Tetris_runGame() {
	initalizeGame();

	while(!g_ShouldQuit) {
		draw();
		update(16.667, 0.0);

		for(int i=0 ; i<10000000 ; i++); // active wait
	}

	puts("Quited tetris, halting");
}
