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
#include "UART.h"


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
	while ((PIND>>7)&1==0);
	TCNT2=0;
	while((PIND>>5)&1);
	long distancia = TCNT2*8*0.017;
	char dis[3];
	ltoa(distancia,dis,1);
	lcd_home();
	lcd_puts(dis);
	
}
 
void servoInit(){
	/*INICIALIZAR PWM*/
	
	TCCR1A|=(1<<WGM11)|(1<<COM1A1);//PWM no invertido
	TCCR1B|=(1<<WGM12)|(1<<WGM13)|(1<<CS10);//no preescalado
	ICR1=19999;
	DDRB|=(1<<DDB1);//OCR1A PB1
	
}
void servoAngle(uint8_t degree){
	OCR1A=9.73*degree+388;
}
void interrupt_TIMER0_init(){
	 cli(); //deshabilitar momentáneamente las interrupciones
	 TCCR0B = 0b101; //preescalador 1024
	 TIMSK0 = 1; // habilitar interrupcion por desbordamiento
	 sei(); //habilitar las interrupciones
	 TCNT0 = 12; //250ms // tiempo de la interrupcion
	
}
uint8_t angulo=70;
bool direccion=true;
long distancia;
ISR(TIMER0_OVF_vect){
	if (direccion){
		servoAngle(angulo);
		angulo++;
		if(angulo==170){direccion=false;}
	}else{
		servoAngle(angulo);
		angulo--;
		if(angulo==70){direccion=true;}
	}
	getDistance();
	
	TCNT0=12;
}

int main(void)
{
	
	/*ENVIAR DATOS DE MANERA SERIAL*/
	//UART_init();
	//UART_write_txt("proyecto prubea aaaa");
	/*INICIALIZAR LA PARTE DEL SENSOR ULTRASONICO Y EL SERVO */
	servoInit();
	ultraSonicoInit();
	/*INICIALIZA LA PARTE DE LA INTERRUPCION EN TIMER 0 PARA EL MOVIMIENTO Y RECOPILACION DE DATOS*/
	interrupt_TIMER0_init();
	
	
	/*VARIABLES A ENVIAR*/
	char tipoDesecho; //Este valor puede ser (I)industrial (D)no industrial
	int cantidadDesechos;//Este valor es el peso aproximado a recoger en kilos
	
	/*VARIABLE DE CONTROL DE CARACTER DEL TECLADO*/
	char key;
	
	/*INICIALIZAR DISPLAY Y TECLADO*/
	lcd_init(LCD_DISP_ON);
	kbrd_init();
	lcd_home();
	
	/*PANTALLA DE BIENVENIDA*/
	lcd_puts("   BIENVENIDO");
	lcd_gotoxy(0,1);
	lcd_puts("  SMART CLEANER");
	_delay_ms(2000);
	lcd_clrscr();
	
	
	
	
	
    while(1)
    {
		/*PANTALLA DE PREGUNTA DE RESIDUO INDUSTRIAL*/
		lcd_puts(" RESIDUO (I/NI)");
		lcd_gotoxy(0,1);
		lcd_puts("  I:ON    NI:=");
		do{
			key=kbrd_read();
			tipoDesecho=key;
		}while (!(key!='='^key!='.'));
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
			
		}while(key!='=');
		/*PANTALLA DE ENVIO DE DATOS*/
		lcd_clrscr();
		lcd_home();
		lcd_puts("ENVIANDO DATOS..");
		_delay_ms(2000);
		lcd_clrscr();
		lcd_home();
		lcd_puts("***COMPLETADO***");
		_delay_ms(10000);
		
		
		
		
		
        //TODO:: Please write your application code 
    }
}