#include <letmecreate/letmecreate.h>
#include <mosquitto.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"

struct idappdata* operacion;
struct mosquitto* mosq;

int main(int argc, char* argv[])
{

    if (argc < 7)
    {
        fprintf (stderr, "uso: publish_mqtt host, port, mid, qos, topic, message\n");
        exit (1);
    }

    mosquitto_lib_init();

    mosq = mosquitto_new(NULL, true, NULL);
    
    if(!mosq) {
        fprintf(stderr, "Can't init Mosquitto library\n");
        exit(1);
    }

    char host[16];
    char msg[MAXDATASIZE - ID_HEADER_LEN];
    char topic[50];
    uint8_t i = 0;
    uint8_t qos = 0;
    uint16_t mid = 0;
    uint16_t port = 0;

    memset(host, '\0', 16);
    memset(msg, '\0', MAXDATASIZE - ID_HEADER_LEN);
    memset(topic, '\0', 50);

    strcpy(host, argv[1]);
    strcpy(topic, argv[5]);
    port = (uint16_t) strtol(argv[2], NULL, 10);
    mid = (uint16_t) strtol(argv[3], NULL, 10);
    qos = (uint8_t) strtol(argv[4], NULL, 10);

    operacion = (struct idappdata*) malloc(MAXDATASIZE);

    operacion->op = (uint16_t) ((strtol(argv[7], NULL, 10) << 8) + strtol(argv[6], NULL, 10));
    operacion->id = (uint16_t) ((strtol(argv[9], NULL, 10) << 8) + strtol(argv[8], NULL, 10));
    operacion->len = (uint8_t) strtol(argv[10], NULL, 10);

    memset(operacion->data, '\0', MAXDATASIZE - ID_HEADER_LEN);

    for (i = 11; i < argc; i++) {
        msg[i - 11] = (char) strtol(argv[i], NULL, 10);
    }
    strcpy(operacion->data, msg);

    int ret = mosquitto_connect(mosq, host, port, 0);

    if(ret) {
        fprintf(stderr, "Can't connect to Mosquitto server\n");
        exit(1);
    }

    mosquitto_publish(mosq, &mid, topic, ID_HEADER_LEN + operacion->len, (char*) operacion, qos, 0);

    free(operacion);

    return 0;
}
