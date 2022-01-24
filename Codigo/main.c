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
long distancia;
char* dis;
bool onDuty=false;

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
		distancia=getDistance();
		_delay_us(100);
	
		sprintf(ang,"%d",angulo);
		sprintf(dis,"%lld",distancia);
		
		UART_write_txt(dis);
		UART_write_txt(",");
		UART_write_txt(ang);
		UART_write_txt(".\r\n");
		
	
		TCNT0=12;}
}

int main(void)
{
	
	/*ENVIAR DATOS DE MANERA SERIAL*/
	UART_init();

	/*INICIALIZAR LA PARTE DEL SENSOR ULTRASONICO Y EL SERVO */
	servoInit();
	ultraSonicoInit();
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
				lcd_puts("   TRABAJANDO");
				lcd_gotoxy(0,1);
				lcd_puts("POR FAVOR ESPERE");
				UART_write();
				UART_write_txt();
				UART_write_txt();
			}
		
		
		
		
		
        //TODO:: Please write your application code 
    }
}