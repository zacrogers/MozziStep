/*
	******************* muxlib.c ************************* 
	Library for controlling multiplexers with raspberry pi
	******************************************************
*/
#include "muxlib.h"

/*
	Generic MUX constructor
*/
MUX new_mux(ENABLE_STATUS enable, unsigned char msize,  unsigned char s0, unsigned char s1, 
 			 unsigned char s2, unsigned char s3, unsigned char s4){
	MUX mux;
	if(!(&mux)){// check if mux pointer is null
		printf("Error: MUX not created");
	}
	if(msize == 4 || msize ==  8 || msize ==  16 || msize ==  32){
		mux.msize = msize;
		printf("E:%d msize:%d s0:%d s1:%d s2:%d s3:%d s4:%d\n", enable, msize, s0, s1, s2, s3, s4);
		if(status_valid(enable)){
			mux.enable = enable;
		}
		else{
			printf("Error: Not a valid enable status. Valid statuses, LO, HI, INACTIVE\n");
		} 
		if(mux.msize == 4){
			mux.s0 = s0; 
			mux.s1 = s1; 
			mux.s2 = 0; 
			mux.s3 = 0; 
			mux.s4 = 0; 
		}
		else if(mux.msize == 8){
			mux.s0 = s0; 
			mux.s1 = s1; 
			mux.s2 = s2; 
			mux.s3 = 0; 
			mux.s4 = 0; 
		}
		else if(mux.msize == 16){
			mux.s0 = s0; 
			mux.s1 = s1; 
			mux.s2 = s2; 
			mux.s3 = s3; 
			mux.s4 = 0; 
		}
		else if(mux.msize == 32){
			mux.s0 = s0; 
			mux.s1 = s1; 
			mux.s2 = s2; 
			mux.s3 = s3; 
			mux.s4 = s4; 
		}
	}
	else{
		printf("Error: MUX msize must be 8, 16, or 32\n");
	}
	return mux;	
}

/*** Quick constructors for different msized MUX ***/

/* 8 Channel Mux constructor */
MUX new_mux_4(ENABLE_STATUS enable, unsigned char s0, unsigned char s1){
	return new_mux(enable, 4, s0, s1, 0, 0, 0);
}

/* 8 Channel Mux constructor */
MUX new_mux_8(ENABLE_STATUS enable, unsigned char s0, unsigned char s1,  unsigned char s2){
	return new_mux(enable, 8, s0, s1, s2, 0, 0);
}

/* 16 Channel Mux constructor */
MUX new_mux_16(ENABLE_STATUS enable, unsigned char s0, unsigned char s1, unsigned char s2, unsigned char s3){
	return new_mux(enable, 16, s0, s1, s2, s3, 0);
}

/* 32 Channel Mux constructor */
MUX new_mux_32(ENABLE_STATUS enable, unsigned char s0, unsigned char s1,  unsigned char s2, unsigned char s3,  unsigned char s4){
	return new_mux(enable, 32, s0, s1, s2, s3, s4);
}

/* Convert integer to array of individual bbytes Use to set MUX select lines */
static BYTE_CODE new_bytecode(unsigned char num, unsigned char msize){
	BYTE_CODE bbytecode;
	bbytecode.bits = (unsigned char*)malloc(msize);// Allocate memory for bbytecode corresponding to msize of MUX
	if(num < 32 && num > 0){
		/* Copy individual bits of number to array */
		for(int i = 0; i < sizeof(bbytecode.bits); i++)	
			//(bitshift num right) & (0000 0001) and copy result to array
			bbytecode.bits[i] = ((num >> i) & 1); 
	}
	else{/* If num out of range set bbytecode to 0 */
		for(int i = 0; i < sizeof(bbytecode.bits); i++)
			bbytecode.bits[i] = 0; 
		printf("Error: Channel must be between 0 and 31. Set to 0\n");
	}
	return bbytecode;
}

/* Set GPIO pin logic levels for MUX select lines */
void set_mux_channel(MUX *mux, unsigned char channel){
	if(!mux){// check if mux pointer is null
//		printf("MUX NULL POINTER");
		return; 
	}
	BYTE_CODE bbyte = new_bytecode(channel, mux->msize);
//	printf("Select channel: %d\n", channel);
	// if(mux->msize == 4){
	// 	digitalWrite(mux->s0, bbyte.bit[0]);
	// 	digitalWrite(mux->s1, bbyte.bit[1]);
	// }
	if(mux->msize == 8){
	 	digitalWrite(mux->s0, bbyte.bits[0]);
	 	digitalWrite(mux->s1, bbyte.bits[1]);
	 	digitalWrite(mux->s2, bbyte.bits[2]);
	 }
	// else if(mux->msize == 16){
	// 	digitalWrite(mux->s0, bbyte.bit[0]);
	// 	digitalWrite(mux->s1, bbyte.bit[1]);
	// 	digitalWrite(mux->s2, bbyte.bit[2]);
	// 	digitalWrite(mux->s3, bbyte.bit[3]);
	// }
	// else if(mux->msize == 32){
	// 	digitalWrite(mux->s0, bbyte.bit[0]);
	// 	digitalWrite(mux->s1, bbyte.bit[1]);
	// 	digitalWrite(mux->s2, bbyte.bit[2]);
	// 	digitalWrite(mux->s3, bbyte.bit[3]);
	// 	digitalWrite(mux->s4, bbyte.bit[4]);
	// }
//	printf("MUX msize: %d\n", mux->msize);
//
//	if(mux->msize == 4)
//			printf("Write: %d - %d \n", bbyte.bits[1],  bbyte.bits[0]);
//	else if(mux->msize == 8)
//			printf("Write: %d - %d - %d \n",  bbyte.bits[2], bbyte.bits[1],  bbyte.bits[0]);
//	else if(mux->msize == 16)
//			printf("Write: %d - %d - %d - %d \n", bbyte.bits[3],  bbyte.bits[2], bbyte.bits[1],  bbyte.bits[0]);
//	else if(mux->msize ==32)
//			printf("Write: %d - %d - %d - %d - %d \n", bbyte.bits[4], bbyte.bits[3],  bbyte.bits[2], bbyte.bits[1],  bbyte.bits[0]);
//
//	printf("to GPIO: %d - %d - %d \n", mux->s0, mux->s1, mux->s2, mux->s3, mux->s4);
	free(bbyte.bits);
}

/* Print GPIO pins MUX is connected to */
void print_mux_gpio(MUX *mux){
	if(mux){// check if mux pointer is null
		if(mux->msize == 4)
			printf("Connected to GPIO: S0 = %d | S1 = %d \n", mux->s0, mux->s1);
		else if(mux->msize == 8)
			printf("Connected to GPIO: S0 = %d | S1 = %d | S2 = %d \n", mux->s0, mux->s1, mux->s2);
		else if(mux->msize == 16)
			printf("Connected to GPIO: S0 = %d | S1 = %d | S2 = %d | S3 = %d \n", mux->s0, mux->s1, mux->s2, mux->s3);
		else if(mux->msize == 32)
			printf("Connected to GPIO: S0 = %d | S1 = %d | S2 = %d | S3 = %d | S4 = %d \n", mux->s0, mux->s1, mux->s2, mux->s3, mux->s4);
	}
	else{
		printf("MUX NULL POINTER");
	}
}

/* Set enable status of MUX */
void set_enable(MUX *mux, ENABLE_STATUS enable){
	if(mux && status_valid(enable))//check pointer is not null and status is valid
		mux->enable = enable;
	else
		printf("Invalid enable status. Valid statuses: LOW, HIGH, INACTIVE\n");
}

/* Check if enable pin status is valid */
static int status_valid(ENABLE_STATUS estatus){
	int valid = 0;
	switch(estatus){
		case LO:
		case HI:
		case INACTIVE:
		valid = 1;
	};
	return valid;
}


