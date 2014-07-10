//CPU CLOCK
# define F_CPU 16000000UL

// include files
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include "LED_Display.c"
#include <timer.h>
#include "io.h"
#include "io.c"


//definitions
#define N 64


// NOTES
#define C3 0x01
#define B3 0x02
#define A3 0x04
#define G4 0x08
#define F4 0x10
#define E4 0x20
#define D4 0x40
#define C4 0x80

// Global State Control Variables

unsigned char _fft_ready;
unsigned char _measurement_cnt;
unsigned char _notes_heard;
unsigned char _game_start;
unsigned char _song_done;
unsigned char _score_on;
unsigned short *_high_score;
unsigned char _lives;
unsigned char _FBB;
unsigned char _input_enable;


//score
unsigned short _score[5];
unsigned short _score_total;

// controls related to song stuff
unsigned char _Number_of_Songs;
uint16_t _SngNum;

//  FFT variables and structures

signed short input_temp[64];

typedef struct complex_array
{
	signed short real[64];
	signed short img[64];
	
} complex_array;

typedef struct complex
{
	signed short real;
	signed short img;
	
} complex;

const signed short Sin[] = {0, 13, 25, 37, 49, 60, 71, 81, 91, 99, 106, 113, 118, 122, 126, 127, 128, 127, 126, 122, 118, 113, 106, 99, 91, 81, 71, 60, 49, 37, 25, 13};
const signed short Cos[] = {128, 127, 126, 122, 118, 113, 106, 99, 91, 81, 71, 60, 49, 37, 25, 13, 0, -13, -25, -37, -49, -60, -71, -81, -91, -99, -106, -113, -118, -122, -126, -127};

const unsigned char Twidle_address[64] = {0, 32, 16, 48, 8, 40, 24, 56, 4, 36, 20, 52, 12, 44, 28, 60, 2, 34, 18, 50, 10, 42, 26, 58, 6, 38, 22, 54, 14, 46, 30, 62, 1, 33, 17, 49, 9, 41, 25, 57, 5, 37, 21, 53, 13, 45, 29, 61, 3, 35, 19, 51, 11, 43, 27, 59, 7, 39, 23, 55, 15, 47, 31, 63};


unsigned char _output[64];
complex_array _input;

//LED controls
unsigned char _LED_on;
unsigned char _blue[8];
unsigned char _green[8];
unsigned char _red[8];

// tasks

typedef struct task {
	int state;
	unsigned long period;
	unsigned long elapsedTime;
	int (*TickFct)(int);
} task;

// song file structure

typedef struct Song_file
{
	const unsigned char *name;
	const unsigned char *notes;
	unsigned short length;
	unsigned char time_sig;
	
} Song_file;


//song_file setup
Song_file Song_of_time, Keyboard_cat, C_scale;

Song_file *_Songs[] = {&Song_of_time, &Keyboard_cat, &C_scale};

//Song_file Song of time
const unsigned char Song_of_time_name[] = "Song of Time";
const unsigned char Song_of_time_notes[] = {0, 0, 0, 0, 0, 0, A3, A3, D4, D4, D4, D4, F4, F4, A3, A3, D4, D4, D4, D4, F4, F4, A3, C3, B3, B3, G4, G4, F4, G4, A3, A3, D4, D4, C4, E4, D4, D4, D4, D4, D4, D4, D4, D4, D4, D4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

//Song_file Keyboard Cat

const unsigned char Key_board_cat_name[] = "Keyboard Cat";
const unsigned char Key_board_cat_notes[] = {0, 0, 0, 0, 0, 0, F4, F4, A3, A3, C3, C3, A3, A3, F4, F4, A3, C3, C3, C3, A3, A3, D4, D4, F4, F4, A3, A3, F4, F4, D4, D4, F4, A3, A3, A3, F4, F4, C4, C4, E4, E4, G4, G4, E4, E4, C4, C4, E4, G4, G4, G4, E4, E4, C4, C4, 0, C4, C4, 0, C4, C4, C4, 0, C4, 0, C4, 0,C4, 0,  F4, F4, A3, A3, C3, C3, A3, A3, F4, F4, A3, C3, C3, C3, A3, A3, D4, D4, F4, F4, A3, A3, F4, F4, D4, D4, F4, A3, A3, A3, F4, F4, C4, C4, E4, E4, G4, G4, E4, E4, C4, C4, E4, G4, G4, G4, E4, E4, C4, C4, C4, C4, C4, C4, C4, C4, C4, C4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// C scale
const unsigned char C_scale_name[] = "C Scale";
const unsigned char C_scale_notes[] = {0, 0, 0, 0, 0, 0, 0, C4, C4, D4, D4, E4, E4, F4, F4, G4, G4, A3, A3, B3, B3, C3, C3, 0, 0, C3, C3, B3, B3,	A3, A3, G4, G4, F4, F4, E4, E4, D4, D4, C4, C4, C4, C4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};



// FIRE BREATHING BRISK FRAMES
const unsigned char blue1[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x50,  0x88};
const unsigned char red1[8] = {0x00, 0x70, 0x70, 0x20, 0x00, 0x88, 0x00, 0x00};
const unsigned char green1[8] = {0x00, 0x00, 0x00, 0x00, 0x70, 0x20, 0x00, 0x00};


const unsigned char red2[8] = {0x00, 0x70, 0x78, 0x20, 0x00, 0x88, 0x00, 0x00};
const unsigned char red3[8] = {0x00, 0x74, 0x7C, 0x24, 0x00, 0x88, 0x00, 0x00};
const unsigned char red4[8] = {0x00, 0x76, 0x7E, 0x26, 0x00, 0x88, 0x00, 0x00};
const unsigned char red5[8] = {0x00, 0x76, 0x7F, 0x26, 0x00, 0x88, 0x00, 0x00};

// Hardware setup code
void interrupt_enable() {
	SREG |= 0x80;
}

void interrupt_disable() {
	SREG &= 0x00;
}

void ADC_timer_setup() {
	TCCR1A = (1 << COM1B1);
	TCCR1B |= (1<<WGM12) | (1 << CS11); // prescaler 1/8.  so 2,000,000 ticks a second.
	OCR1A = 1786; // want a tick every 893 microseconds or every 1653 ticks
	TCNT1 = 0;
	TIMSK1 = 0x04;
	
}


// ADC is set up to run when above timer reaches 1786
void ADC_init() { 
	ADCSRA |= (1 << ADEN) | (1 << ADIE) | (1 << ADATE) | (1<<ADPS1) | (1<<ADPS2);
	ADMUX |= (1 << MUX1);
	ADCSRB |= (1 << ADTS0) | (1 << ADTS2);
}

void ADC_start() {
	ADCSRA |= (1 << ADSC);
}

void ADC_STOP() {
	ADCSRA &= 0b01111111;
}

void ADC_Complete() {
	
	// stores values for FFT use
	if ((_measurement_cnt < N)) {
		input_temp[_measurement_cnt] = (ADC >> 4);
		_measurement_cnt++;
	}
	else if ((_measurement_cnt >= N )) {
		_measurement_cnt = 0;
		for (unsigned char i = 0; i<N; i++)
		{
			_input.real[i] = input_temp[i];
			_input.img[i] = 0;
		}
		if (_fft_ready == 0) {
			_fft_ready = 1;
		}

	}
	
}



// functions
//------Find GCD Function

unsigned short findGCD(unsigned short a, unsigned short b)
{
	unsigned short c;
	while(1) {
		c = a%b;
		if(c==0){return b;}
		a = b;
		b = c;
	}
	return 0;
}

// State Machines

//FFT broken up into states.  Without doing this, the fft takes too long on 64 points and causes my display to flicker/program to be somewhat unresponsive.

enum FFT_states {fft_wait, fft_copy, fft_twidle, fft_radix2, fft_norm, fft_mag};
	
signed int FFT(signed int state) {
	
	static unsigned char index1;
	static unsigned char index2;
	static unsigned char index3;
	static unsigned char p;
	
	static complex temp1;
	static complex temp2;
	
	static complex_array bit_twidle_temp;
	
	switch(state) {
		case -1:
		state = fft_wait;
		break;
		
		case fft_wait:
		if (_fft_ready) {
			state = fft_copy;
		}
		else if (!_fft_ready) {
			state = fft_wait;
		}
		break;
		
		case fft_copy:
		state = fft_twidle;
		break;
		
		case fft_twidle:
		p = 1;
		state = fft_radix2;
		break;
		
		case fft_radix2:
		if (p>6) {
			state = fft_norm;
		}
		else if (p <= 6) {
			state = fft_radix2;
			
		}
		break;
		
		case fft_norm:
		state = fft_mag;
		break;
		
		case fft_mag:
		state = fft_wait;
		break;
		
		default:
		state = fft_wait;
		break;
		
	}
	
	switch(state) {
		
		case -1:
		break;
		
		case fft_wait:
		break;
		
		case fft_copy:
		
		for (unsigned char cpy = 0; cpy < 64; cpy++)
		{
			bit_twidle_temp.real[cpy] = _input.real[cpy];
			bit_twidle_temp.img[cpy] = _input.img[cpy];
		}
		break;
		
		case fft_twidle:
		
		for (unsigned char address = 0; address < 64; address++)
		{
			unsigned char tempadd = Twidle_address[address];
			_input.real[address] = bit_twidle_temp.real[tempadd];
			_input.img[address] = bit_twidle_temp.img[tempadd];
		}
		
		p = 1;
		break;
		
		case fft_radix2:
		
		for (unsigned char L = 0; L < (1<<(6-p)); L++) {
			for (unsigned char R = 0; R < (1<<(p-1)); R++) {
				
				index1 = L*(1 << p)+R;
				index2 = L*(1 << p)+(1 << (p-1))+R;
				index3 = R*(1 << (6-p));
				
				temp1.real = _input.real[index1];
				temp1.img = _input.img[index1];
				temp2.real = _input.real[index2];
				temp2.img = _input.img[index2];
				
				_input.real[index1] = (((temp1.real << 7) + (temp2.real*Cos[index3]) - (temp2.img*Sin[index3])) >> 7);
				_input.img[index1] = (((temp1.img << 7) + (temp2.real*Sin[index3]) + (temp2.img*Cos[index3])) >> 7);
				
				_input.real[index2] = (((temp1.real << 7) - (temp2.real*Cos[index3]) + (temp2.img*Sin[index3])) >> 7 );
				_input.img[index2] = (((temp1.img << 7) - (temp2.real*Sin[index3]) - (temp2.img*Cos[index3])) >> 7);
			}
		}
		
		p++;
		break;
		
		case fft_norm:
		
		for (unsigned char j = 0; j<32; j++)  //normalize values
		{
			_input.real[j] = (_input.real[j]);
			_input.img[j] = (_input.img[j]);
		}
		
		break;
		
		case fft_mag:
		
		for (unsigned char mag = 0; mag<64; mag++)
		{
			_output[mag] = (_input.real[mag]*_input.real[mag] + _input.img[mag]*_input.img[mag]) >> 2;
		}
		
		_fft_ready = 0;
		break;
		
		default:
		break;
	}
		
	return state;

}

// LED Controller SM

enum LED_controller_states {LED_off, disp_screen};
	
signed int LED_controller(signed int state) {
	
	static unsigned char LED_cycle;
	
	switch(state) {
		case -1:
		state = LED_off;
		break;
		
		case LED_off:
		if (_LED_on == 0) {
			state = LED_off;
		}
		else if (_LED_on == 1) {
			state = disp_screen;
		}
		break;
		
		
		case disp_screen:
		if (_LED_on == 0) {
			state = LED_off;
		}
		else if (_LED_on == 1) {
			state = disp_screen;
		}
		break;
		
		default:
		state = LED_off;
		break;
	}
	
	switch(state) {
		case -1:
		break;
		
		case LED_off:
		LED_cycle = 0;
		update_blue( 0x00, 0);  // blanks out led
		break;
		
		case disp_screen:
		update_display(_green[LED_cycle], _blue[LED_cycle], _red[LED_cycle], LED_cycle);
		if (LED_cycle < 7) {
			LED_cycle++;
		}
		else if (LED_cycle >= 7) {
			LED_cycle = 0;
		}
		break;
		
		default:
		break;
		
	}
	return state;
}

// Check which notes are currently being played

enum notes_heard_states {Ini_check, check};


// check which notes are currently being heard/played
signed int check_notes(signed int state){
	
	switch(state) {
		case -1:
		state = Ini_check;
		break;
		
		case Ini_check:
		if (_input_enable) {
			state = check;
		}
		else if (!_input_enable) {
			state = Ini_check;
		}
		break;
		
		case check:
		if (_input_enable) {
			state = check;
		}
		else if (!_input_enable) {
			state = Ini_check;
		}
		break;
		
		default:
		state = Ini_check;
		break;
	}
	
	switch(state) {
		case -1:
		break;
		
		case Ini_check:
		break;
		
		case check:
		
		_notes_heard = 0x00;
		
		if (_output[15] > 18) {
			_notes_heard |= C4;
		}
		if (_output[17] > 24) {
			_notes_heard |= D4;
		}
		if (_output[19] > 18) {
			_notes_heard |= E4;
		}
		if (_output[20] > 18) {
			_notes_heard |= F4;
		}
		if (_output[22] > 18) {
			_notes_heard |= G4;
		}
		if (_output[25] > 18) {
			_notes_heard |= A3;
		}
		if (_output[28] > 18) {
			_notes_heard |= B3;
		}
		if (_output[30] > 18) {
			_notes_heard |= C3;
		}
		break;
		
		default:
		break;
	}
	return state;
}

// Gameplay controller

enum Gameplay_states {game_ini, update_LED, score_check, end_song_pass, end_song_fail, end_song, end_song_wait, done_wait};
	
signed int Gameplay(signed int state) {
	
	static unsigned char song_timer;
	static unsigned char correct;
	static unsigned char score_check_timer;
	static unsigned char end_song_count;
	
	switch(state) {
		case -1:
		state = game_ini;
		break;
		
		case game_ini:
		if (_game_start == 1) {
			state = update_LED;
		}
		else if (_game_start == 0) {
			state = game_ini;
		}
		break;
		
		case update_LED:
		if ((song_timer < _Songs[_SngNum]->length)&&(_lives)) {
			state = score_check;
		}
		else if ((song_timer >= _Songs[_SngNum]->length)&&(_lives)) {
			state = end_song_pass;
		}
		else if (_lives == 0) {
			state = end_song_fail;
		}
		break;
		
		case score_check:
		if (score_check_timer < _Songs[_SngNum]->time_sig) {
			state = score_check;
		}
		else if (score_check_timer >= _Songs[_SngNum]->time_sig) {
			state = update_LED;
		}
		break;
		
		case end_song_pass:
		state = end_song;
		break;
		
		case end_song_fail:
		state = end_song;
		break;
		
		case end_song:
		if (_notes_heard & C3) {
			state = end_song_wait;
		}
		else if (!(_notes_heard & C3)) {
			state = end_song;
		}
		break;
		
		case end_song_wait:
		if (end_song_count < 10) {
			state = end_song_wait;
		}
		else if (end_song_count >= 10) {
			state = done_wait;
		}
		break;
		
		case done_wait:
		if (_game_start) {
			state = done_wait;
		}
		else if (!(_game_start)) {
			state = game_ini;
		}
		break;
		
		
		default:
		state = game_ini;
		break;
	}
	
	switch(state) {
		case -1:
		break;
		
		case game_ini:
		_score_total = 0;
		_song_done = 0;
		song_timer = 0;
		score_check_timer = 0;
		_lives = 9;
		correct = 1;
		break;
		
		case update_LED:
		if (correct == 1) {
			_green[7] = _blue[6];
			_red[7] = 0x00;
		}
		else if (correct == 0)  {
			_green[7] = 0x00;
			_red[7] = _blue[6];
		}
		for (unsigned char i = 6; i > 0; i--) {
			_blue[i] = _blue[i-1];
		}
		_blue[0] = _Songs[_SngNum]->notes[song_timer];
		_green[6] = 0xFF;
		
		if (!correct) {
			_lives--;
		}
		else if (correct&&(_lives<9)) {
			_lives++;
		}
		correct = 0;
		song_timer++; 
		score_check_timer = 0;
		break;
		
		case score_check:
		if ((_notes_heard == _blue[6])) {
			correct = 1;
			if (_blue[6]) {
				_score_total += _lives;
				}
				
		}
		score_check_timer++;
		break;
		
		case end_song_pass:
		_score_on = 0;
		song_timer = 0;
		for (unsigned char i = 0; i<8; i++){
			_blue[i] = 0;
			_green[i] = 0;
			_red[i] = 0;
		}
		LCD_ClearScreen();
		
		
		_score[0] = _score_total/10000;
		_score[1] = (_score_total/1000) % 10;
		_score[2] = (_score_total/100) % 10;
		_score[3] = (_score_total/10) % 10;
		_score[4] = (_score_total)%10;
		
		
		if (_score_total > _high_score[_SngNum]) {
			
			LCD_DisplayString(1, "HIGHSCORE!");
			LCD_WriteData(_score[0] + '0');
			LCD_WriteData(_score[1] + '0');
			LCD_WriteData(_score[2] + '0');
			LCD_WriteData(_score[3] + '0');
			LCD_WriteData(_score[4] + '0');
			
			interrupt_disable();
			eeprom_write_word((uint16_t*)(_SngNum*2), _score_total);
			interrupt_enable();
			_high_score[_SngNum] = _score_total;
		}
		else {
			LCD_DisplayString(1, "score:");
			LCD_WriteData(_score[0] + '0');
			LCD_WriteData(_score[1] + '0');
			LCD_WriteData(_score[2] + '0');
			LCD_WriteData(_score[3] + '0');
			LCD_WriteData(_score[4] + '0');
		}
		
		LCD_DisplayString(17, "C3 to continue");
		break;
		
		case end_song_fail:
		_score_on = 0;
		song_timer = 0;
		for (unsigned char i = 0; i<8; i++){
			_blue[i] = 0;
			_green[i] = 0;
			_red[i] = 0;
		}
		LCD_ClearScreen();
		LCD_DisplayString(1, "You Are BAD");
		LCD_DisplayString(17, "C3 to Continue");
		break;
		
		case end_song:
		end_song_count = 0;
		break;
		
		case end_song_wait:
		if (_notes_heard & C3) {
			end_song_count = 0;
		}
		else if (!(_notes_heard & C3)) {
			end_song_count++;
		}
		break;
		
		case done_wait:
		_song_done = 1;
		break;
		
		default:
		break;
	}
	return state;
}

enum Game_menu_states {init_menu, Menu_wait, menu_up, menu_up_wait, menu_down, menu_down_wait, menu_start_wait, start_ini, Menu_wait_finish, FBB_ini, FBB_wait};

signed int Game_Menu(signed int state) {
	
	static unsigned char menu_count;
	
	switch(state) {
		
		case -1:
		state = init_menu;
		break;
		
		case init_menu:
		state = Menu_wait;
		break;
		
		case Menu_wait:
		if (_notes_heard == C4) {
			state = menu_down;
		}
		else if (_notes_heard == F4) {
			state = menu_up;
		}
		else if (_notes_heard == C3) {
			state = menu_start_wait;
		}
		else if (_notes_heard == (A3 | E4 | B3)) {
			state = FBB_ini;
		}
		else {
			state = Menu_wait;
		}
		
		
		break;
		
		case menu_down_wait:
		if (menu_count < 15) {
			state = menu_down_wait;
		}
		else if (menu_count >= 15) {
			state = Menu_wait;
		}
		break;
		
		case menu_down:
		state = menu_down_wait;
		break;
		
		case menu_up_wait:
		if (menu_count < 15) {
			state = menu_up_wait;
		}
		else if (menu_count >= 15) {
			state = Menu_wait;
		}
		break;
		
		case menu_up:
		state = menu_up_wait;
		break;
		
		case menu_start_wait:
		if (menu_count < 15) {
			state = menu_start_wait;
		}
		else if (menu_count >= 15) {
			state = start_ini;
		}
		break;
		
		case start_ini:
		state = Menu_wait_finish;
		break;
		
		case Menu_wait_finish:
		if (!(_song_done)) {
			state = Menu_wait_finish;
		}
		else if (_song_done) {
			state = init_menu;
		}
		break;
		
		case FBB_ini:
		state = FBB_wait;
		break;
		
		case FBB_wait:
		if (_FBB) {
			state = FBB_wait;
		}
		else if (!_FBB) {
			state = init_menu;
		}
		break;
		
		default:
		state = init_menu;
		break;
	}
	
	switch(state) {
		
		case -1:
		break;
		
		case init_menu:
		_SngNum = 0;
		_game_start = 0;
		_LED_on = 0;
		menu_count = 0;
		for (unsigned char i = 0; i<8; i++) {
			_blue[i] = 0;
			_green[i] = 0;
			_red[i] = 0;
		}
		
		LCD_ClearScreen();
		LCD_DisplayString( 1, _Songs[_SngNum]->name);
		if (_SngNum < _Number_of_Songs) {
			LCD_DisplayString( 17, _Songs[_SngNum + 1]->name);
		}
		break;
		
		case Menu_wait:
		menu_count = 0;
		break;
		
		case menu_up_wait:
		if (_notes_heard & F4) {
			menu_count = 0;
		}
		else if (!(_notes_heard & F4)) {
			menu_count++;
		}
		break;
		
		case menu_up:
		if (_SngNum < _Number_of_Songs) {
			_SngNum++;
		}
		LCD_ClearScreen();		
		LCD_DisplayString( 1, _Songs[_SngNum]->name);
		if (_SngNum < _Number_of_Songs) {
			LCD_DisplayString( 17, _Songs[_SngNum + 1]->name);
		}
		menu_count = 0;
		break;
		
		case menu_down_wait:
		if (_notes_heard & C4) {
			menu_count = 0;
		}
		else if (!(_notes_heard & C4)) {
			menu_count++;
		}
		break;
		
		case menu_start_wait:
		if (_notes_heard & C3) {
			menu_count = 0;
		}
		else if (!(_notes_heard & C3)) {
			menu_count++;
		}
		break;
		
		case menu_down:
		if (_SngNum > 0) {
			_SngNum--;
		}
		LCD_ClearScreen();
		LCD_DisplayString( 1, _Songs[_SngNum]->name);
		if (_SngNum < _Number_of_Songs) {
			LCD_DisplayString( 17, _Songs[_SngNum + 1]->name);
		}
		break;
		
		case start_ini:
		menu_count = 0;
		_LED_on = 1;
		_game_start = 1;
		_score_on = 1;
		LCD_ClearScreen();
		LCD_DisplayString(1, "Score:");
		LCD_DisplayString(17, "High Score:");
		
		_score[0] = _high_score[_SngNum]/10000;
		_score[1] = (_high_score[_SngNum]/1000) % 10;
		_score[2] = (_high_score[_SngNum]/100) % 10;
		_score[3] = (_high_score[_SngNum]/10) % 10;
		_score[4] = (_high_score[_SngNum])%10;
		
		LCD_WriteData(_score[0] + '0');
		LCD_WriteData(_score[1] + '0');
		LCD_WriteData(_score[2] + '0');
		LCD_WriteData(_score[3] + '0');
		LCD_WriteData(_score[4] + '0');
		
		break;
		
		case Menu_wait_finish:
		break;
		
		case FBB_ini:
		_FBB = 1;
		_LED_on = 1;
		LCD_ClearScreen();
		LCD_DisplayString(1, " FIRE BREATHING");
		LCD_DisplayString(17, "     BRISK!");
		break; 
		
		case FBB_wait:
		break;
		
		default:
		break;
		
		
	}
	return state;
}

enum Score_Board_States {Score_Off, cursor, write_score};
	
signed int Score_Board(signed int state) {
	
	static unsigned char cursor_place;
	
	switch(state) {
		case -1:
		state = Score_Off;
		break;
		
		case Score_Off:
		if (_score_on) {
			state = cursor;
		}
		else if (!_score_on) {
			state = Score_Off;
		}
		break;
		
		case cursor:
		if (_score_on) {
			state = write_score;
		}
		else if (!_score_on) {
			state = Score_Off;
		}
		break;
		
		case write_score:
		if (_score_on) {
			state = cursor;
		}
		else if (!_score_on) {
			state = Score_Off;
		}
		break;
		
		default:
		state = Score_Off;
		break;
	}
	
	switch(state) {
		case -1:
		break;
		
		case Score_Off:
		break;
		
		case cursor:
		if (cursor_place==0) {
			cursor_place = 10;
			LCD_Cursor2(cursor_place + 6);
		}
		else if (cursor_place >0) {
			LCD_Cursor2(cursor_place + 6);
		}
		break;
		
		case write_score:
		if (cursor_place >= 0){
			cursor_place--;
			
			if (cursor_place < 5) {
				_score[0] = _score_total/10000;
				_score[1] = (_score_total/1000) % 10;
				_score[2] = (_score_total/100) % 10;
				_score[3] = (_score_total/10) % 10;
				_score[4] = (_score_total) % 10;
				
				LCD_WriteData2(_score[cursor_place] + '0');
			}
			
			else if (cursor_place == 9) {
				LCD_WriteData2(_lives + '0');
			}
			
		}
		break;
		
		default:
		break;
	}
	
	return state;
}
	
enum FFB_States {FFB_off, FFB_frame1, FFB_frame2, FFB_frame3, FFB_frame4, FFB_frame5, FFB_frame6, FFB_frame7, FFB_frame8};

signed int FBB_Control(signed int state) {
	
	switch(state) {
		case -1:
		state = FFB_off;
		break;
		
		case FFB_off:
		if (_FBB) {
			state = FFB_frame1;
		}
		else if (!_FBB) {
			state = FFB_off;
		}
		break;
		
		case FFB_frame1:
		state = FFB_frame2;
		break;
		
		case FFB_frame2:
		state = FFB_frame3;
		break;
		
		case FFB_frame3:
		state = FFB_frame4;
		break;
		
		case FFB_frame4:
		state = FFB_frame5;
		break;
		
		case FFB_frame5:
		state = FFB_frame6;
		break;	
		
		case FFB_frame6:
		state = FFB_frame7;
		break;
		
		case FFB_frame7:
		state = FFB_frame8;
		break;
		
		case FFB_frame8:
		state = FFB_off;
		break;
		
		default:
		state = FFB_off;
		break;
	}
	
	switch(state) {
		case -1:
		break;
		
		case FFB_off:
		break;
		
		case FFB_frame1:
		for (unsigned i = 0; i<8; i++) {
			_blue[i] = blue1[i];
			_green[i] = green1[i];
			_red[i] = red1[i];
		}
		break;
		
		case FFB_frame2:
		for (unsigned i = 0; i<8; i++) {
			_red[i] = red2[i];
		}
		break;
		
		case FFB_frame3:
		for (unsigned i = 0; i<8; i++) {
			_red[i] = red3[i];
		}
		break;
		
		case FFB_frame4:
		for (unsigned i = 0; i<8; i++) {
			_red[i] = red4[i];
		}
		break;
		
		case FFB_frame5:
		for (unsigned i = 0; i<8; i++) {
			_red[i] = red5[i];
		}
		break;
		
		case FFB_frame6:
		for (unsigned i = 0; i<8; i++) {
			_red[i] = red4[i];
		}
		break;
		
		case FFB_frame7:
		for (unsigned i = 0; i<8; i++) {
			_red[i] = red2[i];
		}
		break;
	
		case FFB_frame8:
		for (unsigned i = 0; i<8; i++) {
			_red[i] = red1[i];
		}
		_FBB = 0;
		break;
	
		default:
		break;
	}
	return state;
	
}


//useful debugging tool
/*
ISR(BADISR_vect) {
}
*/

ISR(ADC_vect)  //called when conversion is complete
{
	ADC_Complete();
}

ISR(TIMER1_COMPB_vect) {
}


int main(void)
{
	DDRC = 0xFF; PORTC = 0x00; // LCD data lines
	DDRD = 0xFF; PORTD = 0x00; // LCD control lines
	DDRB = 0x00 ; PORTB = 0xFF;
	// initialize startup variables
	_fft_ready = 0;
	_measurement_cnt = 0;
	_notes_heard = 0;
	_game_start = 0;
	_LED_on = 0;
	_SngNum = 0;
	
	//initialize song files
	_Number_of_Songs = (sizeof(_Songs)/sizeof(Song_file*)) - 1;
	unsigned short high_scores[_Number_of_Songs];
	
	
	// load high scores
	unsigned short test_reboot = eeprom_read_word((uint16_t*)100);
	if (test_reboot != 11111) {
		for (uint16_t i = 0; i < 100; i++) {
			eeprom_write_word((uint16_t*)(i*2), 0);
		}
		eeprom_write_word((uint16_t*)100, 11111);
	}
	
	for (uint16_t i = 0; i < _Number_of_Songs; i++) {
	high_scores[i] = eeprom_read_word((uint16_t*)(i*2));
	}
	
		
	
	
	
	_high_score = high_scores;
	
	//Song of time
	Song_of_time.name = Song_of_time_name;
	Song_of_time.notes = Song_of_time_notes;
	Song_of_time.length = sizeof(Song_of_time_notes);
	Song_of_time.time_sig = 20;
	//Keyboard cat
	
	Keyboard_cat.name = Key_board_cat_name;
	Keyboard_cat.notes = Key_board_cat_notes;
	Keyboard_cat.length = sizeof(Key_board_cat_notes);
	Keyboard_cat.time_sig = 20;
	
	//C scale
	C_scale.name = C_scale_name;
	C_scale.notes = C_scale_notes;
	C_scale.length = sizeof(C_scale_notes);
	C_scale.time_sig = 20;
	
	for (unsigned char j = 0; j < N; j++) {
		input_temp[j] = 0;
	}
	
	
	for (unsigned char j = 0; j < 8; j++) {
		_blue[j] = 0;
		_red[j] = 0;
		_green[j] = 0;
	}
	
	// initialize hardware
	
	interrupt_enable();
	init_IO();
	init_SPI();
	ADC_timer_setup();
	ADC_init();
	ADC_start();
	LCD_init();
	
	
	//initialize state machines
	
	unsigned long int LED_controller_calc = 2;
	unsigned long int check_notes_calc = 2;
	unsigned long int Gameplay_calc = 50;
	unsigned long int fft_calc = 2;
	unsigned long int score_board_calc = 25;
	unsigned long int Menu_calc = 25;
	unsigned long int FBB_calc = 500;
	
	//Calculate GCD
	unsigned long tmpGCD = 1;
	tmpGCD = findGCD(LED_controller_calc, check_notes_calc);
	tmpGCD = findGCD(Gameplay_calc, tmpGCD);
	tmpGCD = findGCD(fft_calc, tmpGCD);
	tmpGCD = findGCD(score_board_calc, tmpGCD);
	tmpGCD = findGCD(Menu_calc, tmpGCD);
	tmpGCD = findGCD(FBB_calc, tmpGCD);
	
	
	unsigned long GCD = tmpGCD;
	
	//Calculate periods in terms of GCD ticks
	unsigned long int LED_controller_period = LED_controller_calc/GCD;
	unsigned long int check_notes_period = check_notes_calc/GCD;
	unsigned long int Gameplay_period = Gameplay_calc/GCD;
	unsigned long int fft_period = fft_calc/GCD;
	unsigned long int score_board_period = score_board_calc/GCD;
	unsigned long int Menu_period = Menu_calc/GCD;
	unsigned long int FBB_period = FBB_calc/GCD;
	
	// declare an array of tasks
	static task LED_task, Chk_note_task, Gameplay_task, fft_task, scoreboard_task, Menu_task, FBB_task;
	task *tasks[] = { &LED_task, &Chk_note_task, &Gameplay_task, &fft_task, &scoreboard_task, &Menu_task, &FBB_task};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
	
	// LED Task
	LED_task.state = -1;
	LED_task.period = LED_controller_period;
	LED_task.elapsedTime = LED_controller_period;
	LED_task.TickFct = &LED_controller;
	
	// Check for notes played task
	Chk_note_task.state = -1;
	Chk_note_task.period = check_notes_period;
	Chk_note_task.elapsedTime = check_notes_period;
	Chk_note_task.TickFct = &check_notes;
	
	// task 3 Display stuff on led task
	Gameplay_task.state = -1;
	Gameplay_task.period = Gameplay_period;
	Gameplay_task.elapsedTime = Gameplay_period;
	Gameplay_task.TickFct = &Gameplay;
	
	// fft task, run fft when data is ready
	fft_task.state = -1;
	fft_task.period = fft_period;
	fft_task.elapsedTime = fft_period;
	fft_task.TickFct = &FFT;
	
	// scoreboard task
	scoreboard_task.state = -1;
	scoreboard_task.period = score_board_period;
	scoreboard_task.elapsedTime = score_board_period;
	scoreboard_task.TickFct = &Score_Board;
	
	// Menu task
	Menu_task.state = -1;
	Menu_task.period = Menu_period;
	Menu_task.elapsedTime = Menu_period;
	Menu_task.TickFct = &Game_Menu;
	
	// FBB task
	FBB_task.state = -1;
	FBB_task.period = FBB_period;
	FBB_task.elapsedTime = FBB_period;
	FBB_task.TickFct = &FBB_Control;
	
	
	TimerSet(GCD);
	TimerOn();
	
	unsigned short i;
	
	while(1) {
		
		for ( i = 0; i< numTasks; i++) {
			if (tasks[i]->elapsedTime == tasks[i]->period ) {
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += 1;
		}
		while(!TimerFlag);
		TimerFlag = 0;
	}
}