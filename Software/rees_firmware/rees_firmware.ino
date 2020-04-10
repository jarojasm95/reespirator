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
#include "src/AutoPID/AutoPID.h"
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
int pasosPorRevolucion     = DEFAULT_PASOS_POR_REVOLUCION;
float flujoTrigger         = DEFAULT_FLUJO_TRIGGER;

bool tieneTrigger;
bool errorFC = false;
int volumenTidal;
int stage = 0;
float speedIns = 0.1;
float speedEsp = 0.1;
float tCiclo, tIns, tEsp;
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
AutoPID *pidI = new AutoPID(0, 100, 80, 2, 5);
AutoPID *pidE = new AutoPID(0, 100, 80, 2, 5);
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(I2C_DIR, LCD_COLS, LCD_ROWS);
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
  stepper.setStepsPerRevolution(pasosPorRevolucion);
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

  /*
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
    printToLcd(1, String(rpm) + " rpm");
  }
  printTo(0, "Valor guardado", true);
  printTo(1, String(rpm) + " rpm");
  delay(1000);


  // CÁLCULO: CONSTANTES DE TIEMPO INSPIRACION/ESPIRACION
  // =========================================================================
  calcularCicloInspiratorio(&tIns, &tEsp, &tCiclo, porcentajeInspiratorio, rpm);

  // INFORMACIÓN: PARÁMETROS
  // =========================================================================
  printTo(0, "Tins (seg): " + String(tIns) + " s", true);
  printTo(1, "Tesp (seg): " + String(tEsp) + " s");
  //printTo(2, "Vol: " + String(volumenTidal) + " ml");
  printTo(2, "Tciclo (seg): " + String(tCiclo) + " s");
  printTo(3, "Frec: " + String(rpm) + " rpm");
  Serial.println("Tiempo inspiratorio (seg):" + String(tIns));
  Serial.println("Tiempo espiratorio (seg):" + String(tEsp));
  Serial.println("Velocidad inspiratoria calculada:" + String(speedIns));
  Serial.println("Velocidad espiratoria calculada:" + String(speedEsp));
  while(!encoder.readButton()){
    delay(500);
  };


// INTERACCIÓN: ARRANQUE
// =========================================================================
  printTo(0, "Pulsa para iniciar", true);
  printTo(1, "Esperando...");
  while(!encoder.readButton()){
    delay(500);
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
  stepper.setAccelerationInRevolutionsPerSecondPerSecond(speed);
  stepper.setTargetPositionRelativeInRevolutions(revs * -1.0);
}

bool sensorStatus = false;

void resetToHome() {
  while(!hallSensorActivated()) {
    setMotorPosition(0.1, 0.01);
    stepper.processMovement();
  }
  setMotorPosition(0.1, 0.52);
  stepper.processMovement();
}

void resetToNonHome() {
  while(hallSensorActivated()) {
    setMotorPosition(0.1, 0.01);
    stepper.processMovement();
  }
}

void reset() {
  errorFC = false;
  disableBuzzer();
  resetToNonHome();
  resetToHome();
  lcd.clear();
  sensorStatus = true;
  stage = 0;
  time = millis();
  cycleTime = tIns;
}

int selection = 0;
int cycles = 0;

void loop() {
  printToLcd(0, String(rpm) + " rpm / " + String(porcentajeInspiratorio) + "%");

  if(selection >= 2) {
    selection = 0;
  }
  bool switched = false;
  while(!stepper.motionComplete()){
    stepper.processMovement();
    if (!switched) {
      bool sensor = hallSensorActivated();
      if (!switched && sensorStatus != sensor) {
        sensorStatus = sensor;
        switched = !switched;
      }
    }
  }
  calcularCicloInspiratorio(&tIns, &tEsp, &tCiclo, porcentajeInspiratorio, rpm);
  if (errorFC) {
    // no se ha llegado al final suena el BUZZ y ordena dar 3 pasos en busca del FC
    if (encoder.readButton()) {
      printTo(2, "Restableciendo");
      reset();
    }
  } else if (stage == 0) { // Primera mitad del ciclo
    printToLcd(1, "T Esp: " + String(cycleTime) + "s/" + String(tEsp) + "s");
    printToLcd(3, "Respiraciones: " + String(cycles));
    setMotorPosition(speedIns, .25);
    //pidE->run(cycleTime, tEsp, &speedEsp);
    float diff = cycleTime - tEsp;
    if (diff > 0.05 || diff < -0.05) {
      speedEsp = speedEsp + (speedEsp * diff / tEsp);
    }
    Serial.println("Velocidad inspiratoria calculada:" + String(speedEsp));
    stage = 1;
    cycles = cycles + 1;
  } else if (stage == 1) {
    cycleTime = (millis() - time) / 1000.0;
    time = millis();
    stage = 2;
  } else if (stage == 2) { // Segunda mitad del ciclo
    //se ha llegado al final de carrera en el momento que toca pensar que esta defino como pullup
    printToLcd(2, "T Ins: " + String(cycleTime) + "s/" + String(tIns) + "s");
    setMotorPosition(speedEsp, .125);
    //pidI->run(cycleTime, tIns, &speedIns);
    float diff = cycleTime - tIns;
    if (diff > 0.05 || diff < -0.05) {
      speedIns = speedIns + (speedIns * diff / tIns);
    }
    Serial.println("Velocidad espiratoria calculada:" + String(speedIns));
    stage = 3;
  } else if (stage == 3) {
    cycleTime = (millis() - time) / 1000.0;
    time = millis();
    stage = 0;
    setMotorPosition(30, 0.625);
  } else {
    // si acabada la segunda parte del movimiento no se ha llegado al SENSOR HALL entonces da un paso y vuelve a hacer la comprovocacion
    errorFC = true;
    //enableBuzzer();
  }
}
