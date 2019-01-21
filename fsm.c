#include <stdlib.h>

#include "fsm.h"

fsm_t*
fsm_new (fsm_trans_t* tt, uint8_t id_clicker, uint8_t id_msg, struct uip_udp_conn* conn, struct mqtt_connection* mqtt_conn)
{
    fsm_t* this = (fsm_t*) malloc (sizeof (fsm_t));
    fsm_init (this, tt, id_clicker, id_msg, conn, mqtt_conn);
    return this;
}

void
fsm_init (fsm_t* this, fsm_trans_t* tt, uint8_t id_clicker, uint8_t id_msg, struct uip_udp_conn* conn, struct mqtt_connection* mqtt_conn)
{
    this->tt = tt;
    this->ir_state = 0;
    this->ir_new_state = 0;
    this->id_msg = id_msg;
    this->id_clicker = id_clicker;
    this->conn = conn;
    this->mqtt_conn = mqtt_conn;
    this->current_state = tt[0].orig_state;
}

void
fsm_fire (fsm_t* this)
{
    fsm_trans_t* t;
    for (t = this->tt; t->orig_state >= 0; ++t) {
        if ((this->current_state == t->orig_state) && t->in(this)) {
            this->current_state = t->dest_state;
            if (t->out)
                t->out(this);
            break;
        }
    }
}


