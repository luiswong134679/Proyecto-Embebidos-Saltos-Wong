#include "sonico.h"

void ultraSonicoInit(){
	DDRD|=(1<<DDD6);//PIN TRIGGED
	DDRD &=~(1<<DDD7);//PIN ECHO;
	PORTD &=~ (1<<6) | (1<<7);//SE INICIALIZAN EN ESTADO BAJO
	TCCR2A=0x00;
	TCCR2B |= (1<<CS11)|(1<<CS10);
	
}

long getDistance(){
	PORTD|=(1<<6);
	_delay_us(10);
	PORTD &=~(1<<6);
	while ((PIND>>7) && 1 == 0);
	TCNT2=0;
	while((PIND>>5) && 1);
	long long distancia = TCNT2*8*0.017;
	return distancia;
	
}