#ifndef DEFAULTS_H
#define DEFAULTS_H

#undef I2C // definido = pantalla i2c, sin definir pantalla parallel

// Valores motor
#define DEFAULT_PASOS_POR_REVOLUCION 200 // Suponiendo un motor de 200 pasos/rev sin microstepper
#define DEFAULT_ACELERACION 6000
#define DEFAULT_MICROSTEPPER 16

// Valores por defecto
#define DEFAULT_RPM 15
#define DEFAULT_VOL 0.5
#define DEFAULT_POR_INSPIRATORIO 60


#endif // DEFAULTS_H
