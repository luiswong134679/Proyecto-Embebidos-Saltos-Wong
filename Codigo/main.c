/*
 * main.c
 *
 * Created: 1/14/2022 1:09:23 AM
 *  Author: Saltos-Wong
 */
#define F_CPU 8000000UL 
#include "avr/io.h"
#include "util/delay.h"
#include "lcd.h"
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdio.h>
#include "UART.h"
/*VARIABLES GLOBALES*/
uint8_t angulo=0;
char* ang;
bool direccion=true;
char* dis;
bool onDuty=false;


void ultraSonicoInit(){
	DDRD|=(1<<DDD6);//PIN TRIGGED
	DDRD &=~(1<<DDD7);//PIN ECHO;
	PORTD &=~ (1<<6) | (1<<7);//SE INICIALIZAN EN ESTADO BAJO
	TCCR2A=0x00;
	TCCR2B |= (1<<CS11)|(1<<CS10);
	
}

void getDistance(){
	PORTD|=(1<<6);
	_delay_us(10);
	PORTD &=~(1<<6);
	while ((PIND>>7) && 1 == 0);
	TCNT2=0;
	while((PIND>>5) && 1);
	long long distancia = TCNT2*8*0.017;
	sprintf(dis,"%lld",distancia);
}
 
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

void interrupt_TIMER0_init(){
	cli(); //deshabilitar momentáneamente las interrupciones
	TCCR0B = 0b101; //preescalador 1024
	TIMSK0 = 1; // habilitar interrupcion por desbordamiento
	sei(); //habilitar las interrupciones
	TCNT0 = 12; //250ms // tiempo de la interrupcion
	
}

ISR(TIMER0_OVF_vect){
	if(onDuty){
		if (direccion){
			servoAngle(angulo);
			angulo++;
			if(angulo==180){direccion=false;}
			}else{
			servoAngle(angulo);
			angulo--;
			if(angulo==0){direccion=true;}
		}
		//getDistance();
		//_delay_us(100);
	
		//sprintf(ang,"%d",angulo);
		//UART_write_txt(dis);
		//UART_write_txt(ang);
	
		TCNT0=12;}
}

int main(void)
{
	
	/*ENVIAR DATOS DE MANERA SERIAL*/
	UART_init();

	/*INICIALIZAR LA PARTE DEL SENSOR ULTRASONICO Y EL SERVO */
	servoInit();
	ultraSonicoInit();
	getDistance();
	UART_write_txt(dis);
	/*INICIALIZA LA PARTE DE LA INTERRUPCION EN TIMER 0 PARA EL MOVIMIENTO Y RECOPILACION DE DATOS*/
	interrupt_TIMER0_init();
	
	/*VARIABLES A ENVIAR*/
	char tipoDesecho; //Este valor puede ser (I)industrial (D)no industrial
	int cantidadDesechos;//Este valor es el peso aproximado a recoger en kilos
	int numeroAuto=1;
	
	/*VARIABLE DE CONTROL DE CARACTER DEL TECLADO*/
	char key;
	
	/*INICIALIZAR DISPLAY Y TECLADO*/
	lcd_init(LCD_DISP_ON);
	kbrd_init();
	lcd_home();
	
	/*PANTALLA DE BIENVENIDA*/
	lcd_clrscr();
	lcd_home();
	lcd_puts("   BIENVENIDO");
	lcd_gotoxy(0,1);
	lcd_puts("  SMART CLEANER");
	_delay_ms(4000);
	lcd_clrscr();
	servoAngle(0);
	

	
	
	
    while(1)
    {
		if(!onDuty){
			/*PANTALLA DE PREGUNTA DE RESIDUO INDUSTRIAL*/
			lcd_puts(" RESIDUO (I/NI)");
			lcd_gotoxy(0,1);
			lcd_puts("  I:ON    NI:=");
			do{
				key=kbrd_read();
				tipoDesecho=key;
				if(key=='='){tipoDesecho='N';}
				if(key=='.'){tipoDesecho='I';}
			}while (!((key!='2') ^ (key!='.')));
			/*PANTALLA DE SELCCION DE KILOS*/
			lcd_clrscr();
			lcd_home();
			lcd_puts("CANTIDAD BASURA:");
			cantidadDesechos=1;
			do{
				if(kbrd_read()=='+' && cantidadDesechos<9){cantidadDesechos++;}
				else if (kbrd_read()=='-' && cantidadDesechos>1){cantidadDesechos--;}
				lcd_gotoxy(0,1);
				lcd_puts("     ");
				char cdch=cantidadDesechos+'0';
				lcd_putc(cdch);
				lcd_puts("KG");
				key=kbrd_read();
			
			}while(key!='2');
			/*PANTALLA DE ENVIO DE DATOS*/
			lcd_clrscr();
			lcd_home();
			lcd_puts("ENVIANDO DATOS..");
			UART_write(numeroAuto+'0');
			UART_write(cantidadDesechos+'0');
			UART_write(tipoDesecho);
			_delay_ms(2000);
			if(UCSR0A&(0<<6)){
				lcd_clrscr();
				lcd_home();
				lcd_puts("***ERROR***");
				_delay_ms(1000);
			}else{
				lcd_clrscr();
				lcd_home();
				lcd_puts("***COMPLETADO***");
				_delay_ms(1000);
				onDuty=true;	
			}
			
			}else{
				lcd_clrscr();
				lcd_home();
				lcd_puts("POR FAVOR ESPERE");
				lcd_gotoxy(0,1);
				lcd_puts("   TRABAJANDO");
				_delay_ms(11000);
			}
		
		
		
		
		
        //TODO:: Please write your application code 
    }
}