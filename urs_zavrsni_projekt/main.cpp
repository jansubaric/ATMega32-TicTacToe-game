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

#define CHAR_H 8 //char height
#define CHAR_W 5 //char width
#define FONT_SIZE 3

#define PLB_HEAD_Y 25 //printleaderboards header y
#define PLB_HEAD_X 100
#define PLB_Y 80
#define PLB_X 100

char newPlayerName[7] = {' '}; //6+nulterm
//newPlayerName[6] = '\0';
uint8_t newPlayerNameIndex = 0;
struct Player {
	//Player() {
	//points = 0;
	//}
	
	uint8_t points;
	uint16_t color;
	uint8_t clicked;
	char name[7]; //6 + nulterm
};
Player players[4];
uint8_t players_size = 4; //stavi u 0 poslije
//uint8_t players_size = 0; //stavi u 0 poslije
uint8_t cross_chosen = 5, nought_chosen = 5; //5 je kao nista nije odabrano, ne mozes -1 jer ne unsigned int
uint8_t hs_first_enter = 0; //home screen


void print_keyboard(char str[]) {
	uint8_t offset_x, offset_y;
	//char *str = "ABCDEFGHIJKLMNJOPRSTUVZ";
	//char str[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'R', 'S', 'T', 'U', 'V', 'Z'};
	offset_x = (KEY_W - 1 * CHAR_W * FONT_SIZE) / 2;
	offset_y = (KEY_H - 1 * CHAR_H * FONT_SIZE) / 2;
	char tmp[2];
	for(uint8_t i = 0; i < strlen(str); i++) { 
		sprintf(tmp, "%c\0", str[i]);
		draw_rectangle(KEY_Y + i/9 * (KEY_H + BLANK_SPACE/2), KEY_X + i%9*(BLANK_SPACE/2 + KEY_W), KEY_H, KEY_W, WHITE);
		print_string(KEY_Y + offset_y + i/9 * (KEY_H + BLANK_SPACE / 2), KEY_X + i%9*(KEY_W + BLANK_SPACE / 2) + offset_x, 3, WHITE, CYAN, tmp);
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

void resetPlayersColors() { //andClicks,, mozda bolje nazvat funkciju
	for(uint8_t i = 0; i < players_size; i++) {
		players[i].color = WHITE;
		players[i].clicked = 0;
	}
	cross_chosen = 5;
	nought_chosen = 5;
}
void checkBackButtonPressed(uint16_t *TP_X, uint16_t *TP_Y, uint8_t *currentPage) {
	if(get_bit(PINB, T_IRQ) == 0) {
		read_touch_coords(TP_X, TP_Y);
		if(check_touch(*TP_X, *TP_Y, BCK_BTN_Y, BCK_BTN_X, BCK_BTN_H, BCK_BTN_W)) { //BACK button, ovo bolje napravit preko define-a --jesam
			//clrScr();
			//_delay_ms(10);
			if(*currentPage == NEW_PLAYER) { //ako je bio u NEW PLAYER da cleara
				resetNewPlayerName();
			}else if(*currentPage == CHOOSE_PLAYER || *currentPage == GAME) { //ako je bio u CHOOSE PLAYER ili u GAME
				resetPlayersColors();
			}
			*currentPage = HOMESCREEN;
			//clrScr();
		}
	}
}
void sortByPoints() {
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

void printLeaderboards(Player *players, uint8_t players_size) { //treba da sortira silazno po points i ovo da ljepse ispisuje
	sortByPoints();
	print_string(PLB_HEAD_Y, PLB_HEAD_X, 3, WHITE, CYAN, "NAME\0");
	print_string(PLB_HEAD_Y, 2*PLB_HEAD_X, 3, WHITE, CYAN, "POINTS\0");
	char tmp[2];
	tmp[1] = '\0';
	for(uint8_t i = 0; i < players_size; i++) {
		tmp[0] = players[i].points + '0';
		print_string(PLB_Y + i*(4*BLANK_SPACE), PLB_X, 3, WHITE, CYAN, players[i].name);
		print_string(PLB_Y + i*(4*BLANK_SPACE), PLB_X + 15*BLANK_SPACE, 3, WHITE, CYAN, tmp);
	}
}

void showPlayers(Player *players, uint8_t players_size) { //dodat da se imena ispisu u sredini i kad kliknes na neko ime ako je bijelo neka postane crveno ako opet kliknes na isto da postane opet bijelo, kad kliknes na neko drugo (a jedno je vec crveno) to drugo postane zeleno itd..--jesam
	uint8_t tmp = 0, offset_x, offset_y;
	for(uint8_t i = 0; i < players_size; i++) {
		tmp = i > 1 ? (SP_BTN_H + BLANK_SPACE) : 0; //da ide u red ispod
		offset_x = (SP_BTN_W - strlen(players[i].name) * CHAR_W * FONT_SIZE)/2;
		offset_y = (SP_BTN_H - 1 * CHAR_H * FONT_SIZE)/2;
		
		draw_rectangle(SP_BTN_Y + tmp, SP_BTN_X + (i%2)*(SP_BTN_W + BLANK_SPACE), SP_BTN_H, SP_BTN_W, players[i].color); //i/2 jer ide i+=2
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

int main() {
	TFT_init();

	//initialize_menu();

	TFT_start();
	
	//uint8_t board[3][3];            // grid
	uint16_t TP_X;                  // received coordiates rom tuch part of screen
	uint16_t TP_Y;                  // received coordiates rom tuch part of screen
	set_background_color(CYAN); //kao neki clearscr
	//uint8_t i = 0;
	uint8_t currentPage = HOMESCREEN; //0 - homescreen //vratit na HOMESCREEN
	char str[22] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'R', 'S', 'T', 'U', 'V', 'Z'};
	//print_keyboard(str);
	//Player players[4];
	
	Player p1; //ovo dinamicki radi
	p1.points = 0;
	for(uint8_t j = 0; j < 7; j++) {
		p1.name[j] = 'A';
	}
	p1.name[6] = '\0';
	p1.clicked = 0;
	p1.color = WHITE;
	
	Player p2;
	p2.points = 2;
	for(uint8_t j = 0; j < 7; j++) {
		p2.name[j] = 'B';
	}
	p2.name[6] = '\0';
	p2.clicked = 1;
	p2.color = WHITE;
	
	Player p3;
	p3.points = 7;
	for(uint8_t j = 0; j < 7; j++) {
		p3.name[j] = 'C';
	}
	p3.name[6] = '\0';
	p3.clicked = 0;
	p3.color = WHITE;
	
	Player p4;
	p4.points = 4;
	for(uint8_t j = 0; j < 7; j++) {
		p4.name[j] = 'D';
	}
	p4.name[6] = '\0';
	p4.clicked = 0;
	p4.color = WHITE;
	
	players[0] = p1;
	players[1] = p2;
	players[2] = p3;
	players[3] = p4;
	
	while (1) {
		// if screen is touched
		if(currentPage == HOMESCREEN) {
			//clrScr();
			if(hs_first_enter) { //bez ovoga ostanu stvari od proslog page-a nez zas iako sam gore u back buttonu stavio clrScr()
				clrScr();
				hs_first_enter = 0;
			}
			drawHomeScreen();
			if(get_bit(PINB, T_IRQ) == 0) {
				read_touch_coords(&TP_X, &TP_Y);
				if(check_touch(TP_X, TP_Y, HS_S_Y, HS_S_X, HS_H, HS_W)) { //NEW_PLAYER
					clrScr();
					hs_first_enter = 1;
					currentPage = NEW_PLAYER;
				}
				if(check_touch(TP_X, TP_Y, CP_Y, CP_X, CP_H, CP_W)) { //CHOOSE_PLAYER
					clrScr();
					hs_first_enter = 1;
					currentPage = CHOOSE_PLAYER;
				}
				if(check_touch(TP_X, TP_Y, LB_Y, LB_X, LB_H, LB_W)) { //LEADERBOARDS
					clrScr(); //tu da se clear-a da se stalno ne re-cleara
					hs_first_enter = 1;
					currentPage = LEADERBOARDS;
				}
			}
		}else if(currentPage == NEW_PLAYER) {
			drawBackButton();
			print_keyboard(str);
			checkBackButtonPressed(&TP_X, &TP_Y, &currentPage);
			drawDeleteButton();
			drawOKButton();
				
			if(get_bit(PINB, T_IRQ) == 0) {
				read_touch_coords(&TP_X, &TP_Y);
				for(uint8_t i = 0; i < strlen(str); i++){
					if(check_touch(TP_X, TP_Y, KEY_Y + i/9 * (KEY_H + BLANK_SPACE/2), KEY_X + i%9*(BLANK_SPACE/2 + KEY_W), KEY_H, KEY_W)) { //stisnut neki key od keyboarda
						if(newPlayerNameIndex == 6) break; //jer ime ima max 7 char-a
						clrScr();
						newPlayerName[newPlayerNameIndex] = str[i];
						newPlayerName[newPlayerNameIndex+1] = '\0';
						newPlayerNameIndex++;
						print_string(200, 10, 3, WHITE, CYAN, newPlayerName);
						//_delay_ms(500); //da ne napravi previse ocitanja
						break;
					}
				}
				if(check_touch(TP_X, TP_Y, DEL_BTN_Y, DEL_BTN_X, DEL_BTN_H, DEL_BTN_W)) {
					if(newPlayerNameIndex > 1) { //znaci da ima jedno slovo, npr A\0
						newPlayerNameIndex--;
						newPlayerName[newPlayerNameIndex] = '\0';
						clrScr();
						print_string(200, 10, 3, WHITE, CYAN, newPlayerName);
					}else if(newPlayerNameIndex == 1){ //nema ni jedno slovo, stavim razmak jer inace ako je nulterm bude zbugano
						newPlayerNameIndex--;
						newPlayerName[newPlayerNameIndex] = ' ';
						clrScr();
						print_string(200, 10, 3, WHITE, CYAN, newPlayerName);
					}
					
				}
				
				if(check_touch(TP_X, TP_Y, OK_BTN_Y, OK_BTN_X, OK_BTN_H, OK_BTN_W)) { //OK button
					//create player i da se vrati na homescreen
					if(players_size == 4 || !newPlayerNameIndex) continue; //ako je 4 igraca ili ako nista nije unio kao ime
					Player newPlayer;
					//newPlayer.name = newPlayerName;
					for(uint8_t i = 0; i < strlen(newPlayerName); i++) { //morat ces reset-at newPlayerName i to --jesam
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
					currentPage = HOMESCREEN;
				}
			}
			
		}else if(currentPage == CHOOSE_PLAYER) {
			drawBackButton();
			checkBackButtonPressed(&TP_X, &TP_Y, &currentPage);
			showPlayers(players, players_size);
			drawStartButton();
			
			if (get_bit(PINB, T_IRQ) == 0) {
				read_touch_coords(&TP_X, &TP_Y);
				uint8_t tmp;
				for(uint8_t i = 0; i < players_size; i++) {
					tmp = i > 1 ? 50 : 0;
					if(check_touch(TP_X, TP_Y, SP_BTN_Y + tmp, SP_BTN_X + (i%2)*(SP_BTN_W + BLANK_SPACE), SP_BTN_H, SP_BTN_W)) { //skuzi zasto poboja cijeli stupac ako gornjeg stisnes, a nista ako donjeg --jer ti je tmp bio izvan petlje
						if(players[i].color == WHITE && cross_chosen == 5) {
							players[i].color = RED;
							players[i].clicked = 1;
							cross_chosen = i;
						}else if(players[i].color == WHITE && cross_chosen != 5 && nought_chosen == 5) {
							players[i].color = GREEN;
							players[i].clicked = 1;
							nought_chosen = i;
						}else if(players[i].color == RED) {
							players[i].color = WHITE;
							cross_chosen = 5;
							players[i].clicked = 0;
						}else if(players[i].color == GREEN) {
							players[i].color = WHITE;
							nought_chosen = 5;
							players[i].clicked = 0;
						}
						_delay_ms(100); //da ne napravi 2 ocitanja
						break;
					}
				}
				
				if(check_touch(TP_X, TP_Y, START_BTN_Y, START_BTN_X, START_BTN_H, START_BTN_W)) {
					if(cross_chosen != 5 && nought_chosen != 5){ //ne moze uc u game ako nije odabrao dvojicu
						clrScr();
						currentPage = GAME;
					}
				}
			}
		}else if(currentPage == LEADERBOARDS) {
			drawBackButton();
			checkBackButtonPressed(&TP_X, &TP_Y, &currentPage);
			//printLeaderboards(players, sizeof(players) / sizeof(players[0])); //moras prije nego saljes u funkciju jer se u funkciju salje samo pointer, ali ovo ce ti poslat za koliko njih je alocirano mjesta, a ne koliko ih je stvarno unutra
			printLeaderboards(players, players_size);
		}else if(currentPage == GAME) {
			//dodat kod za igru
			drawBackButton();
			checkBackButtonPressed(&TP_X, &TP_Y, &currentPage);
			print_string(200, 200, 3, WHITE, CYAN, "BOK\0"); //maknut
		}
		
	}
}