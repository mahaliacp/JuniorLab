#ifndef STATIC_PINOUT_H
#define STATIC_PINOUT_H

/**
\file       static_pinout.h
\brief      GPIO Pin file of the static RPi.

This file maps all GPIO pins used by the hardware and RPi of the static portion of the POV
*/

// GPIO Input Pins
/// Email-My-IP and utility switch "S1" (input pin 37, GPIO_25)
#define IN_SW1            25
/// Motor encoder A (input pin 22, GPIO_6)
#define IN_MT_ENC_A		   6        // changed by M. Mandich & M. Polk
/// Motor encoder B (input pin 24, GPIO_10)
#define IN_MT_ENC_B       10 // changed by M. Mandich & M. Polk

// GPIO Output Pins
#define OUT_MT_EN          5 
#define OUT_PWM            4
#define OUT_MT_DIR         7

#endif
