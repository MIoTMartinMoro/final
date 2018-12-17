#include <stdint.h>

#define PUERTO_SERVIDOR 3001     /* puerto en el servidor */
#define PUERTO_CLIENTE  3000     /* puerto en el cliente */

#define IP6_CI40_CARLOS "fe80::19:f5ff:fe89:1e32"
#define IP6_CI40_ALDA ""
#define IP6_CI40 IP6_CI40_CARLOS

#define MAXDATASIZE 256 /* maximo numero de bytes que podemos recibir */

#define MASK 0xC0
#define IR_PREF 0x00
#define MESA_PREF 0x60
#define PULSERA_PREF 0x80

#define ID_HEADER_LEN (sizeof(uint8_t) * 2 + sizeof(uint16_t))

/* formato de la unidad de datos de aplicacion para Datagramas*/
struct idappdata
{
        uint8_t op;                               /* codigo de operacion */
        uint8_t id;                               /* identificador */
        uint16_t len;                              /* longitud de datos */
        char data[MAXDATASIZE - ID_HEADER_LEN];   /* datos */
};

/* codigos de operacion (appdata.op) */
// Clicker
#define OP_WHOAMI_CLICKER	0x01  /* ¿Quién soy (clicker)? */
#define OP_MESA_LLAMA		0x02  /* La mesa quiere pedir */
#define OP_MESA_CUENTA		0x03  /* La mesa quiere la cuenta */
// IR
#define OP_WHOAMI_IR  	    0x04  /* ¿Quién soy (IR)? */
#define OP_PLATO_LISTO		0x05  /* Un plato está preparado */
#define OP_PLATO_RETIRADO	0x06  /* Un plato está retirado */
// Pulsera
#define OP_WHOAMI_PULSERA   0x07  /* ¿Quién soy (pulsera)? */
#define OP_PULSERA_LLAMADA  0x08  /* Un camarero recibe un aviso de una mesas */
#define OP_CUENTA_PEDIDA    0x09  /* Un camarero recibe un aviso de cuenta */
// Servidor
#define OP_AWAKE            0x0A  /* El servidor pregunta a los dispositivos si están despiertos */
// Otros
#define OP_OK               0xEE  /* OK */
#define OP_ERROR            0xFF  /* ERROR */