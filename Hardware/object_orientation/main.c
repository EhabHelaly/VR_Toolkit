/*
 * object_orientation.c
 *
 * Created: 6/17/2017 3:53:06 AM
 * Author : medo _senpie
 */ 

//#define F_CPU 8000000ul
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
//#include <util/delay.h>
#include <math.h>  //include libm

#include "mpu6050.h"



int main(void) {

	mpu6050_init();
	//_delay_ms(50);


	for(;;) {

		


	}

}
