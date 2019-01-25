#ifndef __MCP3004_H__
#define __MCP3004_H__

#include <stdint.h>

/**
 * @brief Lee el valor de un canal del ADC mcp3004
 *
 * @param[in] channel Índice del canal, debe estar entre 0 y 3
 * @param[out] value Puntero a un float.
 * @return 0 si no hay error, -1 si sí hay.
 */
int8_t mcp3004_read_channel(uint8_t channel, float* value);

/**
 * @brief Lee el valor de todos los canales.
 * 
 * @param[out] values Puntero un array de floats.
 * @param[in] n Longitud del array.
 * @return 0 si no hay error, -1 si sí hay.
 */
int8_t mcp3004_read_all_channels(float* values, uint8_t n);

#endif
