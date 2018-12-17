#include <stdlib.h>
#include "fsm.h"

fsm_t*
fsm_new (fsm_trans_t* tt, int id)
{
	fsm_t* this = (fsm_t*) malloc (sizeof (fsm_t));
	fsm_init (this, tt, id);
	return this;
}

void
fsm_init (fsm_t* this, fsm_trans_t* tt, int id)
{
	this->tt = tt;
	this->ir_state = 0;
	this->ir_new_state = 0;
	this->id_clicker = id;
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


