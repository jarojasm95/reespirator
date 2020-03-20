#include "defaults.h"
#include "pinout.h"
#include "Pantalla.h"
#include "Encoder.h"

#include "src/AccelStepper/AccelStepper.h"

// Variables globales para la pantalla -> globals.h

int rpm = DEFAULT_RPM;
float vol = DEFAULT_VOL;
int porcentajeInspiratorio = DEFAULT_POR_INSPIRATORIO;
int estatura = DEFAULT_ESTATURA;
int sexo = DEFAULT_SEXO; // 0: varón, 1: mujer
float volumenTidal;

float velocidadUno = 0.0; // se calcula el valor de inicio en el setup
float velocidadDos = 0.0; // idem
int microStepper = DEFAULT_MICROSTEPPER;
int aceleracion = DEFAULT_ACELERACION * microStepper;
int pasosPorRevolucion = DEFAULT_PASOS_POR_REVOLUCION;
bool tieneTrigger;
float flujoTrigger = DEFAULT_FLUJO_TRIGGER;
float tCiclo, tIns, tEsp;

// pines en pinout.h
AccelStepper stepper(AccelStepper::DRIVER, DIRpin, PULpin); // direction Digital 6 (CW), pulses Digital 7 (CLK)

boolean modo = true, errorFC = false;

// encoder
Encoder encoder1(DTpin, CLKpin, SWpin);

// pantalla
Pantalla pantalla1 = Pantalla();

/**
 * @brief estima el volumen tidal en función de estatura y sexo, en ml.
 *
 * @param estatura en cm, del paciente
 * @param sexo 0: varón, 1: mujer, sexo del paciente
 * @return *volumenTidal volumen tidal estimado, en mililitros
 */
void calcularVolumenTidal(float* volumenTidal, int estatura, int sexo) {
  float peso0, pesoIdeal, volumenEstimado;
  if (sexo == 0) { // Varón
    peso0 = 50.0;
  } else if (sexo == 1) { // Mujer
    peso0 = 45.5;
  }
  pesoIdeal = peso0 + 0.91 * (estatura - 152.4); // en kg

  *volumenTidal = pesoIdeal * DEFAULT_ML_POR_KG_DE_PESO_IDEAL;
}

/**
 * @brief calcula los tiempos de ciclo, inspiratorio y espiratorio, en seg.
 *
 * Calcula a partir de las respiraciones por minuto, los tiempos de ciclo,
 * inspiratorio y espiratorio, y las velocidades uno y dos.
 * @param velocidadUno 
 * @param velocidadDos 
 * @param tIns 
 * @param tEsp 
 * @param tCiclo 
 * @param pasosPorRevolucion 
 * @param microStepper 
 * @param porcentajeInspiratorio 
 * @param rpm 
 */
void calcularCicloInspiratorio(float* velocidadUno, float* velocidadDos,
                               float* tIns, float* tEsp, float* tCiclo,
                               int pasosPorRevolucion, int microStepper,
                               int porcentajeInspiratorio, int rpm) {
  *tCiclo = 60 / rpm; // Tiempo de ciclo en segundos
  *tIns = *tCiclo * porcentajeInspiratorio/100;
  *tEsp = *tCiclo - *tIns;

  *velocidadUno = (pasosPorRevolucion * microStepper / 2) / *tIns; // TODO: unidades?
  *velocidadDos = (pasosPorRevolucion * microStepper / 2) / *tEsp; // TODO: unidades?
}

void setup()
{
  // INICIALIZACION
  // =========================================================================

  // Puerto serie
  Serial.begin(9600);
  Serial.println("Inicio");

  // Pantalla de inicio
  pantalla1.writeLine(0, "REESPIRATOR");

  // Zumbador
  pinMode(BUZZpin, OUTPUT);
  digitalWrite(BUZZpin, HIGH); // test zumbador
  delay(100);
  digitalWrite(BUZZpin, LOW);

  // FC efecto hall
  pinMode(ENDSTOPpin, INPUT); // el sensor de efecto hall da un 1 cuando detecta

  // Parte motor
  pinMode(ENpin, OUTPUT);
  digitalWrite(ENpin, LOW);

  Serial.println("Setup");
  stepper.setAcceleration(aceleracion);

  // deja la pantalla en blanco
  delay(1000);
  pantalla1.clear();
  delay(100);


  // INTERACCIÓN: ESTATURA
  // =========================================================================
  pantalla1.writeLine(0, "Introduce estatura");
  while(encoder1.leerPulsador() == false) {
    encoder1.actualizarValor(&estatura);
    pantalla1.writeLine(1, "Altura: " + String(estatura) + " cm");
  }
  pantalla1.writeLine(0, "Valor guardado");
  pantalla1.writeLine(1, "Altura: " + String(estatura) + " cm");
  Serial.print("Altura (cm): ");
  Serial.println(estatura);
  delay(1000);


  // INTERACCIÓN: SEXO
  // =========================================================================
  pantalla1.writeLine(0, "Introduce sexo");
  while(encoder1.leerPulsador() == false) {
    encoder1.permutarValor(&sexo);
    if (sexo == 0) {
      pantalla1.writeLine(1, "Sexo: varón");
    } else if (sexo == 1) {
      pantalla1.writeLine(1, "Sexo: mujer");
    }
  }
  pantalla1.writeLine(0, "Sexo seleccionado");
  if (sexo == 0) {
    pantalla1.writeLine(1, "Sexo: varón");
  } else if (sexo == 1) {
    pantalla1.writeLine(1, "Sexo: mujer");
  }
  Serial.print("Sexo (0:V, 1:M): ");
  Serial.println(sexo);
  delay(1000);


  // ESTIMACIÓN: VOLUMEN TIDAL
  // =========================================================================
  pantalla1.writeLine(0, "Volumen tidal");
  // TODO: calcular volumen tidal estimado en función de la estatura
  calcularVolumenTidal(&volumenTidal, estatura, sexo);
  pantalla1.writeLine(1, String(volumenTidal) + " ml");
  Serial.print("Volumen tidal estimado (ml): ");
  Serial.println(volumenTidal);
  delay(2000);


  // INTERACCIÓN: VOLUMEN TIDAL
  // =========================================================================
  pantalla1.writeLine(0, "Modifica volumen");
  while(encoder1.leerPulsador() == false) {
    encoder1.actualizarValor(&volumenTidal);
    volumenTidal = constrain(volumenTidal, DEFAULT_MIN_VOLUMEN_TIDAL, DEFAULT_MAX_VOLUMEN_TIDAL);
    pantalla1.writeLine(1, String(volumenTidal) + " ml");
  }
  pantalla1.writeLine(0, "Valor guardado");
  pantalla1.writeLine(1, String(volumenTidal) + " ml");
  Serial.print("Volumen tidal configurado (ml): ");
  Serial.println(volumenTidal);
  delay(1000);


  // INTERACCIÓN: TRIGGER SI/NO
  // =========================================================================
  pantalla1.writeLine(0, "Trigger?");
  while(encoder1.leerPulsador() == false) {
    encoder1.permutarValor(&tieneTrigger);
    if (tieneTrigger) {
      pantalla1.writeLine(1, "Sí");
    } else {
      pantalla1.writeLine(1, "No");
    }
  }
  pantalla1.writeLine(0, "Valor guardado");
  if (tieneTrigger) {
    pantalla1.writeLine(1, "Trigger: Si");
  } else {
    pantalla1.writeLine(1, "Trigger: No");
  }
  Serial.print("Trigger (0:No, 1:Sí): ");
  Serial.println(tieneTrigger);
  delay(1000);


  // INTERACCIÓN: VALOR DEL TRIGGER
  // =========================================================================
  if (tieneTrigger) {
    pantalla1.writeLine(0, "Modifica trigger");
    while(encoder1.leerPulsador() == false) {
      encoder1.actualizarValor(&flujoTrigger, 0.1);
      pantalla1.writeLine(1, "Flujo: " + String(flujoTrigger) + " LPM");
    }
    pantalla1.writeLine(0, "Valor guardado");
    pantalla1.writeLine(1, "Flujo: " + String(flujoTrigger) + " LPM");
    Serial.print("Flujo trigger (LPM): ");
    Serial.println(flujoTrigger);
    delay(1000);
  }


  // INTERACCIÓN: FRECUENCIA RESPIRATORIA
  // =========================================================================
  pantalla1.writeLine(0, "Frecuencia resp.");
  while(encoder1.leerPulsador() == false) {
    encoder1.actualizarValor(&rpm);
    rpm = constrain(rpm, DEFAULT_MIN_RPM, DEFAULT_MAX_RPM);
    pantalla1.writeLine(1, String(rpm) + " rpm");
  }
  pantalla1.writeLine(0, "Valor guardado");
  pantalla1.writeLine(1, String(rpm) + " rpm");
  Serial.print("Frecuencia respiratoria (rpm): ");
  Serial.println(rpm);
  delay(1000);


  // CÁLCULO: CONSTANTES DE TIEMPO INSPIRACION/ESPIRACION
  // =========================================================================
  pantalla1.writeLine(0, "Tins | Texp (seg)");
  calcularCicloInspiratorio(&velocidadUno, &velocidadDos, &tIns, &tEsp,
                            &tCiclo, pasosPorRevolucion, microStepper,
                            porcentajeInspiratorio, rpm);
  pantalla1.writeLine(1, String(tIns) + " s" + String(tEsp) + " s");
  Serial.print("Tiempo del ciclo (seg):");
  Serial.println(tCiclo);
  Serial.print("Tiempo inspiratorio (seg):");
  Serial.println(tIns);
  Serial.print("Tiempo espiratorio (seg):");
  Serial.println(tEsp);
  Serial.print("Velocidad 1 calculada:");
  Serial.println(velocidadUno);
  Serial.print("Velocidad 2 calculada:");
  Serial.println(velocidadDos);
  delay(2000);

  // TODO: Mostrar todos los parametros

  // INTERACCIÓN: ARRANQUE
  // =========================================================================
  pantalla1.writeLine(0, "Pulsa para iniciar");
  pantalla1.writeLine(1, "Esperando...");
  while(encoder1.leerPulsador() == false);

  // Habilita el motor
  digitalWrite(ENpin, HIGH);
}


void loop()
{


  // TODO: Mostrar todos los parametros

  // TODO: Escuchar entrada desde el encoder
  // si hay nueva configuración: cambiar parámetros


  // TODO: chequear trigger
  // si hay trigger, esperar al flujo umbral para actuar, si no, actuar en cada bucle


  // Si está en inspiración: controlar con PID el volumen tidal (el que se insufla)

  // Si está en expiración: soltar balón (mover leva hacia arriba sin controlar) y esperar

  // Parte menu
  pantalla1.update(encoder1.leerEncoder());

  // Parte stepper
  stepper.run();

  //recalcular valores por si han cambiado en el menu
  // TODO: sustituir por nueva funcion: calcularConstantes();

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
