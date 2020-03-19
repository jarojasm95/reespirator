  
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
int acceleracion=DEFAULT_ACCELERACION * microStepper;      
float tCiclo, tIns, tEsp;


//pines en pinout.h
AccelStepper stepper(AccelStepper::DRIVER, DIRpin, PULpin); //direction Digital 6 (CW), pulses Digital 7 (CLK)

boolean modo = true, errorFC = false;

//encoder
Encoder encoder1(DTpin, CLKpin, SWpin);

//pantalla
Pantalla pantalla1=Pantalla();




void setup() {
  //Iniciamos serial
  Serial.begin (9600);
  Serial.println ("Inicio");
  //Parte pantalla
  pantalla1.begin();
  Serial.println ("PANTALLA ESCRITA");
  
  //Parte motor
  digitalWrite(ENpin, LOW);
  Serial.begin(9600);  // Debugging only
  pinMode(BUZZpin, OUTPUT);
  pinMode(ENDSTOPpin, INPUT);   //el sensor de efecto hall da un 1 cuando detecta
  pinMode(ENpin,OUTPUT); //test zumbador
  digitalWrite(BUZZpin, HIGH);
  delay(100);
  digitalWrite(BUZZpin, LOW);
  Serial.println("Setup");
  stepper.setAcceleration(acceleracion);
  digitalWrite(ENpin, HIGH); //habilita el motor

  tCiclo=60/rpm; //Tiempo de ciclo en segundos
  tIns=(tCiclo*porcentajeInspiratorio)/100;
  tEsp=tCiclo-tIns;
  
  velocidadUno=(pasosPorRevolucion * microStepper/2)/tIns;
  
  velocidadDos=(pasosPorRevolucion * microStepper/2)/tEsp;

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
  
  velocidadUno=(pasosPorRevolucion * microStepper/2)/tIns;
  
  velocidadDos=(pasosPorRevolucion * microStepper/2)/tEsp;
  

  if(!stepper.isRunning() && modo && !errorFC ) // Primera mitad del ciclo
    {
    Serial.println("Modo 1");
    stepper.setMaxSpeed(velocidadUno * microStepper);
    Serial.println (velocidadUno);
    Serial.println (pasosPorRevolucion * microStepper/2);
    stepper.move(pasosPorRevolucion * microStepper/2);
    modo = !modo;
    }
    
  if(!stepper.isRunning() && !modo && !errorFC) // segunda mitad del ciclo
    {
    Serial.println("Modo 2, verificar el final de carrera");
        
    if (digitalRead(ENDSTOPpin) )              //se ha llegado al final de carrera en el momento que toca pensar que esta defino como pullup
        {
        Serial.println (velocidadDos);  
        Serial.println (pasosPorRevolucion * microStepper/2);  
        stepper.setMaxSpeed(velocidadDos * microStepper);
        stepper.move(pasosPorRevolucion * microStepper/2);  
        modo = !modo;
        }

       else 
        {     
        errorFC=true;
        Serial.println ("ZUMBA");
        stepper.move(1 * microStepper);
        digitalWrite(BUZZpin, true);
        }      
    }
      

    }

     else if (!stepper.isRunning() && errorFC) //si estamos en error y ha echo los pasos extra en busca del Final de Carrera
      {
        if (!digitalRead(ENDSTOPpin))           //no se ha llegado al final suena el BUZZ y ordena dar 3 pasos en busca del FC 
          {
            errorFC=true;
            stepper.move(1); 
            digitalWrite(BUZZpin, true);
            Serial.println ("ZUMBA");
          }
         else                                    // cuando lo ha localizado ordena seguir con velocidad 2
          {
            errorFC =false;
            digitalWrite(BUZZpin, false);       //apaga el zumbador
            stepper.setMaxSpeed(velocidadDos * microStepper);
            stepper.move(pasosPorRevolucion * microStepper/2);
            modo = !modo;                       //cambiamos de velocidad
          }
      }
}
