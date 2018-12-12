/**
 * @file led_matrix.h
 * @author Francois Berder
 * @date 2016
 * @copyright 3-clause BSD
 *
 * @example led_matrix/main.c
 */

#ifndef __MCP3004_H__
#define __MCP3004_H__

int mcp3004_read_channel(uint8_t channel, float* value);

#endif
