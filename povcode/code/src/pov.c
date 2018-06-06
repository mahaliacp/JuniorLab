/**
\file       pov.c
\author     Eddy Ferre - ferree@seattleu.edu
\date       01/13/2018

\brief      Main persistence of vision file for the static RPi controllers.

This program will run the logic of the static RPi
*/

// Linux C libraries
#include <stdio.h>     //printf, fprintf, stderr, fflush, scanf, getchar
#include <string.h>    //strncpy, strerror
#include <errno.h>     //errno
#include <stdlib.h>    //exit, EXIT_SUCCESS, EXIT_FAILURE
#include <signal.h>    //signal, SIGINT, SIGQUIT, SIGTERM
#include <wiringPi.h>  //wiringPiSetup, pinMode, digitalWrite, delay, INPUT, OUTPUT, PWM_OUTPUT
#include <time.h>

// Local headers
#include "static_pinout.h"
#include "motor.h"      //initMotor
#include "web_client.h" //initWebClient_new_port

const int value_table[] = { 
0x000000, 0x480000, 0x980000, 0x680000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0xE00000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0xE00000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0xB00000, 0x400000, 0x200000, 0x400000, 0x300000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0xE00000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0xE00000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x400000, 0xA80000, 0x700000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0xE00000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0xE00000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x200000, 0x100000, 0x300000, 0x500000, 0x200000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0xE00000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0xE00000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x400000, 0xF80000, 0x000000 };


// Local function declaration
/// Function controlling exit or interrupt signals
void control_event(int sig);

/**
main function - Entry point function for pov

@param argc number of arguments passed
@param *argv IP address of the rotor RPi

@return number stdlib:EXIT_SUCCESS exit code when no error found.
*/
int main (int argc, char *argv[])
{
    // Inform OS that control_event() function will be handling kill signals
    (void)signal(SIGINT, control_event);
    (void)signal(SIGQUIT, control_event);
    (void)signal(SIGTERM, control_event);

    //Local variable definition
    char rotor_ip[20];
	char server_ip[20];
	char my_ip[20];
	int image_table[120];
	char message[MESSAGE_BUFFER_SIZE];
	
	time_t ugly_time;
	struct tm *pretty_time;

    // Parse 1st argument: rotor RPi IP
    if(argc < 2)
    {
        printf("Enter the rotor RPi IP: > ");
        fflush(stdout);
        scanf(" %s", rotor_ip);
        getchar();
        fflush(stdin);
    }
    else
    {
        strncpy(rotor_ip, argv[1], sizeof rotor_ip - 1);
    }
	
	// Parse 2nd argument: server IP
    if(argc < 3)
    {
        printf("Enter the server IP: > ");
        fflush(stdout);
        scanf(" %s", server_ip);
        getchar();
        fflush(stdin);
    }
    else
    {
        strncpy(server_ip, argv[2], sizeof server_ip - 1);
    }
	
    printf("  rotor RPi IP     : %s\n", rotor_ip);
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

    // Initialize GPIO pins
    printf("Initialize GPIO pins... ");
    pinMode(IN_SW1, INPUT);
	printf("Done\n");

    // Start encoder counter ISRs, setting actual motor RPM
	printf("Initialize Motor... ");
    //initMotor();
	printf("Done\n");

	// Start web client
	printf("Initialize web client... ");
    initWebClient(server_ip);;
	printf("Done\n");
	
    /// \todo
    /// * Communication controller (Separate Thread) -  getMessage responses
	
	
    // Display controller (Main loop)
	int min_slice_idx, min, prev_min = -1;
	int hr_slice_idx, hour;

    printf("Start Main loop - Display controller\n");
    while(1){

		time(&ugly_time);
		pretty_time = localtime(&ugly_time);
		
		min = pretty_time->tm_min;
		hour = pretty_time->tm_hour;
		
		if(prev_min != min)
		{
			min_slice_idx = 2 * min;
			hr_slice_idx = (10 * hour + min / 6) %120;

			for(int i=0; i<120 ; i++)
			{
				image_table[i] = value_table[i];
			}
			//set minute hand
			image_table[min_slice_idx] |= 0xFFFFFF;
			//set hour hand
			image_table[hr_slice_idx] |= 0xFFF;

			//<srcIP>,<destIP>,<command>[,<arguments>]<line feed character '\n'>
			sprintf(message,"%s,%s,display", my_ip, rotor_ip);
			for(int i=0; i<120 ; i++)
			{
				if(image_table[i] != 0)
				{
					sprintf(message,"%s,%d,%X", message, i, image_table[i]);
				}
			}
			sprintf(message,"%s\n", message);
			sendMessage(message);
			prev_min = min;
		}
        delay(1000);  //Always keep a sleep or delay in infinite loop
    }
    return 0;
}

void control_event(int sig)
{
    printf("\b\b  \nExiting pov... ");

    //stop the motor
    stopMotor();

    delay(200);
    printf("Done\n");
    exit(EXIT_SUCCESS);
}
