#define F_CPU 7372800UL

#include <avr/cpufunc.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include "lcd_functions.h"

#define HOMESCREEN 0
#define NEW_PLAYER 1
#define CHOOSE_PLAYER 2
#define LEADERBOARDS 3
#define GAME 4

#define HS_S_Y 10 //homescreen starting y pos
#define HS_S_X 20
#define HS_H 60 //height
#define HS_W 280

#define CP_Y (HS_S_Y + HS_H + BLANK_SPACE) //choose player
#define CP_X (HS_S_X)
#define CP_H HS_H //height
#define CP_W HS_W

#define LB_Y (HS_S_Y + 2*HS_H + 2*BLANK_SPACE) //leaderboards
#define LB_X (HS_S_X)
#define LB_H HS_H //height
#define LB_W HS_W


#define BCK_BTN_Y 10
#define BCK_BTN_X 10
#define BCK_BTN_H 40
#define BCK_BTN_W 80

#define BLANK_SPACE 10
#define OK_BTN_H 40
#define OK_BTN_W 60
#define OK_BTN_Y (MAX_X - BLANK_SPACE - OK_BTN_H)
#define OK_BTN_X (MAX_Y - BLANK_SPACE - OK_BTN_W)

#define DEL_BTN_H 40
#define DEL_BTN_W 60
#define DEL_BTN_Y (MAX_X - BLANK_SPACE - DEL_BTN_H)
#define DEL_BTN_X (MAX_Y - 2*BLANK_SPACE - DEL_BTN_W - OK_BTN_W)

#define START_BTN_H 40
#define START_BTN_W 90
#define START_BTN_Y (MAX_X - BLANK_SPACE - START_BTN_H)
#define START_BTN_X (MAX_Y - BLANK_SPACE - START_BTN_W)

#define SP_BTN_Y 70 //show player rect
#define SP_BTN_X 20
#define SP_BTN_H 40
#define SP_BTN_W 120

#define KEY_Y 70 //key od keyboard
#define KEY_X 10
#define KEY_H 30
#define KEY_W 30

#define KEYS_IN_ONE_ROW (MAX_Y / (KEY_W + BLANK_SPACE/2)) //max_y je zapravo max_x u landscape nacinu

#define CHAR_H 8 //char height
#define CHAR_W 5 //char width
#define FONT_SIZE 3

#define PLB_HEAD_Y 25 //printleaderboards header y
#define PLB_HEAD_X 100
#define PLB_Y 80
#define PLB_X 100

#define MAX_PLAYERS 4
#define MAX_NAME_LENGTH 7 //to jest 6 + nulterm

#define GRID_STARTING_X 110
#define GRID_STARTING_Y (BCK_BTN_Y + BCK_BTN_H - BLANK_SPACE/2)
#define GRID_RECT_SIZE 185 //cijeli rect, SIZE->kao jednaki W i H
#define GRID_BLOCK_SIZE 62 //round(185/3)

#define TRY_AGAIN_Y (MAX_X - 4*BLANK_SPACE)
#define TRY_AGAIN_X 10
#define TRY_AGAIN_H 30
#define TRY_AGAIN_W 105

#define INPUT_NAME_Y 200
#define INPUT_NAME_X 10

#define NONE 5

char newPlayerName[MAX_NAME_LENGTH] = {' '}; //6+nulterm
uint8_t newPlayerNameIndex = 0;
struct Player {	
	uint8_t points;
	uint16_t color;
	uint8_t clicked;
	char name[MAX_NAME_LENGTH]; //6 + nulterm
};


uint8_t players_size = 0;
uint8_t cross_chosen = NONE, nought_chosen = NONE; //5 je kao nista nije odabrano
uint8_t hs_first_enter = 1; //home screen
uint8_t cp_rerender = 1; //choose player
uint8_t np_rerender = 1; //new player (to jest za keyboard)
uint8_t board[3][3] = {0};

#define CROSS 1
#define NOUGHT 2
#define EMPTY 0
uint8_t currentPlayer = CROSS;	
uint8_t gameOver = 0;
uint8_t numberOfMoves = 0;	
	

void print_keyboard(char str[]) {
	uint8_t offset_x, offset_y;
	
	offset_x = (KEY_W - 1 * CHAR_W * FONT_SIZE) / 2;
	offset_y = (KEY_H - 1 * CHAR_H * FONT_SIZE) / 2;
	char tmp[2];
	for(uint8_t i = 0; i < 22; i++) { //ako stavim strlen(str) onda napise jos 3 neka cudna char-a, a ko stavim 22 sve okej ??
		sprintf(tmp, "%c\0", str[i]);
		draw_rectangle(KEY_Y + i/KEYS_IN_ONE_ROW * (KEY_H + BLANK_SPACE/2), KEY_X + i%KEYS_IN_ONE_ROW * (BLANK_SPACE/2 + KEY_W), KEY_H, KEY_W, WHITE);
		print_string(KEY_Y + offset_y + i/KEYS_IN_ONE_ROW * (KEY_H + BLANK_SPACE / 2), KEY_X + i%KEYS_IN_ONE_ROW *(KEY_W + BLANK_SPACE / 2) + offset_x, 3, WHITE, CYAN, tmp);
	}
}


void drawHomeScreen() {
	uint8_t offset_x, offset_y;
	
	offset_x = (HS_W - strlen("NEW PLAYER") * CHAR_W * FONT_SIZE) / 2;
	offset_y = (HS_H - 1 * CHAR_H * FONT_SIZE) / 2;
	draw_rectangle(HS_S_Y, HS_S_X, HS_H, HS_W, WHITE);
	print_string(HS_S_Y + offset_y, HS_S_X + offset_x, 3, WHITE, CYAN, "NEW PLAYER\0");
	
	offset_x = (HS_W - strlen("CHOOSE PLAYER") * CHAR_W * FONT_SIZE) / 2;
	offset_y = (HS_H - 1 * CHAR_H * FONT_SIZE) / 2;
	draw_rectangle(CP_Y, CP_X, CP_H, CP_W, WHITE);
	print_string(CP_Y + offset_y, CP_X + offset_x, 3, WHITE, CYAN, "CHOOSE PLAYER\0");
	
	offset_x = (HS_W - strlen("LEADERBOARDS") * CHAR_W * FONT_SIZE) / 2;
	offset_y = (HS_H - 1 * CHAR_H * FONT_SIZE) / 2;
	draw_rectangle(LB_Y, LB_X, LB_H, LB_W, WHITE);
	print_string(LB_Y + offset_y, LB_X + offset_x, 3, WHITE, CYAN, "LEADERBOARDS\0");
}

void drawBackButton() {
	uint8_t offset_x, offset_y;
	
	offset_x = (BCK_BTN_W - strlen("BACK") * CHAR_W * FONT_SIZE) / 2;
	offset_y = (BCK_BTN_H - 1 * CHAR_H * FONT_SIZE) / 2;
	draw_rectangle(BCK_BTN_Y, BCK_BTN_X, BCK_BTN_H, BCK_BTN_W, WHITE);
	print_string(BCK_BTN_Y + offset_y, BCK_BTN_X + offset_x, 3, WHITE, CYAN, "BACK\0");
}

void clrScr() {
	set_background_color(CYAN);
}

void resetNewPlayerName() {
	for(uint8_t i = 0; i < strlen(newPlayerName); i++) {
		newPlayerName[i] = ' ';
	}
	newPlayerNameIndex = 0;
}

void resetPlayersColors(Player *players) {
	for(uint8_t i = 0; i < players_size; i++) {
		players[i].color = WHITE;
		players[i].clicked = 0;
	}
	cross_chosen = NONE;
	nought_chosen = NONE;
}
void resetGame() {
	memset(board, 0, sizeof(board));
	currentPlayer = CROSS; 
	gameOver = 0;
	numberOfMoves = 0;
}
void checkBackButtonPressed(uint16_t *TP_X, uint16_t *TP_Y, uint8_t *currentPage, Player *players, uint8_t *game_rerender) {
	if(get_bit(PINB, T_IRQ) == 0) {
		read_touch_coords(TP_X, TP_Y);
		if(check_touch(*TP_X, *TP_Y, BCK_BTN_Y, BCK_BTN_X, BCK_BTN_H, BCK_BTN_W)) { //BACK button
			if(*currentPage == NEW_PLAYER) { //ako je bio u NEW PLAYER da cleara
				resetNewPlayerName();
				np_rerender = 1;
			}else if(*currentPage == CHOOSE_PLAYER || *currentPage == GAME) { //ako je bio u CHOOSE PLAYER ili u GAME
				resetPlayersColors(players);
				resetGame();
				cp_rerender = 1;
				*game_rerender = 1;
			}
			*currentPage = HOMESCREEN;
		}
	}
}
void sortByPoints(Player *players) {
	for(uint8_t i = 0; i < players_size - 1; i++) {
		for(uint8_t j = 0; j < players_size - i - 1; j++) {
			if(players[j].points < players[j+1].points) {
				Player tmp = players[j];
				players[j] = players[j+1];
				players[j+1] = tmp;
			}
		}
	}
}

void printLeaderboards(Player *players, uint8_t players_size) {
	sortByPoints(players);
	print_string(PLB_HEAD_Y, PLB_HEAD_X, 3, WHITE, CYAN, "NAME\0");
	print_string(PLB_HEAD_Y, 2*PLB_HEAD_X, 3, WHITE, CYAN, "POINTS\0");
	char tmp[5];
	for(uint8_t i = 0; i < players_size; i++) {
		sprintf(tmp, "%d\0", players[i].points);
		print_string(PLB_Y + i*(4*BLANK_SPACE), PLB_X, 3, WHITE, CYAN, players[i].name);
		print_string(PLB_Y + i*(4*BLANK_SPACE), PLB_X + 15*BLANK_SPACE, 3, WHITE, CYAN, tmp);
	}
}

void showPlayers(Player *players, uint8_t players_size) {
	uint8_t tmp = 0, offset_x, offset_y;
	for(uint8_t i = 0; i < players_size; i++) {
		tmp = i > 1 ? (SP_BTN_H + BLANK_SPACE) : 0; //da ide u red ispod
		offset_x = (SP_BTN_W - strlen(players[i].name) * CHAR_W * FONT_SIZE)/2;
		offset_y = (SP_BTN_H - 1 * CHAR_H * FONT_SIZE)/2;
		
		draw_rectangle(SP_BTN_Y + tmp, SP_BTN_X + (i%2)*(SP_BTN_W + BLANK_SPACE), SP_BTN_H, SP_BTN_W, players[i].color);
		print_string(SP_BTN_Y + tmp + offset_y, SP_BTN_X + offset_x + (i%2)*(SP_BTN_W + BLANK_SPACE), 3, players[i].color, CYAN, players[i].name);
	}
}

void drawOKButton() {
	uint8_t offset_x, offset_y;
	
	offset_x = (OK_BTN_W - strlen("OK") * CHAR_W * FONT_SIZE) / 2;
	offset_y = (OK_BTN_H - 1 * CHAR_H * FONT_SIZE) / 2;
	draw_rectangle(OK_BTN_Y, OK_BTN_X, OK_BTN_H, OK_BTN_W, WHITE);
	print_string(OK_BTN_Y + offset_y, OK_BTN_X + offset_x, 3, WHITE, CYAN, "OK\0");
}

void drawDeleteButton() {
	uint8_t offset_x, offset_y;
	
	offset_x = (DEL_BTN_W - strlen("DEL") * CHAR_W * FONT_SIZE) / 2;
	offset_y = (DEL_BTN_H - 1 * CHAR_H * FONT_SIZE) / 2;
	draw_rectangle(DEL_BTN_Y, DEL_BTN_X, DEL_BTN_H, DEL_BTN_W, WHITE);
	print_string(DEL_BTN_Y + offset_y, DEL_BTN_X + offset_x, 3, WHITE, CYAN, "DEL\0");
}

void drawStartButton() {
	uint8_t offset_x = (START_BTN_W - strlen("START") * CHAR_W * FONT_SIZE) / 2;
	uint8_t offset_y = (START_BTN_H - 1 * CHAR_H * FONT_SIZE) / 2;
	draw_rectangle(START_BTN_Y, START_BTN_X, START_BTN_H, START_BTN_W, WHITE);
	print_string(START_BTN_Y + offset_y, START_BTN_X + offset_x , 3, WHITE, CYAN, "START\0");
}

void drawNames(Player *players) {
	char tmp[MAX_NAME_LENGTH+1+2+1+MAX_NAME_LENGTH]; //max first name, space, vs, space, max 2nd name
	sprintf(tmp, "%s VS %s", players[cross_chosen].name, players[nought_chosen].name);
	uint8_t offset_x = (MAX_Y - BLANK_SPACE - (BCK_BTN_X + BCK_BTN_W) - strlen(tmp) * CHAR_W * 2) / 2; //2 je FONT_SIZE
	print_string(BLANK_SPACE, (BCK_BTN_X + BCK_BTN_W) + offset_x, 2, WHITE, CYAN, tmp);
}

void drawGrid() {
	
	draw_v_line(GRID_STARTING_X + GRID_BLOCK_SIZE, GRID_STARTING_Y, GRID_STARTING_Y + GRID_RECT_SIZE, WHITE); //62~185/3
	draw_v_line(GRID_STARTING_X + 2*GRID_BLOCK_SIZE, GRID_STARTING_Y, GRID_STARTING_Y + GRID_RECT_SIZE, WHITE);
	
	draw_h_line(GRID_STARTING_Y + GRID_BLOCK_SIZE, GRID_STARTING_X, GRID_STARTING_X + GRID_RECT_SIZE, WHITE);
	draw_h_line(GRID_STARTING_Y + 2*GRID_BLOCK_SIZE, GRID_STARTING_X, GRID_STARTING_X + GRID_RECT_SIZE, WHITE);
}

void drawTurn(Player *players) {
	print_string(BCK_BTN_Y + BCK_BTN_H + 4*BLANK_SPACE, BCK_BTN_X, 3, WHITE, CYAN, "TURN:\0");
	if(currentPlayer == CROSS) {
		print_string(BCK_BTN_Y + BCK_BTN_H + 7*BLANK_SPACE, BCK_BTN_X, 2, CYAN, CYAN, players[nought_chosen].name); 
		print_string(BCK_BTN_Y + BCK_BTN_H + 7*BLANK_SPACE, BCK_BTN_X, 2, players[cross_chosen].color, CYAN, players[cross_chosen].name);
	}else{
		print_string(BCK_BTN_Y + BCK_BTN_H + 7*BLANK_SPACE, BCK_BTN_X, 2, CYAN, CYAN, players[cross_chosen].name);
		print_string(BCK_BTN_Y + BCK_BTN_H + 7*BLANK_SPACE, BCK_BTN_X, 2, players[nought_chosen].color, CYAN, players[nought_chosen].name);
	}
}

uint8_t drawOnGrid(uint8_t y, uint8_t x) {
	if(currentPlayer == CROSS) {
		my_draw_cross(y + GRID_BLOCK_SIZE/2, x + GRID_BLOCK_SIZE/2, 20, RED);
		return NOUGHT;
	}else {
		adafruit_drawCircle(y + GRID_BLOCK_SIZE/2, x + GRID_BLOCK_SIZE/2, 20, GREEN);
		return CROSS;
	}
}

void checkGameOver(Player *players) {
	uint8_t winner = 0;
	for (uint8_t i = 0; i < 3; i++) {
		if (board[i][0] != EMPTY && board[i][0] == board[i][1] && board[i][1] == board[i][2]) {
			gameOver = 1; // 3 same in a row
			winner = board[i][0];
			break;
		}
		if (board[0][i] != EMPTY && board[0][i] == board[1][i] && board[1][i] == board[2][i]) {
			gameOver = 1; // 3 same in a column
			winner = board[0][i];
			break;
		}
	}

	if (board[1][1] != EMPTY && ((board[0][0] == board[1][1] && board[1][1] == board[2][2]) || (board[0][2] == board[1][1] && board[1][1] == board[2][0]))) {
		gameOver = 1; // 3 same in a diagonal
		winner = board[1][1];
	}
	
	if(gameOver) {
		print_string(MAX_X - 10*BLANK_SPACE, BLANK_SPACE, 3, WHITE, CYAN, "WON:\0");
		winner == CROSS ? print_string(MAX_X - 7*BLANK_SPACE, BLANK_SPACE, 2, players[cross_chosen].color, CYAN, players[cross_chosen].name) : print_string(MAX_X - 7*BLANK_SPACE, BLANK_SPACE, 2, players[nought_chosen].color, CYAN, players[nought_chosen].name);
		winner == CROSS ? players[cross_chosen].points++ : players[nought_chosen].points++;
		
		uint8_t offset_x, offset_y;
		
		offset_x = (TRY_AGAIN_W - strlen("TRY AGAIN\0") * CHAR_W * 2) / 2;
		offset_y = (TRY_AGAIN_H - 1 * CHAR_H * 2) / 2;
		draw_rectangle(TRY_AGAIN_Y, TRY_AGAIN_X, TRY_AGAIN_H, TRY_AGAIN_W, WHITE);
		print_string(TRY_AGAIN_Y + offset_y, TRY_AGAIN_X + 4, 2, WHITE, CYAN, "TRY AGAIN\0"); //sa offset x bude previse desno ne kuzim zasto ??
	}else if(numberOfMoves == 9) {
		//tie
		gameOver = 1;
		print_string(MAX_X - 9*BLANK_SPACE, BLANK_SPACE, 3, WHITE, CYAN, "TIE\0");
		
		uint8_t offset_x, offset_y;
		
		offset_x = (TRY_AGAIN_W - strlen("TRY AGAIN") * CHAR_W * 2) / 2;
		offset_y = (TRY_AGAIN_H - 1 * CHAR_H * 2) / 2;
		draw_rectangle(TRY_AGAIN_Y, TRY_AGAIN_X, TRY_AGAIN_H, TRY_AGAIN_W, WHITE);
		print_string(TRY_AGAIN_Y + offset_y, TRY_AGAIN_X + 4, 2, WHITE, CYAN, "TRY AGAIN\0"); //sa offset_x bude previse desno ne kuzim zasto ??
	}
	
}

int main() {
	TFT_init();

	TFT_start();
	
	uint16_t TP_X;                  // received coordiates from touch part of screen
	uint16_t TP_Y;                  // received coordiates from touch part of screen
	set_background_color(CYAN);
	uint8_t currentPage = HOMESCREEN; //0 - homescreen //vratit na HOMESCREEN
	char str[22] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'R', 'S', 'T', 'U', 'V', 'Z'};
	uint8_t game_rerender = 1;
	Player players[MAX_PLAYERS];
	
	while (1) {
		// if screen is touched
		if(currentPage == HOMESCREEN) {
			if(hs_first_enter) {
				clrScr();
				drawHomeScreen();
				hs_first_enter = 0;
			}
			cp_rerender = 1;
			game_rerender = 1;
			if(get_bit(PINB, T_IRQ) == 0) {
				read_touch_coords(&TP_X, &TP_Y);
				if(check_touch(TP_X, TP_Y, HS_S_Y, HS_S_X, HS_H, HS_W)) { //NEW_PLAYER
					clrScr();
					hs_first_enter = 1;
					np_rerender = 1;
					currentPage = NEW_PLAYER;
				}
				if(check_touch(TP_X, TP_Y, CP_Y, CP_X, CP_H, CP_W)) { //CHOOSE_PLAYER
					clrScr();
					hs_first_enter = 1;
					currentPage = CHOOSE_PLAYER;
				}
				if(check_touch(TP_X, TP_Y, LB_Y, LB_X, LB_H, LB_W)) { //LEADERBOARDS
					clrScr(); 
					hs_first_enter = 1;
					currentPage = LEADERBOARDS;
				}
			}
		}else if(currentPage == NEW_PLAYER) {
			checkBackButtonPressed(&TP_X, &TP_Y, &currentPage, players, &game_rerender);
			if(np_rerender) {
				drawBackButton();
				print_keyboard(str);
				drawDeleteButton();
				drawOKButton();
				np_rerender = 0;
			}
				
			if(get_bit(PINB, T_IRQ) == 0) {
				read_touch_coords(&TP_X, &TP_Y);
				for(uint8_t i = 0; i < 22; i++){ //strlen(str) ali on nekad baca gresku nez zas
					if(check_touch(TP_X, TP_Y, KEY_Y + i/KEYS_IN_ONE_ROW * (KEY_H + BLANK_SPACE/2), KEY_X + i%KEYS_IN_ONE_ROW * (BLANK_SPACE/2 + KEY_W), KEY_H, KEY_W)) { //stisnut neki key od keyboarda
						if(newPlayerNameIndex == (MAX_NAME_LENGTH-1)) break;
						clrScr();
						newPlayerName[newPlayerNameIndex] = str[i];
						newPlayerName[newPlayerNameIndex+1] = '\0';
						newPlayerNameIndex++;
						print_string(INPUT_NAME_Y, INPUT_NAME_X, 3, WHITE, CYAN, newPlayerName);
						np_rerender = 1;
						break;
					}
				}
				if(check_touch(TP_X, TP_Y, DEL_BTN_Y, DEL_BTN_X, DEL_BTN_H, DEL_BTN_W)) { //delete button
					if(newPlayerNameIndex > 1) { //znaci da ima jedno slovo, npr A\0
						newPlayerNameIndex--;
						newPlayerName[newPlayerNameIndex] = '\0';
						clrScr();
						print_string(INPUT_NAME_Y, INPUT_NAME_X, 3, WHITE, CYAN, newPlayerName);
					}else if(newPlayerNameIndex == 1){ //nema ni jedno slovo, stavim razmak jer inace ako je nulterm bude zbugano
						newPlayerNameIndex--;
						newPlayerName[newPlayerNameIndex] = ' ';
						clrScr();
						print_string(INPUT_NAME_Y, INPUT_NAME_X, 3, WHITE, CYAN, newPlayerName);
					}
					np_rerender = 1;
				}
				
				if(check_touch(TP_X, TP_Y, OK_BTN_Y, OK_BTN_X, OK_BTN_H, OK_BTN_W)) { //OK button
					if(players_size == MAX_PLAYERS || !newPlayerNameIndex) continue; //ako je 4 igraca ili ako nista nije unio kao ime
					Player newPlayer;
										
					for(uint8_t i = 0; i < strlen(newPlayerName); i++) {
						newPlayer.name[i] = newPlayerName[i];
					}
					
					newPlayer.name[newPlayerNameIndex] = '\0';
					newPlayer.points = 0;
					newPlayer.clicked = 0;
					newPlayer.color = WHITE;
					players[players_size] = newPlayer;
					players_size++;
					resetNewPlayerName();
					clrScr();
					np_rerender = 1;
					currentPage = HOMESCREEN;
				}
			}
			
		}else if(currentPage == CHOOSE_PLAYER) {
			checkBackButtonPressed(&TP_X, &TP_Y, &currentPage, players, &game_rerender);
			if(cp_rerender) {
				drawBackButton();
				showPlayers(players, players_size);
				drawStartButton();
				cp_rerender = 0;	
			}
			
			if (get_bit(PINB, T_IRQ) == 0) {
				read_touch_coords(&TP_X, &TP_Y);
				uint8_t tmp;
				for(uint8_t i = 0; i < players_size; i++) {
					tmp = i > 1 ? 50 : 0;
					if(check_touch(TP_X, TP_Y, SP_BTN_Y + tmp, SP_BTN_X + (i%2)*(SP_BTN_W + BLANK_SPACE), SP_BTN_H, SP_BTN_W)) {
						if(players[i].color == WHITE && cross_chosen == NONE) {
							players[i].color = RED;
							players[i].clicked = 1;
							cross_chosen = i;
						}else if(players[i].color == WHITE && cross_chosen != 5 && nought_chosen == NONE) {
							players[i].color = GREEN;
							players[i].clicked = 1;
							nought_chosen = i;
						}else if(players[i].color == RED) {
							players[i].color = WHITE;
							cross_chosen = NONE;
							players[i].clicked = 0;
						}else if(players[i].color == GREEN) {
							players[i].color = WHITE;
							nought_chosen = NONE;
							players[i].clicked = 0;
						}
						_delay_ms(100); 
						cp_rerender = 1;
						break;
					}
				}
				
				if(check_touch(TP_X, TP_Y, START_BTN_Y, START_BTN_X, START_BTN_H, START_BTN_W)) {
					if(cross_chosen != NONE && nought_chosen != NONE){ //ne moze uc u game ako nije odabrao dvojicu
						clrScr();
						currentPage = GAME;
						game_rerender = 1; 
					}
				}
			}
		}else if(currentPage == LEADERBOARDS) {
			drawBackButton();
			checkBackButtonPressed(&TP_X, &TP_Y, &currentPage, players, &game_rerender);
			printLeaderboards(players, players_size);
		}else if(currentPage == GAME) {
			checkBackButtonPressed(&TP_X, &TP_Y, &currentPage, players, &game_rerender);
			if(gameOver && check_touch(TP_X, TP_Y, TRY_AGAIN_Y, TRY_AGAIN_X, TRY_AGAIN_H, TRY_AGAIN_W)) { //TRY AGAIN
				resetGame();
				clrScr();
				game_rerender = 1;
			}
			
			if(gameOver) continue;
			
			if(game_rerender) { 
				drawBackButton();
				drawNames(players);
				drawGrid();
				checkGameOver(players);
				drawTurn(players);
				game_rerender = 0;
			}
					
			if (get_bit(PINB, T_IRQ) == 0) {
				read_touch_coords(&TP_X, &TP_Y);
				for(uint8_t i = 0; i < 3; i++) {
					for(uint8_t j = 0; j < 3; j++) {
						if(check_touch(TP_X, TP_Y, GRID_STARTING_Y + j*GRID_BLOCK_SIZE, GRID_STARTING_X + i*GRID_BLOCK_SIZE, GRID_BLOCK_SIZE, GRID_BLOCK_SIZE)) {
							if(board[i][j] == EMPTY) { 
								board[i][j] = currentPlayer;
								numberOfMoves++;
								currentPlayer = drawOnGrid(GRID_STARTING_Y + j*GRID_BLOCK_SIZE, GRID_STARTING_X + i*GRID_BLOCK_SIZE); //y i x koordinate gornjeg lijevog kuta kvadrata na koji je stisnuo
								game_rerender = 1;
								break;
							}
						}
					}
				}
			}
		}
		
	}
}