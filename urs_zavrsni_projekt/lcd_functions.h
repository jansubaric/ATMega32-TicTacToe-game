#define F_CPU 7372800UL

#include <avr/cpufunc.h>
#include <avr/io.h>
#include <util/delay.h>

// Colors
#define LBLUE 0x963D
#define WHITE 0xFFFF
#define BLACK 0x0000
#define GREEN 0xc72b
#define RED   0xD369
#define CYAN  0x1AAE

// Screen dimensions
#define MAX_X 240
#define MAX_Y 320

// Pinout
#define LCD_DATA_H PORTA // data pins DB8-DB15
#define LCD_DATA_L PORTD // data pins DB0-DB7

#define LCD_RS    PC0 // changing between commands and data
#define LCD_WR    PC1 // write data
#define LCD_RD    PC6 // read data
#define LCD_CS    PC7 // chip select
#define LCD_RESET PB1 // lcd reset

#define T_CLK PB7 // touch controller clock
#define T_CS  PB4 // chip select
#define T_DIN PB5 // sending commands or data to touch part of screen, x and y coordinates
#define T_DO  PB6 // receiving data from touch part of screen
#define T_IRQ PB0 // interrupt, 1 if the screen is being touched
#define TOUCH_PORT PORTB

// RS definitions
#define CMD 0  // command
#define DATA 1 // data


static const unsigned char font[38][5] = { //bilo [29] bez ovih zadnjih 8 brojeva jer su 1 i 2 vec bili
	{0x7E, 0x11, 0x11, 0x11, 0x7E}, // 41 A
	{0x7F, 0x49, 0x49, 0x49, 0x36}, // 42 B
	{0x3E, 0x41, 0x41, 0x41, 0x22}, // 43 C
	{0x7F, 0x41, 0x41, 0x22, 0x1C}, // 44 D
	{0x7F, 0x49, 0x49, 0x49, 0x41}, // 45 E
	{0x7F, 0x09, 0x09, 0x09, 0x01}, // 46 F
	{0x3E, 0x41, 0x49, 0x49, 0x7A}, // 47 G
	{0x7F, 0x08, 0x08, 0x08, 0x7F}, // 48 H
	{0x00, 0x41, 0x7F, 0x41, 0x00}, // 49 I
	{0x20, 0x40, 0x41, 0x3F, 0x01}, // 4a J
	{0x7F, 0x08, 0x14, 0x22, 0x41}, // 4b K
	{0x7F, 0x40, 0x40, 0x40, 0x40}, // 4c L
	{0x7F, 0x02, 0x0C, 0x02, 0x7F}, // 4d M
	{0x7F, 0x04, 0x08, 0x10, 0x7F}, // 4e N
	{0x3E, 0x41, 0x41, 0x41, 0x3E}, // 4f O
	{0x7F, 0x09, 0x09, 0x09, 0x06}, // 50 P
	{0x3E, 0x41, 0x51, 0x21, 0x5E}, // 51 Q
	{0x7F, 0x09, 0x19, 0x29, 0x46}, // 52 R
	{0x46, 0x49, 0x49, 0x49, 0x31}, // 53 S
	{0x01, 0x01, 0x7F, 0x01, 0x01}, // 54 T
	{0x3F, 0x40, 0x40, 0x40, 0x3F}, // 55 U
	{0x1F, 0x20, 0x40, 0x20, 0x1F}, // 56 V
	{0x3F, 0x40, 0x38, 0x40, 0x3F}, // 57 W
	{0x63, 0x14, 0x08, 0x14, 0x63}, // 58 X
	{0x07, 0x08, 0x70, 0x08, 0x07}, // 59 Y
	{0x61, 0x51, 0x49, 0x45, 0x43}, // 5a Z
	{0x00, 0x00, 0x00, 0x00, 0x00}, // 20 space
	
	{0x3E, 0x51, 0x49, 0x45, 0x3E}, //0x30 0
	
	{0x00, 0x42, 0x7F, 0x40, 0x00}, // 31 1
	{0x42, 0x61, 0x51, 0x49, 0x46},  // 32 2

	{0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
	{0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
	{0x27, 0x45, 0x45, 0x45, 0x39}, // 5
	{0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
	{0x03, 0x71, 0x09, 0x05, 0x03}, // 7
	{0x36, 0x49, 0x49, 0x49, 0x36}, // 8
	{0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
	{0x00, 0x36, 0x36, 0x00, 0x00}, // :
};

uint8_t get_bit(uint8_t reg, uint8_t offset) {
	return (reg >> offset) & 1;
}

// touch part starts working
void TFT_start() {
	TOUCH_PORT |= _BV(T_CS) | _BV(T_CLK) | _BV(T_DIN);
}

// writes commands to touch
void TFT_touch_write(uint8_t num) {
	TOUCH_PORT &= ~_BV(T_CLK);
	for (uint8_t i = 0; i < 8; i++) {
		if (get_bit(num, 7 - i)) {
			TOUCH_PORT |= _BV(T_DIN);
			} else {
			TOUCH_PORT &= ~_BV(T_DIN);
		}
		TOUCH_PORT &= ~_BV(T_CLK);
		TOUCH_PORT |= _BV(T_CLK);
	}
}

// reads data from ADC on touch part of the screen (coordiates)
uint16_t TFT_touch_read() {
	uint16_t value = 0;
	for (uint8_t i = 0; i < 12; i++) {
		value <<= 1;
		TOUCH_PORT |= _BV(T_CLK);            // high signal on T_CLK
		TOUCH_PORT &= ~_BV(T_CLK);           // low signal on T_CLK, initialization of data transfer
		value += get_bit(PINB, T_DO);   // touch has 12-bit ADC, counting 0-12, taking one by one bit from T_DO,
	}

	return value;
}

// sending commands to screen
void TFT_write(uint16_t val, uint8_t rs) {
	if (rs) {                   // rs == 1 - data
		PORTC |= _BV(LCD_RS);
		} else {                    // rs == 0 - command
		PORTC &= ~_BV(LCD_RS);
	}
	PORTC &= ~_BV(LCD_CS);
	LCD_DATA_H = val >> 8;
	LCD_DATA_L = val;
	PORTC |= _BV(LCD_WR);
	PORTC &= ~_BV(LCD_WR);
	PORTC |= _BV(LCD_CS);
}

// sending specified command and value to memory
void TFT_write_pair(uint16_t cmd, uint16_t data) {
	TFT_write(cmd, CMD);
	TFT_write(data, DATA);
}

// coordinates that define where elements will be drawn
void TFT_set_address(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
	TFT_write_pair(0x0044, (x2 << 8) + x1);
	TFT_write_pair(0x0045, y1);
	TFT_write_pair(0x0046, y2);
	TFT_write_pair(0x004e, x1);
	TFT_write_pair(0x004f, y1);
	TFT_write(0x0022, CMD);
}

void TFT_init(void) {
	DDRA = 0xff ;
	DDRD = 0xff;
	DDRC |= _BV(LCD_RS) | _BV(LCD_WR) | _BV(LCD_RD) | _BV(LCD_CS); //0xff;
	DDRB = ~(_BV(T_DO) | _BV(T_IRQ)); // pins for receiving data

	// initializing lcd configuration
	DDRB |= _BV(LCD_RESET) | _BV(PB2); //pb2 je LED (tj backlight)
	PORTB |= _BV(LCD_RESET) | _BV(PB2);
	_delay_ms(5);
	PORTB &= ~_BV(LCD_RESET);
	_delay_ms(10);
	PORTB |= _BV(LCD_RESET);
	PORTC |= _BV(LCD_CS);
	PORTC |= _BV(LCD_RD);
	PORTC &= ~_BV(LCD_WR);
	_delay_ms(20);

	TFT_write_pair(0x0000, 0x0001); _delay_ms(1);
	TFT_write_pair(0x0003, 0xA8A4); _delay_ms(1);
	TFT_write_pair(0x000C, 0x0000); _delay_ms(1);
	TFT_write_pair(0x000D, 0x080C); _delay_ms(1);
	TFT_write_pair(0x000E, 0x2B00); _delay_ms(1);
	TFT_write_pair(0x001E, 0x00B0); _delay_ms(1);
	TFT_write_pair(0x0001, 0x2B3F); _delay_ms(1);
	TFT_write_pair(0x0002, 0x0600); _delay_ms(1);
	TFT_write_pair(0x0010, 0x0000); _delay_ms(1);
	TFT_write_pair(0x0011, 0x6070); _delay_ms(1);
	TFT_write_pair(0x0005, 0x0000); _delay_ms(1);
	TFT_write_pair(0x0006, 0x0000); _delay_ms(1);
	TFT_write_pair(0x0016, 0xEF1C); _delay_ms(1);
	TFT_write_pair(0x0017, 0x0003); _delay_ms(1);
	TFT_write_pair(0x0007, 0x0233); _delay_ms(1);
	TFT_write_pair(0x000B, 0x0000); _delay_ms(1);
	TFT_write_pair(0x000F, 0x0000); _delay_ms(1);
	TFT_write_pair(0x0041, 0x0000); _delay_ms(1);
	TFT_write_pair(0x0042, 0x0000); _delay_ms(1);
	TFT_write_pair(0x0048, 0x0000); _delay_ms(1);
	TFT_write_pair(0x0049, 0x013F); _delay_ms(1);
	TFT_write_pair(0x004A, 0x0000); _delay_ms(1);
	TFT_write_pair(0x004B, 0x0000); _delay_ms(1);
	TFT_write_pair(0x0044, 0xEF00); _delay_ms(1);
	TFT_write_pair(0x0045, 0x0000); _delay_ms(1);
	TFT_write_pair(0x0046, 0x013F); _delay_ms(1);
	TFT_write_pair(0x0030, 0x0707); _delay_ms(1);
	TFT_write_pair(0x0031, 0x0204); _delay_ms(1);
	TFT_write_pair(0x0032, 0x0204); _delay_ms(1);
	TFT_write_pair(0x0033, 0x0502); _delay_ms(1);
	TFT_write_pair(0x0034, 0x0507); _delay_ms(1);
	TFT_write_pair(0x0035, 0x0204); _delay_ms(1);
	TFT_write_pair(0x0036, 0x0204); _delay_ms(1);
	TFT_write_pair(0x0037, 0x0502); _delay_ms(1);
	TFT_write_pair(0x003A, 0x0302); _delay_ms(1);
	TFT_write_pair(0x003B, 0x0302); _delay_ms(1);
	TFT_write_pair(0x0023, 0x0000); _delay_ms(1);
	TFT_write_pair(0x0024, 0x0000); _delay_ms(1);

	TFT_write_pair(0x004f, 0);
	TFT_write_pair(0x004e, 0);
	TFT_write(0x0022, CMD);
}

// setting cursor to a specific position
void TFT_set_cursor(uint16_t x, uint16_t y) {
	TFT_write_pair(0x004E, x);
	TFT_write_pair(0x004F, MAX_Y - y);
	TFT_write(0x0022, CMD);
}

// reading x and y coordinates from touch part of screen
void read_touch_coords(uint16_t *TP_X, uint16_t *TP_Y) {
	_delay_ms(1);

	TOUCH_PORT &= ~_BV(T_CS);

	TFT_touch_write(0x90); // sending command to touch part of screen to write y coordinate

	_delay_ms(1);
	TOUCH_PORT |= _BV(T_CLK);  _NOP(); _NOP(); _NOP(); _NOP();
	TOUCH_PORT &= ~_BV(T_CLK); _NOP(); _NOP(); _NOP(); _NOP();
	*TP_Y = (TFT_touch_read() - 80) / 6;

	TFT_touch_write(0xD0); // sendng command to touch part of screen to write x coordinate
	TOUCH_PORT |= _BV(T_CLK);  _NOP(); _NOP(); _NOP(); _NOP();
	TOUCH_PORT &= ~_BV(T_CLK); _NOP(); _NOP(); _NOP(); _NOP();
	*TP_X = (TFT_touch_read() - 80) / 8;

	TOUCH_PORT |= _BV(T_CS);
}

// fill the screen with the specied color
void set_background_color(uint16_t color) {
	TFT_set_address(0, 0, 239, 319);

	for (uint16_t i = 0; i < 320; i++) {
		for (uint8_t j = 0; j < 240; j++) {
			TFT_write(color, DATA);
		}
	}
}

// setting a color of a pixel at the specified position
void draw_pixel(uint16_t x, uint16_t y, uint16_t color) {
	PORTC &= ~_BV(LCD_CS);
	TFT_set_cursor(x, y);
	TFT_write(color, DATA);
	PORTC |= _BV(LCD_CS);
}

// setting a color of a pixel at the specified position for a letter
void draw_font_pixel(uint16_t x, uint16_t y, uint16_t color, uint8_t pixel_size) {
	for(uint8_t i = 0; i < pixel_size; i++) {
		for(uint8_t j = 0; j < pixel_size; j++) {
			draw_pixel(x + i, y + j, color);
		}
	}
}

// setting a color to the pixels needed to write the specified character
void print_char(uint16_t x, uint16_t y, uint8_t font_size, uint16_t color, uint16_t back_color, uint8_t val) {
	for (uint8_t i = 0x00; i < 0x05; i++) {
		uint8_t value = font[val][i];
		for (uint8_t j = 0x00; j < 0x08; j++) {
			if ((value >> j) & 0x01) {
				draw_font_pixel(x + j * font_size, y, color, font_size);
				} else {
				draw_font_pixel(x + j * font_size, y, back_color, font_size);
			}
		}
		y += font_size;
	}
}

// setting a color to the pixels needed to write the specified string
void print_string(uint16_t x, uint16_t y, uint8_t font_size, uint16_t color, uint16_t back_color, const char *ch) {
	uint8_t cnt = 0;

	do {
		if (ch[cnt] == ' ') {
			print_char(x + font_size, y, font_size, color, back_color, 26);
		} else if (ch[cnt] == '0') {
			print_char(x + font_size, y, font_size, color, back_color, 27);
		} else if (ch[cnt] == '1') {
			print_char(x + font_size, y, font_size, color, back_color, 28);
		} else if (ch[cnt] == '2') {
			print_char(x + font_size, y, font_size, color, back_color, 29);
		} else if (ch[cnt] == '3') {
			print_char(x + font_size, y, font_size, color, back_color, 30);
		} else if (ch[cnt] == '4') {
			print_char(x + font_size, y, font_size, color, back_color, 31);
		} else if (ch[cnt] == '5') {
			print_char(x + font_size, y, font_size, color, back_color, 32);
		} else if (ch[cnt] == '6') {
			print_char(x + font_size, y, font_size, color, back_color, 33);
		} else if (ch[cnt] == '7') {
			print_char(x + font_size, y, font_size, color, back_color, 34);
		} else if (ch[cnt] == '8') {
			print_char(x + font_size, y, font_size, color, back_color, 35);
		} else if (ch[cnt] == '9') {
			print_char(x + font_size, y, font_size, color, back_color, 36);
		} else if (ch[cnt] == ':') {
			print_char(x + font_size, y, font_size, color, back_color, 37);
		} else {
			print_char(x + font_size, y, font_size, color, back_color, ch[cnt] - 'A');
		}
		cnt++;
		y += 0x05 * font_size + 0x01;
	} while(ch[cnt] != '\0');
}

// setting a color to the pixels in a horizontal line
void draw_h_line(uint16_t x1, uint16_t y1, uint16_t y2, uint16_t color) { //zapravo y1, x1, x2
	for (; y1 < y2; y1++) {
		draw_pixel(x1, y1, color);
	}
}

// setting a color to the pixels in a vetical line
void draw_v_line(uint16_t y1, uint16_t x1, uint16_t x2, uint16_t color) { //zapravo x1, y1, y2
	for (; x1 < x2; x1++) {
		draw_pixel(x1, y1, color);
	}
}

// setting a color to the pixels in two diagonal lines
void draw_cross(uint16_t x, uint16_t y, uint16_t d, uint16_t color) {
	for (uint8_t i = 0; i < d; i++) {
		draw_pixel(x + i, y + i, color);
		draw_pixel(x + i, d - i + y, color);
	}
}

// setting a color to the pixels in circle
void draw_circle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color) {
	x0 += r;
	y0 += r;

	int16_t x = -r, y = 0, err = 2 - 2 * r, e2;
	do {
		draw_pixel(x0 - x, y0 + y, color);
		draw_pixel(x0 + x, y0 + y, color);
		draw_pixel(x0 + x, y0 - y, color);
		draw_pixel(x0 - x, y0 - y, color);

		e2 = err;
		if (e2 <= y) {
			err += ++y * 2 + 1;
			if (-x == y && e2 <= x)
			e2 = 0;
		}
		if (e2 > x) {
			err += ++x * 2 + 1;
		}
	} while (x <= 0);
}

// setting a color to the pixels of a rectangle
void draw_rectangle(uint16_t x, uint16_t y, uint16_t dx, uint16_t dy, uint16_t color) {
	draw_h_line(x, y, y + dy, color);
	draw_h_line(x + dx, y, y + dy, color);
	draw_v_line(y, x, x + dx, color);
	draw_v_line(y + dy, x, x + dx, color);
}

// check if the screen is being touched
uint8_t check_touch(uint16_t TP_X, uint16_t TP_Y, uint16_t x, uint16_t y, uint16_t dx, uint16_t dy) {
	return TP_Y >= y && TP_Y <= y + dy && TP_X >= x && TP_X <= x + dx;
}