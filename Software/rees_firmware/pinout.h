#ifndef PINOUT_H
#define PINOUT_H

#include "Arduino.h"

//Display

#define LCD_ROWS 4
#define LCD_COLS 20

#ifdef I2C  //i2c

//D irección de LCD I2C
#define I2C_DIR 0x27

#else

// pines Display parallel  // parallel
/*#define LCD_RS A0    - Línea obsoleta
#define LCD_E  A1    - Línea obsoleta
#define LCD_D4 A2    - Línea obsoleta
#define LCD_D5 A3    - Línea obsoleta
#define LCD_D6 A4    - Línea obsoleta
#define LCD_D7 A5    - Línea obsoleta*/

#endif // I2C

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
