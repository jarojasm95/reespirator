#ifndef UTILS_H
#define UTILS_H
#include <Math.h>

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
  float peso0;
  float pesoIdeal;
  float volumenEstimado;
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
 * @param tIns: Tiempo de inspiración
 * @param tEsp: Tiempo de espiración
 * @param tCiclo: Tiempo de ciclo
 * @param porcentajeInspiratorio fraccion del ciclo en la que se inspira, tIns/tCiclo*100
 * @param rpm respiraciones por minuto
 */
void calcularCicloInspiratorio(float* tIns, float* tEsp, float* tCiclo,
                               int porcentajeInspiratorio, int rpm) {
  *tCiclo = 60 / rpm; // Tiempo de ciclo en segundos
  *tIns = *tCiclo * porcentajeInspiratorio/100;
  *tEsp = *tCiclo - *tIns;

}

#endif // UTILS_H
