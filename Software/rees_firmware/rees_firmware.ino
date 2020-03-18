
  
#include "pinout.h"
#include "pantalla.h"
#include "Encoder.h"
#include "defaults.h"

#include "AccelStepper.h"

//Variables globales para la pantalla -> globals.h

int rpm = DEFAULT_RPM;
float vol = DEFAULT_VOL;
int porcentajeInspiratorio = DEFAULT_POR_INSPIRATORIO;




float velocidadUno=0;       //se calcula el valor de inicio en el setup
float velocidadDos=0;       //idem
int acceleracion=DEFAULT_ACCELERACION;      
float tCiclo, tIns, tEsp;


//pines en pinout.h
AccelStepper stepper(AccelStepper::DRIVER, DIRpin, PULpin); //direction Digital 6 (CW), pulses Digital 7 (CLK)

boolean modo = true, errorFC = false;

//encoder
Encoder encoder1(DTpin, CLKpin, SWpin);

//pantalla
Pantalla pantalla1=Pantalla();

void enableMotor ()
{
  digitalWrite(ENpin, HIGH);
}

void disableMotor ()
{
  digitalWrite(ENpin, LOW);
}


void setup() {
  //Iniciamos serial
  Serial.begin (9600);
  Serial.println ("Inicio");
  //Parte pantalla
  pantalla1.begin();
  Serial.println ("PANTALLA ESCRITA");
  
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

  tCiclo=60/rpm; //Tiempo de ciclo en segundos
  tIns=(tCiclo*porcentajeInspiratorio)/100;
  tEsp=tCiclo-tIns;
  
  velocidadUno=(pasosPorRevolucion/2)/tIns;
  
  velocidadDos=(pasosPorRevolucion/2)/tEsp;

  Serial.println (tCiclo);
  Serial.println (tIns);
  Serial.println (tEsp);
    Serial.println (".....");
  Serial.println (velocidadUno);
  Serial.println (velocidadDos);
}


void loop() {
//Parte menu
  pantalla1.update(encoder1.leerEncoder());

//Parte stepper
   stepper.run();

//recalcular valores por si han cambiado en el menu

  tCiclo=60/rpm; //Tiempo de ciclo en segundos
  tIns=(tCiclo*porcentajeInspiratorio)/100;
  tEsp=tCiclo-tIns;
  
  velocidadUno=(pasosPorRevolucion/2)/tIns;
  
  velocidadDos=(pasosPorRevolucion/2)/tEsp;
  

  if(!stepper.isRunning() && !errorFC) //si ha teminado media vuelta
    {
      if (modo)                         //velociad 1
        {
        Serial.println("Modo 1");
        stepper.setMaxSpeed(velocidadUno);
        Serial.println (velocidadUno);
        Serial.println (pasosPorRevolucion/2);
        stepper.move(pasosPorRevolucion/2);
        }
      else                                //velociadad 2
        {
        Serial.println("Modo 2, verificar el final de carrera");
        
        if (digitalRead(ENDSTOPpin) )//no se ha llegado al final
          {
            errorFC=true;
             Serial.println ("ZUMBA");
            stepper.move(1);
            digitalWrite(BUZZpin, true);
          }

       else 
          {      
          Serial.println (velocidadDos);  
           Serial.println (pasosPorRevolucion/2);  
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
            stepper.move(1); 
            digitalWrite(BUZZpin, true);
            Serial.println ("ZUMBA");
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
