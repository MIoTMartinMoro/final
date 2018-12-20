#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <letmecreate/click/common.h>
#include <letmecreate/core/spi.h>

#include "mcp3004.h"

/* Bytes */
#define START_BYTE   0X01
#define CHANNEL(I)  (0x80 + I * 0x10)
#define N_CHANNELS 4

/* Equation values */
#define VREF 5.0
#define BITS 1024

int mcp3004_read_channel(uint8_t channel, float* value)
{
    uint8_t tx_buffer[3], rx_buffer[3];
    tx_buffer[0] = START_BYTE;
    tx_buffer[1] = CHANNEL(channel);

    if (spi_transfer(tx_buffer, rx_buffer, sizeof(tx_buffer)) < 0) {
        fprintf(stderr, "mcp3004: Failed to read data.\n");
        return -1;
    }

    *value = (((rx_buffer[1] & 0x3) << 8) + rx_buffer[2]) * VREF / BITS;

    return 0;
}

int mcp3004_read_all_channels(float* values, int n)
{
    uint8_t i;

    for (i = 0; i < N_CHANNELS || i < n; i++) {
        if (mcp3004_read_channel(i, values + i) < 0) {
            fprintf(stderr, "mcp3004: Failed to read data.\n");
            return -1;
        }
    }

    return 0;
}
