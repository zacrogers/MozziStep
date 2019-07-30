/*
	******************* muxlib.h ************************* 
	Library for controlling multiplexers with raspberry pi
	******************************************************
*/
#ifndef _MUXLIB_H
#define _MUXLIB_H

#include <stdio.h>
#include <stdlib.h>
#include "Arduino.h"
// #include <wiringPi.h> 

/****** 4051 multiplexer  pinout *******

	  ************ Pins ************
		1: I/0 4		16: VDD
		2: I/0 6		15: I/0 2
		3: O/I 			14: I/0 1
		4: I/0 7		13: I/0 0
		5: I/0 5		12: I/0 3
		6: Inhibit		11: Select A
		7: VEE			10: Select B
		8: VSS			9: 	Select C
	  ********************************
*/

/* Valid statuses for mux enable pin */
typedef enum{LO, HI, INACTIVE} ENABLE_STATUS;

/* Holds bytecode in individual bits */
typedef struct{unsigned char *bits;} BYTE_CODE;

/* MUX, store total channels, and GPIO pin assignments */
typedef struct mux{
	unsigned char msize;
	unsigned char s0, s1, s2, s3, s4;
	ENABLE_STATUS enable;
}MUX;

/******Prototypes******/

/* Create new MUX set total channels (8 or 16 or 32), Set GPIO pin assignments for select lines */
MUX new_mux(ENABLE_STATUS enable, unsigned char msize , unsigned char s0, unsigned char s1, 
  			unsigned char s2,  unsigned char s3, unsigned char s4);

/* Quick constructors for different MUX  sizes. Sizes are pre set for each */
MUX new_mux_4(ENABLE_STATUS enable, unsigned char s0, unsigned char s1);
MUX new_mux_8(ENABLE_STATUS enable, unsigned char s0, unsigned char s1, unsigned char s2);
MUX new_mux_16(ENABLE_STATUS enable, unsigned char s0, unsigned char s1, unsigned char s2, unsigned char s3);
MUX new_mux_32(ENABLE_STATUS enable, unsigned char s0, unsigned char s1, unsigned char s2, unsigned char s3, unsigned char s4);

/* Convert integer to array of individual bytes. Use to set MUX select lines */
static BYTE_CODE new_bytecode(unsigned char num, unsigned char msize);

/* Set status of MUX enable pin*/
void set_enable(MUX *mux, ENABLE_STATUS enable);

/* For checking that status used is valid*/
static int status_valid(ENABLE_STATUS estatus);

/* Set current IO pin for MUX */
void set_mux_channel(MUX *mux, unsigned char channel);

/*	Print GPIO pins MUX is connected to */
void print_mux_gpio(MUX *mux);




#endif
