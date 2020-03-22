#ifndef PANTALLA_H
#define PANTALLA_H

#include "pinout.h"
#include "defaults.h"
#include "src\LiquidCrystal_I2C\LiquidCrystal_I2C.h"

class Display
{
public:
  Display();
  void writeLine(int line, String message = "", int offsetLeft = 0);
  void clear();
  void init();

private:
        LiquidCrystal_I2C lcd = LiquidCrystal_I2C(I2C_DIR, 20, 4);
};

#endif // PANTALLA_H
