#include <SI_EFM8BB3_Register_Enums.h> // SFR declarations
#include <stdio.h>
#include <stdlib.h>
#include "InitDevice.h"

#define TIME 1000 //appx 0.5secs

extern void WDT_0_enter_DefaultMode_from_RESET(void) {
	// $[WDTCN - Watchdog Timer Control]
	SFRPAGE = 0x00;
	//Disable Watchdog with key sequence
	WDTCN = 0xDE; //First key
	WDTCN = 0xAD; //Second key
	// [WDTCN - Watchdog Timer Control]$

}

//GPIO & SFR definitions
SI_SBIT(LED0, SFR_P1, 4);                  // P1.4 LED0	GREEN
SI_SBIT(LED1, SFR_P1, 5);                  // P1.5 LED1 BLUE
SI_SBIT(LED2, SFR_P1, 6);                  // P1.6 LED2 RED

SI_SBIT(BTN0, SFR_P0, 2);                  // P0.2 BTN0
SI_SBIT(BTN1, SFR_P0, 3);                  // P0.3 BTN1

SI_SBIT(OP0, SFR_P1, 0);   				   // P1.0 OP0
SI_SBIT(OP1, SFR_P1, 1);   				   // P1.1 OP1

SI_SBIT(CLK, SFR_P1, 3);				   // P1.3 CLK

//TEMP REGISTER
SI_SBIT(T0, SFR_P3, 4);   				   // P3.4 LSB
SI_SBIT(T1, SFR_P3, 1);   				   // P3.1
SI_SBIT(T2, SFR_P3, 2);   				   // P3.2
SI_SBIT(T3, SFR_P3, 3);   				   // P3.3 MSB

//time delay of default TIME seconds
void delay(int n) {
	uint16_t i;
	int j;
	for(j=0;j<n;j++){
		for (i=0; i<TIME; i++);
	}
}

//blinks one of 3 LEDs n times
void blink(int LED, int n){
	if(n<=0){
		printf("nfERROR: blink() arg not valid n\n"); //cannot blink negative times (nf: non-fatal)
	} else{
		switch(LED){
			case 0:
				while(n>0){
					LED0 = 0;
					delay(1);
					LED0 = 1;
					delay(1);
					n--;
				}
				break;
			case 1:
				while(n>0){
					LED1 = 0;
					delay(1);
					LED1 = 1;
					delay(1);
					n--;
				}
				break;
			case 2:
				while(n>0){
					LED2 = 0;
					delay(1);
					LED2 = 1;
					delay(1);
					n--;
				}
				break;
		}
	}
}

//clock signal 1 pulse based on delay()
void clk(){
	CLK = 0;
	delay(1);
	CLK = 1;
	delay(1);
	CLK = 0;
	delay(1);

	blink(2,1); //visual aid
}

//OP code translation (from dec. to GPIOpins) (valid ops: 0,1,2,3)
void op(int opcode){
	switch(opcode){
		case 0: 		//ZEROS
			OP0=0;
			OP1=0;
			break;
		case 1: 		//AND
			OP0=1;
			OP1=0;
			break;
		case 2: 		//OR
			OP0=0;
			OP1=1;
			break;
		case 3: 		//ADDITION
			OP0=1;
			OP1=1;
			break;
	}

	blink(1,1);		//for visual aid
}

int* dec2bin(int c){
	int *arr = malloc(4);

	int i = 0;
	for(i = 31; i >= 0; i--){
		if((c & (1 << i)) != 0){
			arr[i] = 1;
		}else{
			arr[i] = 0;
		}
	}
	return arr;
}

void uTemp(int num){
	int *binptr;
	binptr = dec2bin(num);
	T0 = *(binptr);
	T1 = *(binptr+1);
	T2 = *(binptr+2);
	T3 = *(binptr+3);

	free(binptr);
	blink(0,1);
}

void slowDownCount(int num){
	int i;
	for(i=num;i>=0; i--){
		delay(100);
		op(0);
		clk();
		uTemp(i);
		op(2);
		clk();

		delay(100);
	}
}

void slowUpCount(int num){
	int i;
	for(i=0;i<=num; i++){
		delay(100);
		op(0);
		clk();
		uTemp(i);
		op(2);
		clk();

		delay(100);
	}
}

int main(void) {
	WDT_0_enter_DefaultMode_from_RESET();
	XBR2 |= 0x40; //Enable Crossbar so we can easily turn pins on and off


	op(0);
	clk();

	while (1) {
		if(BTN0==0){
			op(0);
			clk();

			uTemp(7);
			op(2);
			clk();
			delay(100);
			op(0);
			clk();

			uTemp(8);
			op(2);
			clk();
			delay(100);
			op(0);
			clk();

			uTemp(7);
			op(2);
			clk();
			delay(100);



			delay(100);
		}

	} // Spin forever
}
