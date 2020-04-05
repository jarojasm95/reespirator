// =========================================================================
// DEPENDENCIAS
// =========================================================================

#include "defaults.h"
#include "utils.h"
#include "pinout.h"
//#include "Display.h"    - Se ignora; sólo se trabajará con LiquidCrystal_I2C
#include "Encoder.h"
#include "src/FlexyStepper/FlexyStepper.h"
#include "src/LiquidCrystal_I2C/LiquidCrystal_I2C.h"
#include "Arduino.h"
#include "Wire.h"

// =========================================================================
// VARIABLES
// =========================================================================

int rpm                    = DEFAULT_RPM;
int porcentajeInspiratorio = DEFAULT_POR_INSPIRATORIO;
int estatura               = DEFAULT_ESTATURA;
int sexo                   = DEFAULT_SEXO;
int microStepper           = DEFAULT_MICROSTEPPER;
int aceleracion            = DEFAULT_ACELERACION * microStepper;
int pasosPorRevolucion     = DEFAULT_PASOS_POR_REVOLUCION;
float flujoTrigger         = DEFAULT_FLUJO_TRIGGER;

bool tieneTrigger;
bool errorFC = false;
int volumenTidal;
int stage = 0;
float speedIns, speedEsp, tCiclo, tIns, tEsp;
uint32_t time = 0;
float cycleTime = 0.0;

// pines en pinout.h
FlexyStepper stepper;
// direction Digital 6 (CW), pulses Digital 7 (CLK)
Encoder encoder(
  DTpin,
  CLKpin,
  SWpin
);
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27,20,4);
// =========================================================================
// SETUP
// =========================================================================

void enableBuzzer() {
  digitalWrite(BUZZpin, true);
}

void disableBuzzer() {
  digitalWrite(BUZZpin, false);
}

void printToLcd(int col, String message, bool clear = false) {
  if (clear) {
    lcd.clear();
  }
  lcd.setCursor(0, col);
  lcd.print(message);
}

void printTo(int col, String message, bool clear = false) {
  printToLcd(col, message, clear);
  Serial.println(message);
}


void setup()
{
  // INICIALIZACION
  // =========================================================================
  lcd.begin();
  // Puerto serie
  Serial.begin(9600);
  printTo(0, "Inicio", true);
  printTo(1, "REESPIRATOR");

  // Zumbador
  pinMode(BUZZpin, OUTPUT);
  enableBuzzer();
  delay(100);
  disableBuzzer();

  // FC efecto hall
  pinMode(ENDSTOPpin, INPUT); // el sensor de efecto hall da un 1 cuando detecta
  stepper.connectToPins(PULpin, DIRpin);
  stepper.setStepsPerRevolution(3200);
  stepper.setSpeedInRevolutionsPerSecond(1.0);
  stepper.setAccelerationInRevolutionsPerSecondPerSecond(1.0);
  printTo(2, "Setup");
  delay(1000);

  // INTERACCIÓN: ESTATURA
  // =========================================================================
  printTo(0, "Introduce estatura", true);
  while(encoder.adjustValue(&estatura)) {
    printToLcd(1, "Altura: " + String(estatura) + " cm");
  }
  printTo(0, "Valor guardado", true);
  printTo(1, "Altura: " + String(estatura) + " cm");
  delay(1000);

  // INTERACCIÓN: SEXO
  // =========================================================================
  printTo(0, "Introduce sexo", true);
  while(encoder.swapValue(&sexo)) {
    if (sexo == 0) {
      printToLcd(1, "Sexo: varon");
    } else if (sexo == 1) {
      printToLcd(1, "Sexo: mujer");
    }
  }
  printTo(0, "Valor guardado", true);
  if (sexo == 0) {
    printTo(1, "Sexo: varon");
  } else if (sexo == 1) {
    printTo(1, "Sexo: mujer");
  }
  delay(1000);


  // ESTIMACIÓN: VOLUMEN TIDAL
  // =========================================================================
  printTo(0, "Vol. tidal estimado:", true);
  // TODO: calcular volumen tidal estimado en función de la estatura
  calcularVolumenTidal(&volumenTidal, estatura, sexo);
  printTo(1, String(volumenTidal) + " ml");

  // INTERACCIÓN: VOLUMEN TIDAL
  // =========================================================================
  printTo(2, "Modifica vol. tidal");
  while(encoder.adjustValue(&volumenTidal, 10)) {
    volumenTidal = constrain(volumenTidal, DEFAULT_MIN_VOLUMEN_TIDAL, DEFAULT_MAX_VOLUMEN_TIDAL);
    printToLcd(3, String(volumenTidal) + " ml");
  }
  printTo(0, "Valor guardado", true);
  printTo(1, String(volumenTidal) + " ml");
  delay(1000);


  /*
  // INTERACCIÓN: TRIGGER SI/NO
  // =========================================================================
  //display.writeLine(0, "Trigger?");    - Línea obsoleta
  printTo(0, "Trigger?", true);
  while(encoder.swapValue(&tieneTrigger)) {
    if (tieneTrigger) {
      //display.writeLine(1, "Si");    - Línea obsoleta
      printToLcd(1, "Si");
    } else {
      //display.writeLine(1, "No");    - Línea obsoleta
      printToLcd(1, "No");
    }
  }
  printTo(0, "Valor guardado", true);
  if (tieneTrigger) {
    //display.writeLine(1, "Trigger: Si");
    printTo(1, "Trigger: Si");
  } else {
    //display.writeLine(1, "Trigger: No");    - Línea obsoleta
    printTo(1, "Trigger: No");
  }
  delay(1000);


  // INTERACCIÓN: VALOR DEL TRIGGER
  // =========================================================================
  if (tieneTrigger) {
    printTo(0, "Modifica trigger", true);
    while(encoder.adjustValue(&flujoTrigger, 1)) {
      printToLcd(1, "Flujo: " + String(flujoTrigger) + " LPM");
    }
    printTo(0, "Valor guardado", true);
    printTo(1, "Flujo: " + String(flujoTrigger) + " LPM");
    delay(1000);
  }
  */

  // INTERACCIÓN: FRECUENCIA RESPIRATORIA
  // =========================================================================
  printTo(0, "Frec. respiratoria", true);
  while(encoder.adjustValue(&rpm)) {
    lcd.setCursor(0,1);
    rpm = constrain(rpm, DEFAULT_MIN_RPM, DEFAULT_MAX_RPM);
    //display.writeLine(1, String(rpm) + " rpm");    - Línea obsoleta
    printToLcd(1, String(rpm) + " rpm");
  }
  printTo(0, "Valor guardado", true);
  printTo(1, String(rpm) + " rpm");
  delay(1000);


  // CÁLCULO: CONSTANTES DE TIEMPO INSPIRACION/ESPIRACION
  // =========================================================================
  calcularCicloInspiratorio(&speedIns, &speedEsp, &tIns, &tEsp,
                            &tCiclo, pasosPorRevolucion, microStepper,
                            porcentajeInspiratorio, rpm);

  // INFORMACIÓN: PARÁMETROS
  // =========================================================================
  printTo(0, "Tins: " + String(tIns) + " s", true);
  printTo(1, "Tesp (seg): " + String(tEsp) + " s");
  printTo(2, "Vol: " + String(volumenTidal) + " ml");
  printTo(3, "Frec: " + String(rpm) + " rpm");
  Serial.println("Tiempo del ciclo (seg):" + String(tCiclo));
  Serial.println("Tiempo inspiratorio (seg):" + String(tIns));
  Serial.println("Tiempo espiratorio (seg):" + String(tEsp));
  Serial.println("Velocidad inspiratoria calculada:" + String(speedIns));
  Serial.println("Velocidad espiratoria calculada:" + String(speedEsp));
  delay(5000);


// INTERACCIÓN: ARRANQUE
// =========================================================================
  printTo(0, "Pulsa para iniciar", true);
  printTo(1, "Esperando...");
  while(!encoder.readButton()){
    delay(1000);
  };
  printTo(0, "Iniciando...", true);
  digitalWrite(ENpin, LOW);
  delay(500);
  reset();
}

// =========================================================================
// LOOP
// =========================================================================

bool hallSensorActivated() {
  return !digitalRead(ENDSTOPpin);
}

void setMotorPosition(float speed, float revs = 0.5) {
  stepper.setCurrentPositionInRevolutions(0.0);
  stepper.setSpeedInRevolutionsPerSecond(speed);
  stepper.setTargetPositionRelativeInRevolutions(revs * -1.0);
}

bool sensorStatus = false;

void resetToHome() {
  while(!hallSensorActivated()) {
    setMotorPosition(0.1, 0.01);
    stepper.processMovement();
  }
}

void resetToNonHome() {
  while(hallSensorActivated()) {
    setMotorPosition(0.1, 0.01);
    stepper.processMovement();
  }
}

void reset() {
  errorFC = false;
  // stage = 0; //cambiamos de velocidad
  disableBuzzer();
  //resetToNonHome();
  //resetToHome();
  lcd.clear();
  sensorStatus = true;
  stage = 0;
  time = millis();
  cycleTime = tIns;
  speedIns = 0.10;
  speedEsp = 0.10;
}

void loop() {
  printToLcd(0, "Operando...");

  // TODO: chequear trigger
  // si hay trigger, esperar al flujo umbral para actuar, si no, actuar en cada bucle
  // Si está en inspiración: controlar con PID el volumen tidal (el que se insufla)
  // Si está en espiración: soltar balón (mover leva hacia arriba sin controlar) y esperar

  // recalcular valores por si han cambiado en el menu
  // TODO: sustituir por nueva funcion: calcularConstantes();
  bool switched = false;
  while(!stepper.motionComplete()){
    stepper.processMovement();
    if (!switched) {
      /*
      bool sensor = hallSensorActivated();
      if (!switched && sensorStatus != sensor) {
        sensorStatus = sensor;
        switched = !switched;
      }
      */
      sensorStatus = !sensorStatus;
      switched = !switched;
    }
  }
  if (errorFC) {
    // no se ha llegado al final suena el BUZZ y ordena dar 3 pasos en busca del FC
    if (encoder.readButton()) {
      printTo(2, "Restableciendo");
      reset();
    }
  } else if (stage == 0) { // Primera mitad del ciclo
    printToLcd(1, "Ciclo 1");
    printToLcd(2, "Tiempo: " + String(cycleTime) + "s/" + String(tIns) + "s");
    printToLcd(3, "Velocidad: " + String(speedIns));
    setMotorPosition(speedIns);
    float diff = cycleTime - tIns;
    if (diff != 0.0) {
      speedIns = speedIns + (speedIns * diff / tIns);
    }
    stage = 1;
  } else if (stage == 1 && !sensorStatus) {
    cycleTime = (millis() - time) / 1000.0;
    time = millis();
    stage = 2;
  } else if (stage == 2) { // Segunda mitad del ciclo
    //se ha llegado al final de carrera en el momento que toca pensar que esta defino como pullup
    printToLcd(1, "Ciclo 2");
    printToLcd(2, "Tiempo: " + String(cycleTime) + "s/" + String(tEsp) + "s");
    printToLcd(3, "Velocidad: " + String(speedEsp));
    setMotorPosition(speedEsp);
    float diff = cycleTime - tEsp;
    if (diff != 0.0) {
      speedEsp = speedEsp + (speedEsp * diff / tEsp);
    }
    stage = 3;
  } else if (stage == 3 && sensorStatus) {
    cycleTime = (millis() - time) / 1000.0;
    time = millis();
    stage = 0;
  } else {
    // si acabada la segunda parte del movimiento no se ha llegado al SENSOR HALL entonces da un paso y vuelve a hacer la comprovocacion
    errorFC = true;
    //enableBuzzer();
  }
}
