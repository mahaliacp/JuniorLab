#include <stdio.h> //printf, fprintf, stderr, fflush, scanf, getchar 
#include <string.h> //strncpy, strerror include <errno.h> //errno 
#include <stdlib.h> //exit, EXIT_SUCCESS, EXIT_FAILURE 
#include <signal.h> //signal, SIGINT, SIGQUIT, SIGTERM 
#include <wiringPi.h> //wiringPiSetup, pinMode, delay, INPUT, OUTPUT, PWM_OUTPUT


int main(){
	wiringPiSetup();

	system("gpio mode 23 ALT0");
	system("gpio mode 26 ALT5");

	pinMode(4, OUTPUT);
	pinMode(5, OUTPUT);
	pinMode(27, OUTPUT);
	digitalWrite(4,0); 
	digitalWrite(5,0); 
	digitalWrite(27,1); 

	printf("Playing..."); fflush(stdout);
	system("omxplayer -o local --vol=-2000 bigbenstrike.mp3 > /dev/null 2>&1");
	printf(" done!\n");
}
