#include <SI_EFM8BB3_Register_Enums.h>                // SFR declarations
#include<stdio.h>
#include<stdlib.h>

uint16_t delay_count;               // Used to implement a delay

int i = 0;
int time_arr[5] = {1, 2, 5, 10, 20};

void delay(int time) {
	for (delay_count = 25000*time; delay_count > 0; delay_count--);
}

void check_btn(){
	if(P0_B3 == 0){
		if(i==0){
			i=4;
		} else {
			i--;
		}
	}
	if(P0_B2 == 0){
		if(i==5){
			i=0;
		} else {
			i++;
		}
	}
}

int main (void) {

	XBR2 |= 0x40; //Enable Crossbar so we can easily turn pins on and off.
	while (1) {
		check_btn();
		P1_B7=1;
		delay(time_arr[i]);
		check_btn();
		P1_B7=0;
		delay(time_arr[i]);
		check_btn();
	}// Spin forever
}
