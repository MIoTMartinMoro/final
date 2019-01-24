#include <stdio.h>
#include <xc.h>

#include <contiki.h>
#include <contiki-net.h>

#include "dev/leds.h"
#include "lib/memb.h"
#include "sys/clock.h"

#include "letmecreate/core/common.h"
#include "letmecreate/core/debug.h"
#include "letmecreate/core/network.h"
#include "letmecreate/core/spi.h"
#include "mqtt.h"

#include "common.h"
#include "fsm.h"
#include "mcp3004.h"

#define UMBRAL 2
#define N_PLATOS 4

MEMB(appdata, struct idappdata, MAXDATASIZE);

/*---------------------------------------------------------------------------*/
/* Función que compara un array de voltajes con un umbral y pone a 1 la posición que lo supera y a 0 la que no */
uint8_t convert_values(float* values, int n)
{
    uint8_t resp = 0;  // ¡¡MUY IMPORTANTE QUE NO SE STATIC!!
    static uint8_t i = 0;

    for (i = 0; i < n; i++) {  // Para todos lo valores
        if (values[i] > UMBRAL) {  // Comprueba si pasa el umbral
            resp |= (1 << i);  // Y hace un OR lógico con 1 en la posición correspondiente para ponerlo a 1
        }
    }

    return resp;
}
/*---------------------------------------------------------------------------*/
//Funciones de salida. Hacen una acción. Devuelven void
void send_alert(fsm_t* fsm)
{
    static char topic[50];
    static uint8_t i = 0;
    static uint8_t id_ir = 0;
    static uint16_t mid = 0;
    static struct idappdata* envio;    
    envio = memb_alloc(&appdata);

    leds_on(LED1);

    memset(envio->data, '\0', MAXDATASIZE - ID_HEADER_LEN);

    for (i = 0; i < N_PLATOS; i++) {  // Para cada uno de los platos
        id_ir = fsm->id_clicker + i;  // Suma al id del clicker la posición del IR para conocer su id real
        if ((fsm->ir_new_state ^ fsm->ir_state) & (1 << i)) {  // Hace un XOR para comprobar los valores que son diferentes y un AND para quedarse solo con una posición
            if ((fsm->ir_new_state & (1 << i))) {  // Si el estado es diferente y el nuevo es encendido entra aquí
                sprintf(envio->data, "Plato %ld detectado", id_ir);
                envio->op = OP_PLATO_DETECTADO;
                envio->id = fsm->id_msg + (id_ir << 8);
                mid = envio->id;
                envio->len = strlen(envio->data);

                sprintf(topic, "restaurante/plato/%ld/detectado", id_ir);
                mqtt_publish(fsm->mqtt_conn, &mid, topic, (char*) envio, ID_HEADER_LEN + envio->len, 1, 0);
                udp_packet_send(fsm->conn, (char*) envio, ID_HEADER_LEN + envio->len);
                fsm->id_msg++;
            } else {  // Si el estado es diferente y el nuevo es apagado entra aquí
                sprintf(envio->data, "Plato %ld retirado", id_ir);
                envio->op = OP_PLATO_RETIRADO;
                envio->id = fsm->id_msg + (id_ir << 8);
                mid = envio->id;
                envio->len = strlen(envio->data);

                sprintf(topic, "restaurante/plato/%ld/retirado", id_ir);
                mqtt_publish(fsm->mqtt_conn, &mid, topic, (char*) envio, ID_HEADER_LEN + envio->len, 1, 0);
                udp_packet_send(fsm->conn, (char*) envio, ID_HEADER_LEN + envio->len);
                fsm->id_msg++;
            }
        }
    }

    memb_free(&appdata, envio);
    fsm->ir_state = fsm->ir_new_state;
    leds_off(LED1);
}
/*---------------------------------------------------------------------------*/
/* 
 * Funciones de lectura de sensores para comprobar el cambio de estado.
 * Devuelven 1 si queremos cambiar, 0 si no
 *
 */
int check_sensor(fsm_t* fsm)
{
    static float values[N_PLATOS];
    static uint8_t ir_values;

    mcp3004_read_all_channels(values, N_PLATOS);  // Lee todos los sensores
    ir_values = convert_values(values, N_PLATOS);  // Los compara con un umbral
    fsm->ir_new_state = ir_values;  // Y los guarda en la variable de nuevo estado

    // Si el nuevo estado es diferente al anterior y además hay al menos 0 encendido devuelve 1, si no 0
    return ((fsm->ir_new_state ^ fsm->ir_state) > 0) && ir_values > 0;
}
/*---------------------------------------------------------------------------*/
int check_sensor_none(fsm_t* fsm)
{
    static float values[N_PLATOS];
    static uint8_t ir_values;

    mcp3004_read_all_channels(values, N_PLATOS);  // Lee todos los sensores
    ir_values = convert_values(values, N_PLATOS);  // Los compara con un umbral
    fsm->ir_new_state = ir_values;  // Y los guarda en la variable de nuevo estado

    return ir_values == 0;  // Si no hay ninguno encendido devuelve 1
}
/*---------------------------------------------------------------------------*/
/* Función necesaria para usar MQTT */
void mqtt_event(struct mqtt_connection *m, mqtt_event_t event, void *data)
{
    
}
/*---------------------------------------------------------------------------*/
PROCESS(main_process, "Main process");
AUTOSTART_PROCESSES(&main_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(main_process, ev, data)
{
    PROCESS_BEGIN();
    INIT_NETWORK_DEBUG();
    {
        static char buffer[MAXDATASIZE + 1];
        static fsm_t* fsm;
        static struct etimer et;
        static struct idappdata* operacion;
        static struct idappdata* resultado;
        static struct mqtt_connection mqtt_conn;
        static struct uip_ip_hdr metadata;
        static struct uip_udp_conn* conn;
        static uint8_t flashes = 0;
        static uint8_t i = 0;
        static uint8_t id_clicker = 0;
        static uint8_t id_msg = 0;

        enum fsm_state { EMPTY, DETECTED };
        
        static fsm_trans_t sensor_ir[] = {
            {EMPTY,     check_sensor,        DETECTED,  send_alert},
            {DETECTED,  check_sensor_none,   EMPTY,     send_alert},
            {DETECTED,  check_sensor,        DETECTED,  send_alert},
            {-1, NULL, -1, NULL},
        }; 

        operacion = memb_alloc(&appdata);
        resultado = memb_alloc(&appdata);

        PRINTF("=====Start=====\n");

        // CONFIGURACIÓN SPI
        if(spi_init() < 0 ||
            spi_set_mode(MIKROBUS_1, SPI_MODE_3) < 0)
        {
            PRINTF("SPI init failed\n");
            PROCESS_EXIT();
        }

        PRINTF("SPI init passed\n");

        flashes = 2;
        while(flashes--) {
              /* Flash every second */
            for(i = 0; i < 20; i++)
                clock_delay_usec(50000);

            leds_toggle(LED1);
        }

        // CONFIGURACIÓN SOCKET UDP
        conn = udp_new_connection(PUERTO_CLIENTE, PUERTO_SERVIDOR, IP6_CI40);
        PROCESS_WAIT_UDP_CONNECTED();
        PROCESS_WAIT_UDP_CONNECTED();
        PROCESS_WAIT_UDP_CONNECTED();
        ipv6_add_default_route(IP6_CI40, NETWORK_INFINITE_LIFETIME);
        
        flashes = 4;
        while(flashes--) {
            /* Flash every second */
            for(i = 0; i < 20; i++)
                clock_delay_usec(50000);

            leds_toggle(LED1);
        }

        PRINTF("Quien soy?\n");

        // ENVIAR MENSAJE AL SERVIDOR PIDIENDO UN ID
        memset(operacion->data, '\0', MAXDATASIZE - ID_HEADER_LEN);
        strcpy(operacion->data, "Quien soy (IR)");
        operacion->op = OP_WHOAMI_IR;
        operacion->id = (uint16_t) ((IR_PREF + id_clicker) << 8) + id_msg;
        operacion->len = strlen(operacion->data);

        PRINTF("(Enviado) OP: 0x%X ID: %ld Len: %ld Data: %s\n", operacion->op, operacion->id, operacion->len, operacion->data);

        leds_on(LED1);
        leds_on(LED2);
        udp_packet_send(conn, (char*) operacion, ID_HEADER_LEN + operacion->len);
        id_msg++;
        PROCESS_WAIT_UDP_SENT();
        leds_off(LED1);
        PROCESS_WAIT_UDP_RECEIVED();
        leds_off(LED2);
        memset(buffer, '\0', MAXDATASIZE + 1);
        udp_packet_receive(buffer, MAXDATASIZE, &metadata);

        resultado = (struct idappdata*) &buffer;

        id_clicker = (uint8_t) (IR_PREF + (uint8_t) strtol(resultado->data, NULL, 10));

        PRINTF("(Recibido) OP: 0x%X ID: %ld Len: %ld Data: %s\n", resultado->op, resultado->id, resultado->len, resultado->data);

        // CONEXIÓN CON EL BROKER MQTT
        if (mqtt_register(&mqtt_conn, &main_process, &id_clicker, mqtt_event, MAXDATASIZE) < 0) {
            PRINTF("MQTT register failed\n");
            PROCESS_EXIT();
        }

        if (mqtt_connect(&mqtt_conn, IP6_CI40, PUERTO_MQTT, 99) < 0) {
            PRINTF("MQTT connect failed\n");
            PROCESS_EXIT();
        }        

        // CREACIÓN DE LA MÁQUINA DE ESTADOS
        fsm = fsm_new(sensor_ir, id_clicker, id_msg, conn, &mqtt_conn);

        PRINTF("********FSM CREADA********\n");

        PRINTF("Starting loop\n");

        // COMIENZO DEL BUCLE
        while(1)
        {
            etimer_set(&et, CLOCK_SECOND);

            fsm_fire(fsm);

            PROCESS_WAIT_EVENT();
        }

        spi_release();
        memb_free(&appdata, operacion);
        memb_free(&appdata, resultado);
    }
    PROCESS_END();
}

/*---------------------------------------------------------------------------*/
