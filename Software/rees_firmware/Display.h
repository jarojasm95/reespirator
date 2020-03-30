#ifndef PANTALLA_H
#define PANTALLA_H

#include <LiquidCrystal_I2C.h>

#include "pinout.h"
#include "defaults.h"

class Display
{
public:
  Display();
  Display(bool init);
  void writeLine(int line, String message = "", int offsetLeft = 0);
  void clear();

private:
  LiquidCrystal_I2C *lcd;
};

#endif // PANTALLA_H
