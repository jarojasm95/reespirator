/*
 * UPDATE 18/03/2020 1:39
 * Librerías guardadas en local, no necesaria su descarga
 */


#include "Encoder.h"
#include "LiquidCrystal_I2C.h"
#include "AccelStepper.h"

//Dirección de LCD I2C
#define DIR 0x3F

//Rotary encoder
#define CLKpin 2
#define DTpin 3
#define SWpin  9

//Stepper driver
#define PULpin 6
#define DIRpin 7
#define ENpin 8

//Buzzer
#define BUZZpin 11

//Sensor hall
#define ENDSTOPpin 5

//Crer el objeto lcd
LiquidCrystal_I2C lcd(DIR, 16, 2);


//Crea el objeto Encoder Rotativo
Encoder encoder1(DTpin, CLKpin, SWpin);

 //Variable donde almacenamos el return de leerEncoder
byte tecla = 0;

//Variables a controlar
byte rpm = 15;
float vol = 0.5;

//Variable para el menu
int posMenu = 0;
bool editandoMenu = false;

//Constantes motor
#define pasosPorRevolucion 200 //Suponiendo un motor de 200 pasos/rev sin microstepper
#define velocidadUno 75       //es un ejemplo
#define velocidadDos 150      //es un ejemplo
#define acceleracion 60       //es un ejemplo


//mis pines son diferentes por ahora!!
AccelStepper stepper(1, DIRpin, PULpin); //direction Digital 6 (CW), pulses Digital 7 (CLK)

boolean modo = true, errorFC = true;



void setup() {
  //Iniciamos serial
  Serial.begin (9600);
  
  //Parte pantalla
  inicializarPantalla();
  escribirPantalla(rpm, vol, posMenu, 0);

  //Parte motor
   disableMotor();
  Serial.begin(9600);  // Debugging only
  pinMode(BUZZpin, OUTPUT);
  pinMode(ENDSTOPpin, INPUT_PULLUP);
  pinMode(ENpin,OUTPUT); //test zumbador
  digitalWrite(BUZZpin, HIGH);
  delay(100);
  digitalWrite(BUZZpin, LOW);
  Serial.println("Setup");
  stepper.setAcceleration(acceleracion);
  enableMotor();

  
  
}


void loop() {
//Parte menu
  actualizarMenu();

//Parte stepper
   stepper.run();

  if(!stepper.isRunning() && !errorFC) //si ha teminado media vuelta
    {
      if (modo)                         //velociad 1
        {
        Serial.println("Modo 1");
        stepper.setMaxSpeed(velocidadUno);
        stepper.move(pasosPorRevolucion/2);
        }
      else                                //velociadad 2
        {
        Serial.println("Modo 2, verificar el final de carrera");

        if (digitalRead(ENDSTOPpin) )//no se ha llegado al final
          {
            errorFC=true;
            stepper.move(100);
            digitalWrite(BUZZpin, true);
          }

       else 
          {          
          stepper.setMaxSpeed(velocidadDos);
          stepper.move(pasosPorRevolucion/2);  
          }      
        }
      modo = !modo;

    }

     else if (!stepper.isRunning() && errorFC) //si hay un error pero ha echo los 100 pasos extra en busca del Final de Carrera
      {
        if (digitalRead(ENDSTOPpin)) //no se ha llegado al final
          {
            errorFC=true;
            stepper.move(50);
            digitalWrite(BUZZpin, true);
          }
         else 
          {
            errorFC =false;
            digitalWrite(BUZZpin, false);
            stepper.setMaxSpeed(velocidadDos);
            stepper.move(pasosPorRevolucion/2);
          }
      }
}
