#include "defaults.h"
#include "pinout.h"
#include "Display.h"
#include "Encoder.h"

#include "src/AccelStepper/AccelStepper.h"

// Variables globales para la Display -> globals.h

int rpm                    = DEFAULT_RPM;
int porcentajeInspiratorio = DEFAULT_POR_INSPIRATORIO;
int estatura               = DEFAULT_ESTATURA;
int sexo                   = DEFAULT_SEXO; // 0: varón, 1: mujer
int microStepper           = DEFAULT_MICROSTEPPER;
int aceleracion            = DEFAULT_ACELERACION * microStepper;
int pasosPorRevolucion     = DEFAULT_PASOS_POR_REVOLUCION;
float flujoTrigger         = DEFAULT_FLUJO_TRIGGER;

bool tieneTrigger;
bool modo = true, errorFC = false;
int volumenTidal;
float velocidadUno, velocidadDos, tCiclo, tIns, tEsp;

// pines en pinout.h
AccelStepper stepper(AccelStepper::DRIVER, DIRpin, PULpin); // direction Digital 6 (CW), pulses Digital 7 (CLK)

// Encoder
Encoder encoder(DTpin, CLKpin, SWpin);

// Display
Display display = Display();

// =========================================================================
// FUNCIONES DE CÁLCULO
// =========================================================================

/**
 * @brief estima el volumen tidal en función de estatura y sexo, en ml.
 *
 * @param estatura en cm, del paciente
 * @param sexo 0: varón, 1: mujer, sexo del paciente
 * @return *volumenTidal volumen tidal estimado, en mililitros
 */
void calcularVolumenTidal(int* volumenTidal, int estatura, int sexo) {
  float peso0, pesoIdeal, volumenEstimado;
  if (sexo == 0) { // Varón
    peso0 = 50.0;
  } else if (sexo == 1) { // Mujer
    peso0 = 45.5;
  }
  pesoIdeal = peso0 + 0.91 * (estatura - 152.4); // en kg

  *volumenTidal = int(round(pesoIdeal * DEFAULT_ML_POR_KG_DE_PESO_IDEAL));
}

/**
 * @brief calcula los tiempos de ciclo, inspiratorio y espiratorio, en seg.
 *
 * Calcula a partir de las respiraciones por minuto, los tiempos de ciclo,
 * inspiratorio y espiratorio, y las velocidades uno y dos.
 * @param velocidadUno TODO: explicación?
 * @param velocidadDos TODO: explicación?
 * @param tIns tiempo de inspiracion, en segundos
 * @param tEsp tiempo de espiracion, en segundos
 * @param tCiclo tiempo de ciclo, en segundos
 * @param pasosPorRevolucion TODO: explicación?
 * @param microStepper TODO: explicación?
 * @param porcentajeInspiratorio fraccion del ciclo en la que se inspira, tIns/tCiclo*100
 * @param rpm respiraciones por minuto
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

// =========================================================================
// SETUP
// =========================================================================
void setup()
{

  // INICIALIZACION
  // =========================================================================

  // Puerto serie
  Serial.begin(9600);
  Serial.println("Inicio");

  // Display de inicio
  display.writeLine(0, "REESPIRATOR");

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

  // deja la display en blanco
  delay(1000);
  display.clear();
  delay(100);


  // INTERACCIÓN: ESTATURA
  // =========================================================================
  display.writeLine(0, "Introduce estatura");
  while(!encoder.readButton()) {
    encoder.updateValue(&estatura);
    display.writeLine(1, "Altura: " + String(estatura) + " cm");
  }
  display.writeLine(0, "Valor guardado");
  display.writeLine(1, "Altura: " + String(estatura) + " cm");
  Serial.println("Altura (cm): " + String(estatura));
  delay(1000);
  display.clear();


  // INTERACCIÓN: SEXO
  // =========================================================================
  display.writeLine(0, "Introduce sexo");
  while(!encoder.readButton()) {
    encoder.swapValue(&sexo);
    if (sexo == 0) {
      display.writeLine(1, "Sexo: varon");
    } else if (sexo == 1) {
      display.writeLine(1, "Sexo: mujer");
    }
  }
  display.writeLine(0, "Sexo seleccionado");
  if (sexo == 0) {
    display.writeLine(1, "Sexo: varon");
  } else if (sexo == 1) {
    display.writeLine(1, "Sexo: mujer");
  }
  Serial.println("Sexo (0:V, 1:M): " + String(sexo));
  delay(1000);
  display.clear();


  // ESTIMACIÓN: VOLUMEN TIDAL
  // =========================================================================
  display.writeLine(0, "Volumen tidal");
  // TODO: calcular volumen tidal estimado en función de la estatura
  calcularVolumenTidal(&volumenTidal, estatura, sexo);
  display.writeLine(1, String(volumenTidal) + " ml");
  Serial.println("Volumen tidal estimado (ml): " + String(volumenTidal));
  delay(2000);
  display.clear();


  // INTERACCIÓN: VOLUMEN TIDAL
  // =========================================================================
  display.writeLine(0, "Modifica volumen");
  while(!encoder.readButton()) {
    encoder.updateValue(&volumenTidal, 10);
    volumenTidal = constrain(volumenTidal, DEFAULT_MIN_VOLUMEN_TIDAL, DEFAULT_MAX_VOLUMEN_TIDAL);
    display.writeLine(1, String(volumenTidal) + " ml");
  }
  display.writeLine(0, "Valor guardado");
  display.writeLine(1, String(volumenTidal) + " ml");
  Serial.println("Volumen tidal configurado (ml): " + String(volumenTidal));
  delay(1000);
  display.clear();


  // INTERACCIÓN: TRIGGER SI/NO
  // =========================================================================
  display.writeLine(0, "Trigger?");
  while(!encoder.readButton()) {
    encoder.swapValue(&tieneTrigger);
    if (tieneTrigger) {
      display.writeLine(1, "Si");
    } else {
      display.writeLine(1, "No");
    }
  }
  display.writeLine(0, "Valor guardado");
  if (tieneTrigger) {
    display.writeLine(1, "Trigger: Si");
  } else {
    display.writeLine(1, "Trigger: No");
  }
  Serial.println("Trigger (0:No, 1:Si): " + String(tieneTrigger));
  delay(1000);
  display.clear();


  // INTERACCIÓN: VALOR DEL TRIGGER
  // =========================================================================
  if (tieneTrigger) {
    display.writeLine(0, "Modifica trigger");
    while(!encoder.readButton()) {
      encoder.updateValue(&flujoTrigger, 0.1);
      display.writeLine(1, "Flujo: " + String(flujoTrigger) + " LPM");
    }
    display.writeLine(0, "Valor guardado");
    display.writeLine(1, "Flujo: " + String(flujoTrigger) + " LPM");
    Serial.println("Flujo trigger (LPM): " + String(flujoTrigger));
    delay(1000);
    display.clear();
  }


  // INTERACCIÓN: FRECUENCIA RESPIRATORIA
  // =========================================================================
  display.writeLine(0, "Frecuencia resp.");
  while(!encoder.readButton()) {
    encoder.updateValue(&rpm);
    rpm = constrain(rpm, DEFAULT_MIN_RPM, DEFAULT_MAX_RPM);
    display.writeLine(1, String(rpm) + " rpm");
  }
  display.writeLine(0, "Valor guardado");
  display.writeLine(1, String(rpm) + " rpm");
  Serial.println("Frecuencia respiratoria (rpm): " + String(rpm));
  delay(1000);
  display.clear();


  // CÁLCULO: CONSTANTES DE TIEMPO INSPIRACION/ESPIRACION
  // =========================================================================
  display.writeLine(0, "Tins | Tesp (seg)");
  calcularCicloInspiratorio(&velocidadUno, &velocidadDos, &tIns, &tEsp,
                            &tCiclo, pasosPorRevolucion, microStepper,
                            porcentajeInspiratorio, rpm);
  display.writeLine(1, String(tIns) + " s" + String(tEsp) + " s");
  Serial.println("Tiempo del ciclo (seg):" + String(tCiclo));
  Serial.println("Tiempo inspiratorio (seg):" + String(tIns));
  Serial.println("Tiempo espiratorio (seg):" + String(tEsp));
  Serial.println("Velocidad 1 calculada:" + String(velocidadUno));
  Serial.println("Velocidad 2 calculada:" + String(velocidadDos));
  delay(2000);
  display.clear();

  // TODO: Mostrar todos los parametros

  // INTERACCIÓN: ARRANQUE
  // =========================================================================
  display.writeLine(0, "Pulsa para iniciar");
  display.writeLine(1, "Esperando...");
  while(!encoder.readButton());
  display.clear();
  display.writeLine(1, "Iniciando...");

  // Habilita el motor
  digitalWrite(ENpin, HIGH);
  delay(500);
  display.clear();
}


void loop()
{


  // TODO: Mostrar todos los parametros

  // TODO: Escuchar entrada desde el encoder
  // si hay nueva configuración: cambiar parámetros


  // TODO: chequear trigger
  // si hay trigger, esperar al flujo umbral para actuar, si no, actuar en cada bucle


  // Si está en inspiración: controlar con PID el volumen tidal (el que se insufla)

  // Si está en espiración: soltar balón (mover leva hacia arriba sin controlar) y esperar


  // ======================================================================
  // CÓDIGO OBSOLETO DE AQUÍ PARA ABAJO
  // ======================================================================

  // Parte menu
  // display.update(encoder.read());

  // Parte stepper
  stepper.run();

  //recalcular valores por si han cambiado en el menu
  // TODO: sustituir por nueva funcion: calcularConstantes();

  // Primera mitad del ciclo
  if (!stepper.isRunning() && modo && !errorFC) {
    Serial.println("Modo 1");
    stepper.setMaxSpeed(velocidadUno * microStepper);

    stepper.move(pasosPorRevolucion * microStepper / 2);
    modo = !modo;
  }

  // Segunda mitad del ciclo
  if (!stepper.isRunning() && !modo && !errorFC) {
    Serial.println("Modo 2");

    //se ha llegado al final de carrera en el momento que toca pensar que esta defino como pullup
    if (digitalRead(ENDSTOPpin)) {
      Serial.println("Final de carrera OK");
      stepper.setMaxSpeed(velocidadDos * microStepper);
      stepper.move(pasosPorRevolucion * microStepper / 2);
      modo = !modo;
    }
    // si acabada la segunda parte del movimiento no se ha llegado al SENSOR HALL entonces da un paso y vuelve a hacer la comprovocacion
    else {
      Serial.println("Final de carrera NO DETECTADO: Buscando FC");
      errorFC = true;
      digitalWrite(BUZZpin, true); //activa el zumbador
      stepper.move(1 * microStepper);
    }
  }

  //si estamos en error y ha echo los pasos extra en busca del Final de Carrera
  if (!stepper.isRunning() && errorFC) {
    // no se ha llegado al final suena el BUZZ y ordena dar 3 pasos en busca del FC
    if (!digitalRead(ENDSTOPpin)) {
      Serial.println("--Buscando FC");
      errorFC = true;
      stepper.move(1);
    }
    // cuando lo ha localizado ordena seguir con velocidad 2
    else {
      Serial.println("Detectado FC: restableciendo el origen");
      errorFC = false;
      digitalWrite(BUZZpin, false); //apaga el zumbador
      stepper.setMaxSpeed(velocidadDos * microStepper);
      stepper.move(pasosPorRevolucion * microStepper / 2);
      modo = !modo; //cambiamos de velocidad
    }
  }
  // si hay un error pero ha hecho los 100 pasos extra en busca del Final de Carrera
  else if (!stepper.isRunning() && errorFC) {
    // no se ha llegado al final suena el BUZZ y ordena dar 3 pasos en busca del FC
    if (digitalRead(ENDSTOPpin)) {
      errorFC = true;
      stepper.move(1);
      digitalWrite(BUZZpin, true);
      Serial.println("ZUMBA");
    }
    // cuando lo ha localizado ordena seguir con velocidad 2
    else {
      errorFC = false;
      digitalWrite(BUZZpin, false);
      stepper.setMaxSpeed(velocidadDos);
      stepper.move(pasosPorRevolucion / 2);
    }
  }
}
