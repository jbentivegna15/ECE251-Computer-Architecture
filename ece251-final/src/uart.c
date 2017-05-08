///////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////

#include "bsp.h"
#include <stdio.h>
#include "tick.h"
#include "disp.h"
#include "render.h"
#include "voltmeter.h"
#include "joystick.h"
#include "adc.h"
#include "thinfont.h"
#include "InitDevice.h"
#include <SI_EFM8BB3_Register_Enums.h>
#include "retargetserial.h"

// Configurator set for HFOSC0/8
#define SYSCLK             3062000

#define LED_TOGGLE_RATE           800 //toggles timer0 about every 100ms

///////////////////////////////////////////////////////////////////////////////
// PIN DEFINITIONS
///////////////////////////////////////////////////////////////////////////////
SI_SBIT(BTN0, SFR_P0, 2);                 // P0.2 BTN0
SI_SBIT(BTN1, SFR_P0, 3);                 // P0.3 BTN1
SI_SBIT(LEDG, SFR_P1, 4);				  // P1.4 LED GREEN

SI_SBIT (DISP_EN, SFR_P3, 4);          // Display Enable
#define DISP_BC_DRIVEN   0             // 0 = Board Controller drives display
#define DISP_EFM8_DRIVEN 1             // 1 = EFM8 drives display

SI_SBIT (BC_EN, SFR_P2, 2);            // Board Controller Enable
#define BC_DISCONNECTED 0              // 0 = Board Controller disconnected
                                       //     to EFM8 UART pins
#define BC_CONNECTED    1              // 1 = Board Controller connected
                                       //     to EFM8 UART pins

#define SCREEN_HEIGHT 11
#define SCREEN_WIDTH 20
#define BUFFER_EXTRA 10

///////////////////////////////////////////////////////////////////////////////
// Globals
///////////////////////////////////////////////////////////////////////////////

// Generic line buffer
SI_SEGMENT_VARIABLE(Line[DISP_BUF_SIZE], uint8_t, RENDER_LINE_SEG);
SI_SEGMENT_VARIABLE(String[DISP_BUF_SIZE], uint8_t, RENDER_LINE_SEG);

static unsigned char xdata buffer[SCREEN_HEIGHT+BUFFER_EXTRA][SCREEN_WIDTH] = {0};
static unsigned char xdata hexbuff[SCREEN_HEIGHT+BUFFER_EXTRA][SCREEN_WIDTH] = {0};
static const unsigned char blank[SCREEN_WIDTH] = {0};

unsigned char hex[3] = {0};
uint8_t hex_mode = 0;
uint8_t drawMode = 1;

uint8_t i = 0;
uint8_t j = 0;

uint8_t m = 0;
uint8_t n = 0; //hex buff indicies

uint8_t insertion_mode = 1; //insertion mode == 1, typing possible, ==0, typing not possible

uint8_t dir; //joystick direction

uint8_t topASCII = 0;
uint8_t topHEX = 0; //top row of each buffer visible on LCD

uint8_t drawASCII = 0;
uint8_t drawHEX = 0;



///////////////////////////////////////////////////////////////////////////////
// Supporting Functions
//////////////////////////////////////////////////////////////////////////////

//(taken from voltmeter.c)
//-----------------------------------------------------------------------------
// drawScreenText
//-----------------------------------------------------------------------------
//
// Show one line of text on the screen
//
// str - pointer to string text (0 - 21 characters) to display
// rowNum - row number of the screen (0 - 127)
// fontScale - font scaler (1 - 4)
//
void drawScreenText(SI_VARIABLE_SEGMENT_POINTER(str, char, RENDER_STR_SEG), uint8_t rowNum, uint8_t fontScale)
{
  uint8_t i, j;

  for (i = 0; i < FONT_HEIGHT; i++)
  {
    RENDER_ClrLine(Line);
    RENDER_Large_StrLine(Line, 4, i, (SI_VARIABLE_SEGMENT_POINTER(, char, RENDER_STR_SEG))str, fontScale);

    for (j = 1; j < fontScale + 1; j++)
    {
      DISP_WriteLine(rowNum + (i * fontScale) + j, Line);
    }
  }
}

char hexDigit(unsigned n){
    if (n < 10) {
        return n + '0';
    } else {
        return (n - 10) + 'A';
    }
}

void charToHex(char c){
    hex[0] = hexDigit(c / 0x10);
    hex[1] = hexDigit(c % 0x10);
    hex[2] = 32;
}

void charToHexbuff(char c){
	charToHex(buffer[i][j]);
	hexbuff[m][n] = hex[0];
	hexbuff[m][n+1] = hex[1];
	hexbuff[m][n+2] = hex[2];
}

void blankBlock(void){
	uint8_t p;
	for(p=0; p<SCREEN_HEIGHT+1; p++){ //blank screen
		drawScreenText(blank, p*10, 1);
	}
}

void drawBlockHEX(void){
	uint8_t p;
	if(m-topHEX>SCREEN_HEIGHT){
		for(p=topHEX; p<SCREEN_HEIGHT; p++){ //draw buffer from topASCII to i
			drawScreenText(hexbuff[p], (p-topHEX)*10, 1);
		}
	} else {
		for(p=topHEX; p<=m; p++){ //draw buffer from topASCII to i
			drawScreenText(hexbuff[p], (p-topHEX)*10, 1);
		}
	}
}

void drawBlockASCII(void){
	uint8_t p;
	if(i-topASCII>SCREEN_HEIGHT){
		for(p=topASCII; p<SCREEN_HEIGHT; p++){ //draw buffer from topASCII to i
			drawScreenText(buffer[p], (p-topASCII)*10, 1);
		}
	} else {
		for(p=topASCII; p<=i; p++){ //draw buffer from topASCII to i
			drawScreenText(buffer[p], (p-topASCII)*10, 1);
		}
	}
}

void redrawScreen(void){
	if(drawMode == 1){
		if(hex_mode == 0 && insertion_mode == 1){
			drawScreenText("Write          ASCII", 120, 1);
		} else if(hex_mode == 1 && insertion_mode == 1){
			drawScreenText("Write            HEX", 120, 1);
		} else if(hex_mode == 0 && insertion_mode == 0){
			drawScreenText("Read           ASCII", 120, 1);
		} else if(hex_mode == 1 && insertion_mode == 0){
			drawScreenText("Read             HEX", 120, 1);
		}
		drawMode = 0;
	}
	if(hex_mode == 1 && drawHEX == 1){ //REDRAWING HEXIDECIMAL
		blankBlock();
		drawBlockHEX();
		drawHEX = 0;
	} else if(hex_mode == 0 && drawASCII == 1){ //REDRAWING ASCII
		blankBlock();
		drawBlockASCII();
		drawASCII = 0;
	}
}

void scroll_check(void){
	dir = JOYSTICK_convert_mv_to_direction(ADC_GetVoltage());
	if(dir == JOYSTICK_N){
		if(topASCII<SCREEN_HEIGHT+BUFFER_EXTRA && hex_mode == 0){
			topASCII++;
			drawASCII = 1;
		}
		if(topHEX<SCREEN_HEIGHT+BUFFER_EXTRA && hex_mode == 1){
			topHEX++;
			drawHEX = 1;
		}
	}
	else if (dir == JOYSTICK_S){
		if(topASCII>0 && hex_mode == 0){
			topASCII--;
			drawASCII = 1;
		}
		if(topHEX>0 && hex_mode == 1){
			topHEX--;
			drawHEX = 1;
		}
	}
}

void insertchar(void){
	buffer[i][j] = getchar();

	if(buffer[i][j] == 27){  //if ESC
		insertion_mode = 0;
		drawMode = 1;
		buffer[i][j] = 0;

	} else if(buffer[i][j] == 8 || buffer[i][j] == 127) { //if Backspace or DEL
		buffer[i][j] = 0;
		if(hex_mode == 0 && insertion_mode == 1){
			if(j==0){
				i--;
				j=19;
				buffer[i][j] = 0;
				drawScreenText(buffer[i], i*10, 1);
			} else{
				j--;
				buffer[i][j] = 0;
				drawScreenText(buffer[i], i*10, 1);
			}
		} else if (hex_mode == 1 && insertion_mode == 1){
			if(n<2){
				m--;
				n=17;
				hexbuff[m][n] = 0;
				hexbuff[m][n-1] = 0;
				hexbuff[m][n-2] = 0;
				n = n-2;
				drawScreenText(hexbuff[m], m*10, 1);
			} else{
				n--;
				buffer[m][n] = 0;
				hexbuff[m][n-1] = 0;
				hexbuff[m][n-2] = 0;
				n = n-2;
				drawScreenText(hexbuff[m], m*10, 1);
			}
		}

	} else {
		charToHexbuff(buffer[i][j]);

		if(hex_mode == 1 && insertion_mode == 1){
			drawScreenText(hexbuff[m], (m-topHEX)*10, 1);
		} else if(hex_mode == 0 && insertion_mode == 1){
			drawScreenText(buffer[i], (i-topASCII)*10, 1);
		}

		j++;
		n = n+3;

		if(j==SCREEN_WIDTH-2){
			j=0;
			i++;
		}
		if(i==SCREEN_HEIGHT+BUFFER_EXTRA){
			i=0;
			topASCII = 0;
		}else if(i==topASCII+SCREEN_HEIGHT){
			topASCII++;
			drawASCII = 1;
		}
		if(n>SCREEN_WIDTH-3){
			n=0;
			m++;
		}
		if(m==SCREEN_HEIGHT+BUFFER_EXTRA){
			m=0;
			topHEX = 0;
		}else if(m==topHEX+SCREEN_HEIGHT){
			topHEX++;
			drawHEX = 1;
		}
	}
}


//sets up timer0 (off by default)
void initTimer0(){
	uint8_t TCON_save;
	TCON_save = TCON;
	TCON &= ~TCON_TR0__BMASK & ~TCON_TR1__BMASK;
	TH0 = 0xC0; //(0xC0 << TH0_TH0__SHIFT);
	TCON |= (TCON_save & TCON_TR0__BMASK) | (TCON_save & TCON_TR1__BMASK);

	CKCON0 = CKCON0_SCA__SYSCLK_DIV_48;

	TMOD = TMOD_T0M__MODE2 | TMOD_T1M__MODE0 | TMOD_CT0__TIMER
				| TMOD_GATE0__DISABLED | TMOD_CT1__TIMER | TMOD_GATE1__DISABLED;

	TCON |= TCON_TR0__RUN;

	IE_ET0 = 0;
}

//sets up BTN interrupts (on by default)
void initBTNinterrupts(void){
	IT01CF = IT01CF_IN0PL__ACTIVE_LOW | IT01CF_IN0SL__P0_2
				| IT01CF_IN1PL__ACTIVE_LOW | IT01CF_IN1SL__P0_3;
	IE_EX0 = 1;
	IE_EX1 = 1;
}

void initUART0(void){
	SCON0 |= SCON0_REN__RECEIVE_ENABLED;
}

void initTimer1(void){
	uint8_t TCON_save;
	TCON_save = TCON;
	TCON &= ~TCON_TR0__BMASK & ~TCON_TR1__BMASK;
	TH1 = (0x96 << TH1_TH1__SHIFT);
	TCON |= (TCON_save & TCON_TR0__BMASK) | (TCON_save & TCON_TR1__BMASK);

	CKCON0 = CKCON0_SCA__SYSCLK_DIV_12 | CKCON0_T0M__PRESCALE
				| CKCON0_T2MH__EXTERNAL_CLOCK | CKCON0_T2ML__EXTERNAL_CLOCK
				| CKCON0_T3MH__EXTERNAL_CLOCK | CKCON0_T3ML__EXTERNAL_CLOCK
				| CKCON0_T1M__SYSCLK;

	TMOD = TMOD_T0M__MODE0 | TMOD_T1M__MODE2 | TMOD_CT0__TIMER
				| TMOD_GATE0__DISABLED | TMOD_CT1__TIMER | TMOD_GATE1__DISABLED;

	TCON |= TCON_TR1__RUN;
}

///////////////////////////////////////////////////////////////////////////////
// Interrupt Service Routines
///////////////////////////////////////////////////////////////////////////////

//BTN0 TRIGGERED INTERRUPT
SI_INTERRUPT (INT0_ISR, INT0_IRQn)
{
	if(BTN0 == 0){
		hex_mode = !hex_mode;
		drawMode = 1;
		if(hex_mode == 1){
			drawHEX = 1;
		} else drawASCII = 1;
	}
	IE_EX0 = 0; //TURN OFF EXTERNAL0 INTERRUPTS (SO THAT MULTIPLE BTN0 PRESSES ARE NOT REGISTERED)
}

//BTN1 TRIGGERED INTERRUPT
SI_INTERRUPT (INT1_ISR, INT1_IRQn)
{
	if(BTN1 == 0 && insertion_mode == 0){
		insertion_mode = 1;
		drawMode = 1;
	}

	IE_EX1 = 0; //TURN OFF EXTERNAL1 INTERRUPTS (SO THAT MULTIPLE BTN1 PRESSES ARE NOT REGISTERED)
}


//-----------------------------------------------------------------------------
// TIMER0_ISR
//-----------------------------------------------------------------------------
// Increments variable 'time' by 1 every time LED_TOGGLE RATE is reached
//-----------------------------------------------------------------------------
SI_INTERRUPT (TIMER0_ISR, TIMER0_IRQn)
{
   static uint16_t counter = 0;

   counter++;

   if(counter == LED_TOGGLE_RATE)
   {
	  counter = 0;
   }
}

///////////////////////////////////////////////////////////////////////////////
// Driver Function
///////////////////////////////////////////////////////////////////////////////

void uart_main(void)
{
	initBTNinterrupts();
	initTimer1();
	initUART0();

    BC_EN = BC_DISCONNECTED;               // Board controller connected to EFM8
									   // UART pins

    SCON0_TI = 1;                       // This STDIO library requires TI to
									   // be set for prints to occur

	while(1)
	{
		redrawScreen();

		if(insertion_mode == 1){
			insertchar();
		} else {
			scroll_check();
		}


		if(BTN0 == 1) //BTN0 UNPRESSED
			IE_EX0 = 1; //TURN BACK ON EXTERNAL (BTN) INTERRUPTS
		if(BTN1 == 1)
			IE_EX1 = 1;
	}
}
