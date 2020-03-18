/*
 * UPDATE 18/03/2020 1:39
 * Librerías guardadas en local, no necesaria su descarga
 */

#include "pantalla.h"
#include "pinout.h"
#include "defaults.h"


#include "AccelStepper.h"













//Constantes motor
#define pasosPorRevolucion 200 //Suponiendo un motor de 200 pasos/rev sin microstepper
int velocidadUno=0;       //se calcula el valor de inicio en el setup
int velocidadDos=0;       //idem
int acceleracion=6000;      //6000 para que no se note en el tiempo de ciclo 
float tCiclo, tIns, tEsp;


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

//  tCiclo=60/rpm; //Tiempo de ciclo en segundos
//  tIns=tCiclo*(porcentajeInspiratorio/100);
//  tEsp=tCiclo-tCiclo
//  
//  velocidadUno=(pasosPorRevolucion/2)/tIns
//  
//  velocidadUno=(pasosPorRevolucion/2)/tEsp
  
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
        if (digitalRead(ENDSTOPpin)) //no se ha llegado al final suena el BUZZ y ordena dar 3 pasos en busca del FC 
          {
            errorFC=true;
            stepper.move(3); 
            digitalWrite(BUZZpin, true);
          }
         else                           // cuando lo ha localizado ordena seguir con velocidad 2
          {
            errorFC =false;
            digitalWrite(BUZZpin, false);
            stepper.setMaxSpeed(velocidadDos);
            stepper.move(pasosPorRevolucion/2);
          }
      }
}
