#include "Display.h"
#include "Arduino.h"
#include "src/LiquidCrystal_I2C/LiquidCrystal_I2C.h"
//LiquidCrystal_I2C lcd(0x27,20,4);
Display::Display() {
//#ifdef I2C
  LiquidCrystal_I2C lcd(0x27,20,4);
  lcd.begin();
  lcd.setBacklight(HIGH);
//#else
  //lcd.begin(20, 4);

  byte _Flecha[] = {
      B00000,
      B00100,
      B01100,
      B11111,
      B11111,
      B01100,
      B00100,
      B00000};

  byte _Cruz[] = {
      B00000,
      B01110,
      B01110,
      B11111,
      B11111,
      B01110,
      B01110,
      B00000};
  //Creamos el icono flecha
  lcd.createChar(0, _Flecha);
  lcd.createChar(1, _Cruz);
  //#endif
}

/**
 * @brief Escribe en una linea de la Display un mensaje.
 *
 * @param line línea donde se escribe, puede ser 0 o 1
 * @param message mensaje a escribir (max. 16 caracteres)
 * @param offsetLeft posicion inicial desde la que se empieza a escribit
 */
void Display::writeLine(int line, String message, int offsetLeft) {
  //#ifdef I2C
  lcd.setCursor(0, line);
  lcd.print("");
  lcd.setCursor(offsetLeft, line);
  lcd.print(message);
  //#endif
}

/**
 * @brief Deja la Display en blanco, sin caracteres.
 */
void Display::clear() {
  //#ifdef I2C
  lcd.clear();
  //#endif
}
