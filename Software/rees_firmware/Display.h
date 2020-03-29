#ifndef PANTALLA_H
#define PANTALLA_H

#include "src/LiquidCrystal_I2C/LiquidCrystal_I2C.h"

#include "pinout.h"
#include "defaults.h"

class Display
{
public:
  Display();
  void writeLine(int line, String message = "", int offsetLeft = 0);
  void clear();

};

#endif // PANTALLA_H
