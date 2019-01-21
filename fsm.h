#ifndef FSM_H
#define FSM_H

#include <contiki-net.h>
#include <stdint.h>

#include "mqtt.h"

typedef struct fsm_t fsm_t;

typedef int (*fsm_input_func_t) (fsm_t*);
typedef void (*fsm_output_func_t) (fsm_t*);

typedef struct fsm_trans_t {
    int orig_state;
    fsm_input_func_t in;
    int dest_state;
    fsm_output_func_t out;
} fsm_trans_t;

struct fsm_t {
    fsm_trans_t* tt;
    struct uip_udp_conn* conn;
    struct mqtt_connection* mqtt_conn;
    uint8_t current_state;
    uint8_t id_clicker;
    uint8_t id_msg; 
    uint8_t ir_new_state;
    uint8_t ir_state;
};

fsm_t* fsm_new (fsm_trans_t* tt, uint8_t id_clicker, uint8_t id_msg, struct uip_udp_conn* conn, struct mqtt_connection* mqtt_conn);
void fsm_init (fsm_t* this, fsm_trans_t* tt, uint8_t id_clicker, uint8_t id_msg, struct uip_udp_conn* conn, struct mqtt_connection* mqtt_conn);
void fsm_fire (fsm_t* this);
    
#endif