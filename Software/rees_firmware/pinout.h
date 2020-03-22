#ifndef PINOUT_H
#define PINOUT_H

#include "Arduino.h"

//Display

#define LCD_ROWS 2
#define LCD_COLS 16

//Dirección de LCD I2C
#define I2C_DIR 0x27

// Rotary encoder
#define CLKpin 2
#define DTpin 3
#define SWpin 9

// Stepper driver
#define PULpin 6
#define DIRpin 7
#define ENpin 8

// Buzzer
#define BUZZpin 11

// Sensor hall
#define ENDSTOPpin 5

#endif // ENCODER_H
