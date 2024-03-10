#ifndef _CONTEXT_HEADER_
#define _CONTEXT_HEADER_

#include <stdint.h>
#include <pthread.h>
#include "cliente.h"
#include "../src/db.h"

// app context
typedef struct{
	db_t *db;
	clientes_t clientes;
}ctx_t;

extern ctx_t ctx;


#endif