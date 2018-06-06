/*light test*/


#include <wiringPi.h>
#include <signal.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "web_client.h"

int valueIR;
int value_table[120];	
	
void* communicationThrFunc (void* null_ptr);

// set up the ISr
void lightISR (void)
	{
		valueIR = 64;
	}

void control_event(int sig);

int main (int argc, char *argv[])
{
    // Inform OS that control_event() function will be handling kill signals
    (void)signal(SIGINT, control_event);
    (void)signal(SIGQUIT, control_event);
    (void)signal(SIGTERM, control_event);

	char server_ip[20];
	char my_ip[20];
	const int pin_table[] = {28, 27, 26, 24, 23, 22, 21, 16, 15,
	 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
	pthread_t communicationThr;

	// Parse 1st argument: server IP
    if(argc < 2)
    {
        printf("Enter the server IP: > ");
        fflush(stdout);
        scanf(" %s", server_ip);
        getchar();
        fflush(stdin);
    }
    else
    {
        strncpy(server_ip, argv[1], sizeof server_ip - 1);
    }
    printf("  server IP     : %s\n", server_ip);
	sprintf(my_ip, "%s", getMyIP("wlan0"));
    printf("  My wireless IP is: %s\n", my_ip);

	
    // Initialize the Wiring Pi facility
	printf("Initialize Wiring Pi facility... ");
    if (wiringPiSetup() != 0)
    {
        // Handles error Wiring Pi initialization
        fprintf(stderr, "Unable to setup wiringPi: %s\n", strerror(errno));
        fflush(stderr);
        exit(EXIT_FAILURE);
    }
	printf("Done\n");

	// set up the pins
	printf("Initialize GPIO pins... ");
	for(int j = 0; j < 24; j++)
	{
		pinMode(pin_table[j], OUTPUT);
	}
	printf("Done\n");
	
	// Start web client
	printf("Initialize web client... ");
    initWebClient(server_ip);;
	printf("Done\n");
	
	// set up thr variables, call the ISR
	//int lengthValT = sizeof value_table/ sizeof value_table[0];
	valueIR = 0;

	wiringPiISR(29, INT_EDGE_RISING, &lightISR);
	
	for(int i=0; i<120 ; i++)
	{	
		value_table[i] = 0;
	}

	int ret = pthread_create( &(communicationThr), NULL, communicationThrFunc, NULL);
    if( ret )
    {
        fprintf(stderr,"Error creating communicationThr - pthread_create() return code: %d\n",ret);
        fflush(stderr);
        return ret;
    }


	while(1)
	{
		for (int ti = 0; ti < 24; ti++)
		{
			digitalWrite(pin_table[ti], value_table[valueIR] & (1 << ti)); //>> ti);
			//printf("writing: %i \n", ti);
			
		}
		delayMicroseconds(243); // we have tried 244, 200, 300, 333, and values in between.
		valueIR++;
		valueIR %= 120;
	
	}


	return 0;
}


void* communicationThrFunc (void* null_ptr)
{
	char message[MESSAGE_BUFFER_SIZE];
	//char *token;
	//char delim[] = ",\n";
	
	while(1) {
		sprintf(message,"%s", getMessage());
		

	}

}



void control_event(int sig)
{
    printf("\b\b  \nExiting pov... ");

    delay(200);
    printf("Done\n");
    exit(EXIT_SUCCESS);
}
