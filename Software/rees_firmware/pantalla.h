#ifndef PANTALLA_H
#define PANTALLA_H


#ifdef I2C
  #include "LiquidCrystal_I2C.h"  
#else
  #include "LiquidCrystal.h"
#endif


#include "pinout.h"
#include "defaults.h"
#include "globals.h"

class Pantalla {
  public:
    Pantalla (); //init
    void begin();
    void write(int p_rpm, float p_vol, int _posMenu, int caracter);
    void update(int tecla);
    bool editando();
  private:
   #ifdef I2C
    LiquidCrystal_I2C lcd = LiquidCrystal_I2C(I2C_DIR, LCD_COLS, LCD_ROWS);
   #else
    LiquidCrystal lcd = LiquidCrystal(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
   #endif
    int _posMenu = 0;
    bool _editandoMenu = false;
};

#endif // PANTALLA_H
