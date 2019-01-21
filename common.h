#include <stdint.h>

#define PUERTO_SERVIDOR 3001  /* puerto en el servidor */
#define PUERTO_CLIENTE  3000  /* puerto en el cliente */
#define PUERTO_MQTT     1883  /* puerto del broker MQTT */

#define IP6_CI40_CARLOS "fe80::19:f5ff:fe89:1e32"
#define IP6_CI40_ALDA "fe80::19:f5ff:fe89:1d96"
#define IP6_CI40 IP6_CI40_CARLOS

#define MAXDATASIZE 256  /* maximo numero de bytes que podemos recibir */

/* Máscara y prefijo de los dispositivos */
#define MASK          0xC0
#define IR_PREF       0x00
#define MESA_PREF     0x40
#define PULSERA_PREF  0x80
#define CI40_PREF     0xC0

#define ID_HEADER_LEN (sizeof(uint16_t) * 2 + sizeof(uint8_t))

/* formato de la unidad de datos de aplicacion para Datagramas*/
struct idappdata
{
    uint16_t op;                              /* codigo de operacion */
    uint16_t id;                              /* identificador */
    uint8_t len;                              /* longitud de datos */
    char data[MAXDATASIZE - ID_HEADER_LEN];   /* datos */
};

/* codigos de operacion (appdata.op) */
// Clicker
#define OP_WHOAMI_MESA      0x0001  /* ¿Quién soy (clicker mesa)? */
#define OP_MESA_LLAMA       0x0002  /* La mesa quiere pedir */
#define OP_MESA_CUENTA      0x0003  /* La mesa quiere la cuenta */
#define OP_MESA_OCUPADA     0x0004  /* La mesa se ocupa */
#define OP_MESA_VACIA       0x0005  /* La mesa se vacía */
#define OP_MESA_ATENDIDA    0x0006  /* La mesa ya ha sido atendida */
// IR
#define OP_WHOAMI_IR        0x0007  /* ¿Quién soy (IR)? */
#define OP_PLATO_DETECTADO  0x0008  /* Un plato está preparado */
#define OP_PLATO_RETIRADO   0x0009  /* Un plato está retirado */
// Pulsera
#define OP_WHOAMI_PULSERA   0x000A  /* ¿Quién soy (pulsera)? */
#define OP_PULSERA_LLAMADA  0x000B  /* Un camarero recibe un aviso de una mesas */
#define OP_CUENTA_PEDIDA    0x000C  /* Un camarero recibe un aviso de cuenta */
// Servidor
#define OP_AWAKE            0x000D  /* El servidor pregunta a los dispositivos si están despiertos */
#define OP_ASIGNAR_MESA     0x000E  /* El servidor le asigna una mesa a un camarero */
// Otros
#define OP_OK               0xEEEE  /* OK */
#define OP_ERROR            0xFFFF  /* ERROR */


#define BUTTON_CHECK_IRQ(port, pin) __BUTTON_CHECK_IRQ(port, pin)
/* Check if button1 was pressed */
#define BUTTON1_CHECK_IRQ() BUTTON_CHECK_IRQ(BUTTON1_PORT, BUTTON1_PIN)
#define BUTTON2_CHECK_IRQ() BUTTON_CHECK_IRQ(BUTTON2_PORT, BUTTON1_PIN)  
/* Method to be called when button1 is pressed */
