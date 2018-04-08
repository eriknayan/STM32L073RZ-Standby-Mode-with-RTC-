#include "mbed.h"
#include "standby.h"

DigitalOut led(LED1);

int main(){
	
		debug("STM32 L073RZ\n");
		led = 1;
		wait_ms(1000);
		standby_mode(60);
}
