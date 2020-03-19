#include "Pantalla.h"
#include "Arduino.h"

extern int rpm;
extern float vol;
extern int porcentajeInspiratorio;

Pantalla::Pantalla()
{
#ifdef I2C
  lcd.begin();
#else
  lcd.begin(20, 4);
#endif
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
}

void Pantalla::write(int rpm, float vol, int posMenu, int caracter)
{

  // Escribimos el Mensaje en el LCD.
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Freq:    rpm");
  lcd.setCursor(6, 0);
  lcd.print(rpm);
  lcd.setCursor(0, 1);
  lcd.print("Vol:      L/r");
  lcd.setCursor(5, 1);
  lcd.print(vol);
  lcd.setCursor(15, posMenu);
  lcd.write(caracter);
}

void Pantalla::begin()
{
  Pantalla::write(rpm, vol, 0, 0);
}

void Pantalla::update(int tecla)
{
  switch (tecla)
  {
  case 2: //Giramos horario (Bajamos en el menu)
    if (_posMenu == 0 && !_editandoMenu)
    {
      _posMenu++;
      Pantalla::write(rpm, vol, _posMenu, 0);
    }
    else if (_posMenu == 0 && _editandoMenu)
    {
      rpm++;
      Pantalla::write(rpm, vol, _posMenu, 1);
    }
    else if (_posMenu == 1 && _editandoMenu)
    {
      vol = vol + 0.1;
      Pantalla::write(rpm, vol, _posMenu, 1);
    }
    break;

  case 8: //Giramos antihorario (Subimos en el menu)
    if (_posMenu == 1 && !_editandoMenu)
    {
      _posMenu--;
      Pantalla::write(rpm, vol, _posMenu, 0);
    }
    else if (_posMenu == 1 && _editandoMenu)
    {
      if (vol >= 0.1)
      {
        vol = vol - 0.1;
        Pantalla::write(rpm, vol, _posMenu, 1);
      }
    }
    else if (_posMenu == 0 && _editandoMenu)
    {
      if (rpm >= 1)
      {
        rpm--;
        Pantalla::write(rpm, vol, _posMenu, 1);
      }
    }
    break;

  case 5: //Pulsamos (Modificamos valor)
    _editandoMenu = !_editandoMenu;
    if (_editandoMenu == true)
    {
      Pantalla::write(rpm, vol, _posMenu, 1);
    }
    else
    {
      Pantalla::write(rpm, vol, _posMenu, 0);
    }
    break;

  default:
    break;
  }
}
