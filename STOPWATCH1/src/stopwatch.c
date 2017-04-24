/**************************************************************************//**
 * Copyright (c) 2015 by Silicon Laboratories Inc. All rights reserved.
 *
 * http://developer.silabs.com/legal/version/v11/Silicon_Labs_Software_License_Agreement.txt
 *****************************************************************************/
///////////////////////////////////////////////////////////////////////////////
// voltmeter.c
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Includes
///////////////////////////////////////////////////////////////////////////////

#include "bsp.h"
#include "tick.h"
#include "disp.h"
#include "render.h"
#include "voltmeter.h"
#include "joystick.h"
#include "adc.h"
#include "thinfont.h"
#include "InitDevice.h"
#include <SI_EFM8BB3_Register_Enums.h>

// Configurator set for HFOSC0/8
#define SYSCLK             3062000

// Configurator set for timer overflow every 100 ms / 10 Hz.
#define LED_TOGGLE_RATE           800 // LED toggle rate in milliseconds
                                       // if LED_TOGGLE_RATE = 1, the LED will
                                       // be on for 1 millisecond and off for
                                       // 1 millisecond

///////////////////////////////////////////////////////////////////////////////
// PIN DEFINITIONS
///////////////////////////////////////////////////////////////////////////////
SI_SBIT(LED1, SFR_P1, 4);                 //P1.4 LED1
SI_SBIT(BTN0, SFR_P0, 2);                 // P0.2 BTN0
SI_SBIT(BTN1, SFR_P0, 3);                 // P0.3 BTN1

///////////////////////////////////////////////////////////////////////////////
// Globals
///////////////////////////////////////////////////////////////////////////////

// Generic line buffer
SI_SEGMENT_VARIABLE(Line[DISP_BUF_SIZE], uint8_t, RENDER_LINE_SEG);
SI_SEGMENT_VARIABLE(String[DISP_BUF_SIZE], uint8_t, RENDER_LINE_SEG);

uint16_t time;
char timeStr[9];
uint16_t Laps[9];
uint8_t lapcount;
uint8_t currentlap;
uint8_t start_track;

///////////////////////////////////////////////////////////////////////////////
// Supporting Functions
//////////////////////////////////////////////////////////////////////////////
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

void time2str(uint16_t t, char * str)
{
  int8_t pos = 7;
  uint8_t count = 0;

  // Work backwards generating the string
  // Start with null-terminator
  // followed by V symbol
  str[pos--] = '\0';
  str[pos--] = 's';
  str[pos--] = ' ';

  // followed by thousandths digit
  // followed by hundredths digit
  // followed by tenths digit
  // followed by the decimal point at pos=1
  // followed by the ones digit
  for (; pos >= 0; pos--)
  {
    // Pos = 1 is the location of the decimal point
    if (count == 1)
    {
      str[pos--] = '.';
    }
	// Get the right-most digit from the number
	// and convert to ascii
	str[pos] = (t % 10) + '0';

	// Shift number by 1 decimal place to the right
	t /= 10;
	count+=1;
  }
}

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

void initBTNinterrupts(void){
	IT01CF = IT01CF_IN0PL__ACTIVE_LOW | IT01CF_IN0SL__P0_2
				| IT01CF_IN1PL__ACTIVE_LOW | IT01CF_IN1SL__P0_3;
	IE_EX0 = 1;
	IE_EX1 = 1;
}

void drawTime(){
	time2str(time, timeStr);
	drawScreenText(timeStr, 20, 2);
}

void drawNewLap(){
	uint16_t temp_time;
	if(currentlap != lapcount && lapcount<9){
		Laps[currentlap] = time;
		if(currentlap == 0){
			drawScreenText("Laps:", 40, 1);
			time2str(time, timeStr);
			drawScreenText(timeStr, 50, 1);
		}else {
			temp_time = Laps[currentlap]-Laps[currentlap-1];
			time2str(temp_time, timeStr);
			drawScreenText(timeStr, 50+(8*currentlap), 1);
		}
		currentlap = lapcount;
	}
}

///////////////////////////////////////////////////////////////////////////////
// Interrupt Service Routines
///////////////////////////////////////////////////////////////////////////////

//BTN0 TRIGGERED INTERRUPT
SI_INTERRUPT (INT0_ISR, INT0_IRQn)
{
	if(BTN0 == 0){
		IE_ET0 = !IE_ET0;
		start_track = !start_track;
	}
	IE_EX0 = 0; //TURN OFF EXTERNAL0 INTERRUPTS (SO THAT MULTIPLE BTN PRESSES ARE NOT REGISTERED)
}

SI_INTERRUPT (INT1_ISR, INT1_IRQn)
{
	if(BTN1 == 0){
		lapcount++;
	}
	IE_EX1 = 0;
}


//-----------------------------------------------------------------------------
// TIMER0_ISR
//-----------------------------------------------------------------------------
//
// TIMER0 ISR Content goes here. Remember to clear flag bits:
// TCON::TF0 (Timer 0 Overflow Flag)
//
// Here we process the Timer0 interrupt and toggle the LED when appropriate
//
//-----------------------------------------------------------------------------
SI_INTERRUPT (TIMER0_ISR, TIMER0_IRQn)
{
   static uint16_t counter = 0;

   counter++;

   if(counter == LED_TOGGLE_RATE)
   {
	  time++;
	  counter = 0;
   }
}

extern void start(void);

///////////////////////////////////////////////////////////////////////////////
// Driver Function
///////////////////////////////////////////////////////////////////////////////

void Stopwatch_main(void)
{
	initBTNinterrupts();
	initTimer0();

	drawScreenText("STOPWATCH", 0, 2);
	drawScreenText("\t\t\t\t\t\t\t\t\tJoey&Nikola", 116, 1); //Draws string in bottom right corner
	time = 0;
	lapcount = 0;
	currentlap = 0;
	start_track = 0;

	while(1)
	{
		drawTime();
		drawNewLap();
		if((start_track == 1) && (IE_EX0 == 0)){
			IE_ET0 = 0;
			start();
			IE_ET0 = 1;
		}

		if(BTN0 == 1) //BTN0 UNPRESSED
			IE_EX0 = 1; //TURN BACK ON EXTERNAL (BTN) INTERRUPTS
		if(BTN1 == 1)
			IE_EX1 = 1;
	}
}
