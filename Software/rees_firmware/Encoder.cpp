#include "Encoder.h"
#include "Arduino.h"

const int8_t KNOBDIR[] = {
     0, -1,  1,  0,
     1,  0,  0, -1,
    -1,  0,  0,  1,
     0,  1, -1,  0
};

Encoder::Encoder(int pin1, int pin2, int pulsador) {
  _pin1 = pin1;
  _pin2 = pin2;
  _pulsador = pulsador;

  // Setup the input pins and turn on pullup resistor
  pinMode(pin1, INPUT_PULLUP);
  pinMode(pin2, INPUT_PULLUP);
  pinMode(pulsador, INPUT_PULLUP);

  // when not started in motion, the current state of the encoder should be 3
  _oldState = 3;

  // start with position 0;
  _position = 0;
  _positionExt = 0;
  _positionExtPrev = 0;
  _flag = false;
  _tiempo = 0;
}

long Encoder::getPosition() {
  return _positionExt;
}

int Encoder::getDirection() {

  int ret = 0;

  if (_positionExtPrev > _positionExt) {
    ret = -1;
    _positionExtPrev = _positionExt;
  }
  else if (_positionExtPrev < _positionExt) {
    ret = 1;
    _positionExtPrev = _positionExt;
  }
  else {
    ret = 0;
    _positionExtPrev = _positionExt;
  }

  return ret;
}

// unused
void Encoder::setPosition(long newPosition) {
  _position = ((newPosition << 2) | (_position & 0x03L));
  _positionExt = newPosition;
  _positionExtPrev = newPosition;
}

void Encoder::tick(void) {
  int sig1 = digitalRead(_pin1);
  int sig2 = digitalRead(_pin2);
  int8_t thisState = sig1 | (sig2 << 1);

  if (_oldState != thisState)   {
    _position += KNOBDIR[thisState | (_oldState << 2)];

    if (thisState == LATCHSTATE)
      _positionExt = _position >> 2;

    _oldState = thisState;
  }
}

/**
 * @brief Modifica un valor dicotómico entre 0 y 1.
 *
 * Cuando se gira el encoder, el valor se pasa de 0 a 1 y
 * de 1 a 0 cada vez que se mueve el dial.
 *
 * @param valor valor a actualizar
 */
bool Encoder::swapValue(int* valor) {
  // Giramos horario o antihorario
  int val = read();
  if (val == 1 || val == -1) {
    if (*valor == 0) {
      *valor = 1;
    } else if (*valor == 1) {
      *valor = 0;
    }
  }
  return val != 0;
}

bool Encoder::swapValue(bool* valor) {
  // Giramos horario o antihorario
  int val = read();
  if (val == 1 || val == -1) {
    *valor = !(*valor);
  }
  return val != 0;
}

/**
 * @brief Lee la señal emitida al pulsar el encoder
 *
 * @return true cuando se pulsa
 * @return false si no se ha pulsado
 */
bool Encoder::readButton() {
  if (digitalRead(_pulsador) != 1) {
    if (!_flag) {
      _tiempo = millis();
      _flag = true;
    }
    while (digitalRead(_pulsador) == 0) {
      if (millis() - _tiempo > 30) {
        _flag = false;
        return true;
      }
    }
  }
  return false;
}

/**
 * @brief Lee el giro del knob del encoder.
 *
 * @return int giro antihorario: -1, giro horario: 1
 */
int Encoder::read() {
  tick();
  int dir = getDirection();

  // Giramos antihorario (Subimos en el menu)
  if (dir > 0) {
    return 1;
  }
  // Giramos horario (Subimos en el menu)
  else if (dir < 0) {
    return -1;
  }
  // Pulsamos (Modificamos valor)
  if (readButton()) {
    return 0;
  }
  return 2;
}


bool Encoder::adjustValue(int* valor, int delta = 1) {
  int val = read();
  if (val == 1) {
    *valor = *valor + delta;
  } else if (val == -1) {
    *valor = *valor - delta;
  }
  return val != 0;
}

bool Encoder::adjustValue(float* valor, float delta = 1.0) {
  int val = read();
  if (val == 1) {
    *valor = *valor + delta;
  } else if (val == -1) {
    *valor = *valor - delta;
  }
  return val != 0;
}
