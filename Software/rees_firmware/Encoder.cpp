#include "Encoder.h"
#include "Arduino.h"

const int8_t KNOBDIR[] = {
     0, -1,  1,  0,
     1,  0,  0, -1,
    -1,  0,  0,  1,
     0,  1, -1,  0
};

Encoder::Encoder(int A, int B, int pulsador)
{
  _pin1 = A;
  _pin2 = B;
  _pulsador = pulsador;

  // Setup the input pins and turn on pullup resistor
  pinMode(A, INPUT_PULLUP);
  pinMode(B, INPUT_PULLUP);
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

long Encoder::getPosition()
{
  return _positionExt;
}

int Encoder::getDirection()
{

  int ret = 0;

  if (_positionExtPrev > _positionExt)
  {
    ret = -1;
    _positionExtPrev = _positionExt;
  }
  else if (_positionExtPrev < _positionExt)
  {
    ret = 1;
    _positionExtPrev = _positionExt;
  }
  else
  {
    ret = 0;
    _positionExtPrev = _positionExt;
  }

  return ret;
}

void Encoder::setPosition(long newPosition)
{
  _position = ((newPosition << 2) | (_position & 0x03L));
  _positionExt = newPosition;
  _positionExtPrev = newPosition;
}

void Encoder::tick(void)
{
  int sig1 = digitalRead(_pin1);
  int sig2 = digitalRead(_pin2);
  int8_t thisState = sig1 | (sig2 << 1);

  if (_oldState != thisState)
  {
    _position += KNOBDIR[thisState | (_oldState << 2)];

    if (thisState == LATCHSTATE)
      _positionExt = _position >> 2;

    _oldState = thisState;
  } // if
} // tick()

int Encoder::leerPulsador()
{
  if (digitalRead(_pulsador) != 1)
  {
    if (!_flag)
    {
      _tiempo = millis();
      _flag = true;
    }
    while (digitalRead(_pulsador) == 0)
    {
      if (millis() - _tiempo > 300)
      {
        _flag = false;
        return 5;
      }
    }
  }
  return 0;
}

int Encoder::leerEncoder()
{
  static int _pos = 0;
  tick();
  int _newPos = getPosition();
  if (_pos > _newPos)
  {
    _pos = _newPos;
    return 8;
  }
  else if (_pos < _newPos)
  {
    _pos = _newPos;
    return 2;
  }
  else if (leerPulsador() == 5)
  {
    return 5;
  }
  else
  {
    return 0;
  }
}
