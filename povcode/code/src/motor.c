#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <wiringPi.h>
#include <pthread.h>
#include <softPwm.h>
#include <sys/time.h>

#include "motor.h"
#include "static_pinout.h"
#include "utils.h"

#define PULSE_SAMPLING_MS  250
#define MAX_PWM           1024

// Local function declaration
void* motorCtrlThrFunc (void* null_ptr);

int pulseCounter; //declared currentPulseCounter (does it need to be a volatile int?)
pthread_t motorCtrlThr;
static int init_motors_done = 0;
const float rpm = 1500.0; //changed
int count;
float dutyCycle;


// ISR function for the encoder counter
void counterA_ISR (void)
{
    ++pulseCounter;
}

// Thread function to calculate RPM
void* motorCtrlThrFunc (void* null_ptr)
{
	int value = 0;
	float KP = 1.0;
	int expected_cnt = (int)(rpm * 211.2 * (float)PULSE_SAMPLING_MS / (4000.0 * 60.0));

	while(1) {
        pulseCounter = 0;
        delay(PULSE_SAMPLING_MS);
        count = pulseCounter;
        value += (int)((float)(expected_cnt - count) * KP);
        value = value > MAX_PWM ? MAX_PWM : (value < 0 ? 0 : value);  // Bounds 0 <= value <= MAX_PWM
        softPwmWrite(OUT_PWM, value);
    }

}


// init_motor: initializes motor pin (GPIO allocation and initial values of output)
// and initialize the elements of all motor control data structure
int initMotor(void)
{
    pulseCounter = 0;
    count = 0;

    // Initialize GPIO pins
    pinMode(IN_MT_ENC_A, INPUT);
    pinMode(IN_MT_ENC_B, INPUT);
    pinMode(OUT_MT_EN, OUTPUT);
    pinMode(OUT_PWM, OUTPUT);
    digitalWrite(OUT_MT_EN,  0);
    digitalWrite(OUT_MT_DIR, 1);
	
	if( softPwmCreate(OUT_PWM, 0, MAX_PWM) )  //Create a SW PWM, value from 0 to MAX_PWM (=100% duty cycle)
    {
        fprintf(stderr,"Error creating software PWM: %s\n", strerror(errno));
        fflush(stderr);
		return -1;
    }
	
    if(!init_motors_done)
    {
        // Start counter ISR and RPM calculator
        wiringPiISR(IN_MT_ENC_A, INT_EDGE_FALLING, &counterA_ISR);
        int ret = pthread_create( &(motorCtrlThr), NULL, motorCtrlThrFunc, NULL);
        if( ret )
        {
            fprintf(stderr,"Error creating motorCtrlThr - pthread_create() return code: %d\n",ret);
            fflush(stderr);
            return ret;
        }
    }
    init_motors_done = 1;
    return 0;
}

// stopMotor: stop the motor
void stopMotor(void)
{
    softPwmWrite(OUT_PWM, 0); // soft pwm from pwmWrite
    digitalWrite(OUT_PWM, 0);
}


// getCount: accessor funtion of a motor encoder counter
int getCount(void)
{
    return count;
}


// getRPM: accessor funtion of a motor encoder counter
int getRPM(void)
{
    return (int)(((float)count * 60.0 * 4000.0)/(211.2*(float)PULSE_SAMPLING_MS));
}


// setDutyCycle: sets the motor PWM duty-cycle
void setDutyCycle(int dutyCycle)
{
    int dc;
    if(dutyCycle <= 0)
    {
        stopMotor();
    }
    else
    {
        dc = dutyCycle <= 100 ? dutyCycle : 100;
        digitalWrite(OUT_MT_EN, 1);
        softPwmWrite(OUT_PWM, (dc * 1024)/100);
    }
}
