#include "defaults.h"
#include "pinout.h"
#include "Pantalla.h"
#include "Encoder.h"

#include "src/AccelStepper/AccelStepper.h"

// Variables globales para la pantalla -> globals.h

int rpm = DEFAULT_RPM;
float vol = DEFAULT_VOL;
int porcentajeInspiratorio = DEFAULT_POR_INSPIRATORIO;

float velocidadUno = 0.0; // se calcula el valor de inicio en el setup
float velocidadDos = 0.0; // idem
int microStepper = DEFAULT_MICROSTEPPER;
int aceleracion = DEFAULT_ACELERACION * microStepper;
int pasosPorRevolucion = DEFAULT_PASOS_POR_REVOLUCION;
float tCiclo, tIns, tEsp;

// pines en pinout.h
AccelStepper stepper(AccelStepper::DRIVER, DIRpin, PULpin); // direction Digital 6 (CW), pulses Digital 7 (CLK)

boolean modo = true, errorFC = false;

// encoder
Encoder encoder1(DTpin, CLKpin, SWpin);

// pantalla
Pantalla pantalla1 = Pantalla();

void calcularConstantes()
{
  tCiclo = 60 / rpm; //Tiempo de ciclo en segundos
  tIns = (tCiclo * porcentajeInspiratorio) / 100;
  tEsp = tCiclo - tIns;

  velocidadUno = (pasosPorRevolucion * microStepper / 2) / tIns;

  velocidadDos = (pasosPorRevolucion * microStepper / 2) / tEsp;
}

void setup()
{
  //Iniciamos serial
  Serial.begin(9600);
  Serial.println("Inicio");
  //Parte pantalla
  pantalla1.begin();

  //Zumbador
  pinMode(BUZZpin, OUTPUT);
  digitalWrite(BUZZpin, HIGH); //test zumbador
  delay(100);
  digitalWrite(BUZZpin, LOW);

  //FC efecto hall
  pinMode(ENDSTOPpin, INPUT); //el sensor de efecto hall da un 1 cuando detecta

  //Parte motor
  pinMode(ENpin, OUTPUT);
  digitalWrite(ENpin, LOW);

  Serial.println("Setup");
  stepper.setAcceleration(aceleracion);

  calcularConstantes(); //calcular las constantes de movimiento

  velocidadDos = (pasosPorRevolucion / 2) / tEsp;

  Serial.println(tCiclo);
  Serial.println(tIns);
  Serial.println(tEsp);
  Serial.println(".....");
  Serial.println(velocidadUno);
  Serial.println(velocidadDos);

  digitalWrite(ENpin, HIGH); //habilita el motor
}



void loop()
{
  // Parte menu
  pantalla1.update(encoder1.leerEncoder());

  // Parte stepper
  stepper.run();

  //recalcular valores por si han cambiado en el menu
  calcularConstantes();

  if (!stepper.isRunning() && modo && !errorFC) // Primera mitad del ciclo
  {
    Serial.println("Modo 1");
    stepper.setMaxSpeed(velocidadUno * microStepper);

    stepper.move(pasosPorRevolucion * microStepper / 2);
    modo = !modo;
  }

  if (!stepper.isRunning() && !modo && !errorFC) // segunda mitad del ciclo
  {
    Serial.println("Modo 2");

    if (digitalRead(ENDSTOPpin)) //se ha llegado al final de carrera en el momento que toca pensar que esta defino como pullup
    {
      Serial.println("Final de carrera OK");
      stepper.setMaxSpeed(velocidadDos * microStepper);
      stepper.move(pasosPorRevolucion * microStepper / 2);
      modo = !modo;
    }

    else // si acabada la segunda parte del movimiento no se ha llegado al SENSOR HALL entonces da un paso y vuelve a hacer la comprovocacion
    {
      Serial.println("Final de carrera NO DETECTADO: Buscando FC");
      errorFC = true;
      digitalWrite(BUZZpin, true); //activa el zumbador
      stepper.move(1 * microStepper);
    }
  }

  if (!stepper.isRunning() && errorFC) //si estamos en error y ha echo los pasos extra en busca del Final de Carrera
  {
    if (!digitalRead(ENDSTOPpin)) //no se ha llegado al final suena el BUZZ y ordena dar 3 pasos en busca del FC
    {
      Serial.println("--Buscando FC");
      errorFC = true;
      stepper.move(1);
    }
    else // cuando lo ha localizado ordena seguir con velocidad 2
    {
      Serial.println("Detectado FC: restableciendo el origen");
      errorFC = false;
      digitalWrite(BUZZpin, false); //apaga el zumbador
      stepper.setMaxSpeed(velocidadDos * microStepper);
      stepper.move(pasosPorRevolucion * microStepper / 2);
      modo = !modo; //cambiamos de velocidad
    }
  }


else if (!stepper.isRunning() && errorFC) // si hay un error pero ha hecho los 100 pasos extra en busca del Final de Carrera
  {
    if (digitalRead(ENDSTOPpin)) // no se ha llegado al final suena el BUZZ y ordena dar 3 pasos en busca del FC
    {
      errorFC = true;
      stepper.move(1);
      digitalWrite(BUZZpin, true);
      Serial.println("ZUMBA");
    }
    else // cuando lo ha localizado ordena seguir con velocidad 2
    {
      errorFC = false;
      digitalWrite(BUZZpin, false);
      stepper.setMaxSpeed(velocidadDos);
      stepper.move(pasosPorRevolucion / 2);
    }
  }
}
