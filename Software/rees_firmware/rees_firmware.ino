// =========================================================================
// DEPENDENCIAS
// =========================================================================

#include "defaults.h"
#include "utils.h"
#include "pinout.h"
//#include "Display.h"    - Se ignora; sólo se trabajará con LiquidCrystal_I2C
#include "Encoder.h"
#include "MechVentilation.h"
#include "src/AccelStepper/AccelStepper.h"
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
bool modo = true, errorFC = false;
int volumenTidal;
float speedIns, speedEsp, tCiclo, tIns, tEsp;

// pines en pinout.h
AccelStepper stepper(
  AccelStepper::DRIVER,
  PULpin,
  DIRpin
); // direction Digital 6 (CW), pulses Digital 7 (CLK)
Encoder encoder(
  DTpin,
  CLKpin,
  SWpin
);
//Display display = Display();    - Línea obsoleta
//MechVentilation ventilation;    - Línea obsoleta
MechVentilation ventilation = ventilation;
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27,20,4);
// =========================================================================
// SETUP
// =========================================================================

void setup()
{
  // INICIALIZACION
  // =========================================================================
  lcd.begin();
  // Puerto serie
  Serial.begin(9600);
  Serial.println("Inicio");
  //display.clear();
  lcd.clear();
  // Display de inicio
  //display.writeLine(0, "REESPIRATOR");    - Línea obsoleta
  lcd.print("REESPIRATOR");
  // Zumbador
  pinMode(BUZZpin, OUTPUT);
  digitalWrite(BUZZpin, HIGH); // test zumbador
  delay(100);
  digitalWrite(BUZZpin, LOW);

  // FC efecto hall
  pinMode(ENDSTOPpin, INPUT); // el sensor de efecto hall da un 1 cuando detecta

  // Parte motor
  //pinMode(ENpin, OUTPUT);
  //digitalWrite(ENpin, LOW);

  Serial.println("Setup");
  stepper.setAcceleration(aceleracion);

  // deja la display en blanco
  delay(1000);
  //display.clear();    - Línea obsoleta
  lcd.clear();
  delay(100);


  // INTERACCIÓN: ESTATURA
  // =========================================================================
  //display.writeLine(0, "Introduce estatura");    - Línea obsoleta
  lcd.print("Introduce estatura");
  while(!encoder.readButton()) {
    encoder.updateValue(&estatura);
    //display.writeLine(1, "Altura: " + String(estatura) + " cm");    - Línea obsoleta
    lcd.setCursor(0,1);
    lcd.print("Altura: " + String(estatura) + " cm");
  }
    lcd.clear();
  //display.writeLine(0, "Valor guardado");    - Línea obsoleta
    lcd.print("Valor guardado");
    lcd.setCursor(0,1);
  //display.writeLine(1, "Altura: " + String(estatura) + " cm")    - Línea obsoleta;
    lcd.print("Altura: " + String(estatura) + " cm");
  Serial.println("Altura (cm): " + String(estatura));
  delay(1000);
  //display.clear();    - Línea obsoleta
    lcd.clear();

  // INTERACCIÓN: SEXO
  // =========================================================================
  //display.writeLine(0, "Introduce sexo");    - Línea obsoleta
  lcd.print("Introduce sexo");
  while(!encoder.readButton()) {
    encoder.swapValue(&sexo);
    if (sexo == 0) {
      //display.writeLine(1, "Sexo: varon");    - Línea obsoleta
      lcd.setCursor(0,1);
      lcd.print("Sexo: varon");
    } else if (sexo == 1) {
      //display.writeLine(1, "Sexo: mujer");    - Línea obsoleta
      lcd.setCursor(0,1);
      lcd.print("Sexo: mujer");
    }
  }
  lcd.clear();
  //display.writeLine(0, "Sexo seleccionado");    - Línea obsoleta
  lcd.print("Sexo seleccionado");
  if (sexo == 0) {
    //display.writeLine(1, "Sexo: varon");    - Línea obsoleta
    lcd.setCursor(0,1);
    lcd.print("Sexo: varon");
  } else if (sexo == 1) {
    //display.writeLine(1, "Sexo: mujer");    - Línea obsoleta
    lcd.setCursor(0,1);
    lcd.print("Sexo: mujer");
  }
  Serial.println("Sexo (0:V, 1:M): " + String(sexo));
  delay(1000);
  //display.clear();    - Línea obsoleta
  lcd.clear();


  // ESTIMACIÓN: VOLUMEN TIDAL
  // =========================================================================
  //display.writeLine(0, "Volumen tidal");    - Línea obsoleta
  lcd.print("Vol. tidal estimado:");
  lcd.setCursor(0,2);
  // TODO: calcular volumen tidal estimado en función de la estatura
  calcularVolumenTidal(&volumenTidal, estatura, sexo);
  //display.writeLine(1, String(volumenTidal) + " ml");    - Línea obsoleta
  lcd.print(String(volumenTidal) + " ml");
  Serial.println("Volumen tidal estimado (ml): " + String(volumenTidal));
  delay(3000);
  //display.clear();    - Línea obsoleta
  lcd.clear();

  // INTERACCIÓN: VOLUMEN TIDAL
  // =========================================================================
  //display.writeLine(0, "Modifica volumen");    - Línea obsoleta
  lcd.print("Modifica volumen");
  while(!encoder.readButton()) {
    encoder.updateValue(&volumenTidal, 10);
    volumenTidal = constrain(volumenTidal, DEFAULT_MIN_VOLUMEN_TIDAL, DEFAULT_MAX_VOLUMEN_TIDAL);
    //display.writeLine(1, String(volumenTidal) + " ml");    - Línea obsoleta
    lcd.setCursor(0,1);
    lcd.print(String(volumenTidal) + " ml");
  }
  //display.writeLine(0, "Valor guardado");    - Línea obsoleta
  lcd.clear();
  lcd.print("Valor guardado");
  //display.writeLine(1, String(volumenTidal) + " ml");    - Línea obsoleta
  lcd.setCursor(0,1);
  lcd.print(String(volumenTidal) + " ml");
  Serial.println("Volumen tidal configurado (ml): " + String(volumenTidal));
  delay(1000);
  //display.clear();    - Línea obsoleta
  lcd.clear();


  // INTERACCIÓN: TRIGGER SI/NO
  // =========================================================================
  //display.writeLine(0, "Trigger?");    - Línea obsoleta
  lcd.print("Trigger?");
  while(!encoder.readButton()) {
    encoder.swapValue(&tieneTrigger);
    lcd.setCursor(0,1);
    if (tieneTrigger) {
      //display.writeLine(1, "Si");    - Línea obsoleta
      lcd.print("Si");
    } else {
      //display.writeLine(1, "No");    - Línea obsoleta
      lcd.print("No");
    }
  }
  lcd.clear();
  //display.writeLine(0, "Valor guardado");    - Línea obsoleta
  lcd.print("Valor guardado");
  lcd.setCursor(0,1);
  if (tieneTrigger) {
    //display.writeLine(1, "Trigger: Si");
    lcd.print("Trigger: Si");
  } else {
    //display.writeLine(1, "Trigger: No");    - Línea obsoleta
    lcd.print("Trigger: No");
  }
  Serial.println("Trigger (0:No, 1:Si): " + String(tieneTrigger));
  delay(1000);
  //display.clear();    - Línea obsoleta
  lcd.clear();


  // INTERACCIÓN: VALOR DEL TRIGGER
  // =========================================================================
  if (tieneTrigger) {
    //display.writeLine(0, "Modifica trigger");    - Línea obsoleta
    lcd.print("Modifica trigger");
    while(!encoder.readButton()) {
      encoder.updateValue(&flujoTrigger, 1);
      lcd.setCursor(0,1);
      //display.writeLine(1, "Flujo: " + String(flujoTrigger) + " LPM");    - Línea obsoleta
      lcd.print("Flujo: " + String(flujoTrigger) + " LPM");
    }
    lcd.clear();
    //display.writeLine(0, "Valor guardado");
    lcd.print("Valor guardado");
    lcd.setCursor(0,1);
    //display.writeLine(1, "Flujo: " + String(flujoTrigger) + " LPM");    - Línea obsoleta
    lcd.print("Flujo: " + String(flujoTrigger) + " LPM");
    Serial.println("Flujo trigger (LPM): " + String(flujoTrigger));
    delay(1000);
    //display.clear();    - Línea obsoleta
  }
  lcd.clear();

  // INTERACCIÓN: FRECUENCIA RESPIRATORIA
  // =========================================================================
  //display.writeLine(0, "Frecuencia resp.");    - Línea obsoleta
  lcd.print("Frec. respiratoria");
  while(!encoder.readButton()) {
    encoder.updateValue(&rpm);
    lcd.setCursor(0,1);
    rpm = constrain(rpm, DEFAULT_MIN_RPM, DEFAULT_MAX_RPM);
    //display.writeLine(1, String(rpm) + " rpm");    - Línea obsoleta
    lcd.print(String(rpm) + " rpm");
  }
  lcd.clear();
  //display.writeLine(0, "Valor guardado");    - Línea obsoleta
  lcd.print("Valor guardado");
  lcd.setCursor(0,1);
  //display.writeLine(1, String(rpm) + " rpm");    - Línea obsoleta
  lcd.print(String(rpm) + " rpm");
  Serial.println("Frecuencia respiratoria (rpm): " + String(rpm));
  delay(1000);
  //display.clear();    - Línea obsoleta
  lcd.clear();


  // CÁLCULO: CONSTANTES DE TIEMPO INSPIRACION/ESPIRACION
  // =========================================================================
  //display.writeLine(0, "Tins | Tesp (seg)");    - Línea obsoleta
  calcularCicloInspiratorio(&speedIns, &speedEsp, &tIns, &tEsp,
                            &tCiclo, pasosPorRevolucion, microStepper,
                            porcentajeInspiratorio, rpm);
  //display.writeLine(1, String(tIns) + " s" + String(tEsp) + " s");    - Línea obsoleta
  lcd.print("Tins: " + String(tIns) + " s");
  lcd.setCursor(0,1);
  lcd.print("Tesp (seg): " + String(tEsp) + " s");
  Serial.println("Tiempo del ciclo (seg):" + String(tCiclo));
  Serial.println("Tiempo inspiratorio (seg):" + String(tIns));
  Serial.println("Tiempo espiratorio (seg):" + String(tEsp));
  Serial.println("Velocidad 1 calculada:" + String(speedIns));
  Serial.println("Velocidad 2 calculada:" + String(speedEsp));
  //display.clear();    - Línea obsoleta

  // INFORMACIÓN: PARÁMETROS
  // =========================================================================
  //display.writeLine(0, "Vol: " + String(volumenTidal) + " ml | " + "Frec: " + String(rpm) + " rpm");    - Línea obsoleta
  lcd.setCursor(0,2);
  lcd.print("Vol: " + String(volumenTidal) + " ml");
  lcd.setCursor(0,3);
  lcd.print("Frec: " + String(rpm) + " rpm");
  delay(5000);
  lcd.clear();
  if (tieneTrigger) {
    //display.writeLine(1, "Trigger: " + String(flujoTrigger) + " LPM");    - Línea obsoleta
    lcd.print("Trigger: " + String(flujoTrigger) + " LPM");
  } else {
    //display.writeLine(1, "No trigger");    - Línea obsoleta
    lcd.print("No trigger");
  }
  delay(2000);
  lcd.clear();


  // INTERACCIÓN: ARRANQUE
  // =========================================================================
 // display.writeLine(0, "Pulsa para iniciar");    - Línea obsoleta
  lcd.print("Pulsa para iniciar");
  lcd.setCursor(0,1);
  //display.writeLine(1, "Esperando...");    - Línea obsoleta
  lcd.print("Esperando...");
  while(!encoder.readButton()){
    delay(1000);
  };
  //display.clear();    - Línea obsoleta
  lcd.clear();
 // display.writeLine(1, "Iniciando...");    - Línea obsoleta
  lcd.print("Iniciando...");

  // Habilita el motor
  //digitalWrite(ENpin, HIGH);

  // configura la ventilación
  if (tieneTrigger) {
    ventilation = MechVentilation(volumenTidal, tIns, tEsp, speedIns, speedEsp, flujoTrigger);
  } else {
    ventilation = MechVentilation(volumenTidal, tIns, tEsp, speedIns, speedEsp);
  }
  stepper.setEnablePin(ENpin);
  stepper.setMaxSpeed(1000);
  stepper.setAcceleration(500);
  stepper.moveTo(8000);
  // Run to target position with set speed and acceleration/deceleration:
  stepper.runToPosition();
  delay(1000);
  // Move back to zero:
  stepper.moveTo(0);
  stepper.runToPosition();
  ventilation.start();
  delay(500);
  //display.clear();    - Línea obsoleta
  lcd.clear();
}

// =========================================================================
// LOOP
// =========================================================================

void loop() {
  //display.writeLine(0, "Operando...");    - Línea obsoleta
  lcd.setCursor(0,0);
  lcd.print("Operando...");
  // TODO: display.writeLine(1, "TODO Prompt ventilation status");
  ventilation.update();

  // TODO: si hay nueva configuración: cambiar parámetros escuchando entrada desde el encoder


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
    Serial.println(String(speedIns) + " * " + String(microStepper));
    stepper.setMaxSpeed(speedIns * microStepper);
    Serial.println(String(pasosPorRevolucion) + " * " + String(microStepper));
    stepper.move(pasosPorRevolucion * microStepper / 2);
    stepper.runToPosition();
    modo = !modo;
  }

  // Segunda mitad del ciclo
  if (!stepper.isRunning() && !modo && !errorFC) {
    Serial.println("Modo 2");

    //se ha llegado al final de carrera en el momento que toca pensar que esta defino como pullup
    if (digitalRead(ENDSTOPpin)) {
      Serial.println("Final de carrera OK");
      stepper.setMaxSpeed(speedEsp * microStepper);
      stepper.move(pasosPorRevolucion * microStepper / 2);
      stepper.runToPosition();
      modo = !modo;
    }
    // si acabada la segunda parte del movimiento no se ha llegado al SENSOR HALL entonces da un paso y vuelve a hacer la comprovocacion
    else {
      Serial.println("Final de carrera NO DETECTADO: Buscando FC");
      errorFC = true;
      digitalWrite(BUZZpin, true); //activa el zumbador
      stepper.move(1 * microStepper);
      stepper.runToPosition();
    }
  }

  //si estamos en error y ha hecho los pasos extra en busca del Final de Carrera
  if (!stepper.isRunning() && errorFC) {
    // no se ha llegado al final suena el BUZZ y ordena dar 3 pasos en busca del FC
    if (!digitalRead(ENDSTOPpin)) {
      Serial.println("--Buscando FC");
      errorFC = true;
      stepper.move(1);
      stepper.runToPosition();
    }
    // cuando lo ha localizado ordena seguir con velocidad 2
    else {
      Serial.println("Detectado FC: restableciendo el origen");
      errorFC = false;
      digitalWrite(BUZZpin, false); //apaga el zumbador
      stepper.setMaxSpeed(speedEsp * microStepper);
      stepper.move(pasosPorRevolucion * microStepper / 2);
      modo = !modo; //cambiamos de velocidad
      stepper.runToPosition();
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
      stepper.runToPosition();
    }
    // cuando lo ha localizado ordena seguir con velocidad 2
    else {
      errorFC = false;
      digitalWrite(BUZZpin, false);
      stepper.setMaxSpeed(speedEsp);
      stepper.move(pasosPorRevolucion / 2);
      stepper.runToPosition();
    }
  }
}
