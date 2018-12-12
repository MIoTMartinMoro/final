#include <stdio.h>

#include <contiki.h>
#include <contiki-net.h>

#include "sys/clock.h"
#include "dev/leds.h"

#include "letmecreate/core/network.h"
#include "letmecreate/core/debug.h"
#include "letmecreate/core/common.h"
#include "letmecreate/core/spi.h"
#include "lib/memb.h"

#include "common.h"
#include "mcp3004.h"

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

MEMB(ipdata, struct idappdata, MAXDATASIZE);
static struct idappdata* operacion;// = memb_alloc(&ipdata);
static struct idappdata* resultado;// = memb_alloc(&ipdata);

PROCESS(main_process, "Main process");
AUTOSTART_PROCESSES(&main_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(main_process, ev, data)
{

    PROCESS_BEGIN();
    INIT_NETWORK_DEBUG();
    {
        static struct etimer et;
        static float value = 0.0f;
        static struct uip_udp_conn* conn;
        static uint16_t minor, major;
        static char buffer[MAXDATASIZE];
        static struct uip_ip_hdr metadata;
        static int i;
        static uint8_t id;

        operacion = memb_alloc(&ipdata);
        resultado = memb_alloc(&ipdata);

        PRINTF("=====Start=====\n");

        if(spi_init() < 0 ||
            spi_set_mode(MIKROBUS_1, SPI_MODE_3) < 0)
        {
            PRINTF("SPI init failed\n");
            return 1;
        }

        PRINTF("SPI init passed\n");

        int flashes = 2;
        while(flashes--) {
              /* Flash every second */
            for(i = 0; i < 20; i++)
                clock_delay_usec(50000);

            leds_toggle(LED1);
        }

        ipv6_add_default_route(IP6_CI40, NETWORK_INFINITE_LIFETIME);
        conn = udp_new_connection(PUERTO_CLIENTE, PUERTO_SERVIDOR, IP6_CI40);
        
        flashes = 4;
        while(flashes--) {
            /* Flash every second */
            for(i = 0; i < 20; i++)
                clock_delay_usec(50000);

            leds_toggle(LED1);
        }

        PRINTF("Quien soy?\n");

        memset(operacion->data, '\0', MAXDATASIZE - ID_HEADER_LEN);
        operacion->op = OP_WHOAMI_IR;
        operacion->id = 0;
        operacion->len = strlen(operacion->data);

        PRINTF("buffer: %s\n", (char*) operacion);

        udp_packet_send(conn, (char*) operacion, ID_HEADER_LEN + operacion->len);


        PROCESS_WAIT_UDP_SENT();

        PROCESS_WAIT_UDP_RECEIVED();
        udp_packet_receive(buffer, MAXDATASIZE, &metadata);

        PRINTF("buffer: %s\n", buffer);

        PRINTF("Starting loop\n");

        while(1)
        {
            static uint8_t channel = 1;
            mcp3004_read_channel(channel, &value);

            major = (int)value;
            minor = (int)((value - (float)major) * 100.0f);

            PRINTF("Valor: %i.%02i V\n", major, minor);

            sprintf(buffer, "Voltage channel%ld: %i.%02i\n", channel, major, minor);

            udp_packet_send(conn, buffer, strlen(buffer));
            leds_on(LED1);
            PROCESS_WAIT_UDP_SENT();
            leds_off(LED1);

            etimer_set(&et, CLOCK_SECOND);
            PROCESS_WAIT_EVENT();
        }

        spi_release();
        memb_free(&ipdata, operacion);
        memb_free(&ipdata, resultado);
    }
    PROCESS_END();
}

/*---------------------------------------------------------------------------*/
