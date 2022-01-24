#include "servo.h"

void servoInit(){
	/*INICIALIZAR PWM*/
	
	DDRB |= ( 1<< PB1 );  // Configuramos el PB1 como salida.
	TCNT1 = 0; // Reiniciamos el contador inicial (por siacaso)
	ICR1 = 19999; // Configuramos el periodo de la señal (el TOP de nuestra PWM)
	TCCR1A =  (1 << COM1A1) | (0 << COM1A0) ; // Ponemos a 'bajo' el OCR1A cuando coincida el Compare Match
	TCCR1A |=  (1 << WGM11) | (0 << WGM10) ; // Fast PWM: TOP: ICR1
	TCCR1B = (1 << WGM13) | (1 << WGM12); // // Fast PWM: TOP: ICR1
	TCCR1B |= (0 << CS12) | (1 << CS11) | ( 0 << CS10 ); // Preesc = 8
	OCR1A=1300;//posicion 0 grados
	
}

void servoAngle(uint8_t degree){
	OCR1A=((2000/90)*degree)+1300;//valor minimo 1300 =0 grados valor maximo 5300
}